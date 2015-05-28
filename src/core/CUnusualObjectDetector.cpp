#include "CUnusualObjectDetector.h"

#include <stdio.h>
#include <unistd.h>

#include <ILiveResultManager.h>
#include <IImageSource.h>
#include <settings.h>

#include "../utils/CSettingsRegistry.h"
#include "../utils/CTimer.h"
#include "../utils/CThread.h"

#include "CImageStore.h"
#include "CHog.h"
#include "CHogStore.h"
#include "CModel.h"
#include "CScoreDistribution.h"
#include "CRepresentationFunctionBuilder.h"

#include <opencv2/core/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/operations.hpp>
#include <opencv2/imgproc/imgproc.hpp>

namespace
{
	const uint32_t IDLE_THREAD_SLEEP_US = 100;

	typedef std::lock_guard<std::recursive_mutex> AutoMutex;
}

CUnusualObjectDetector::CUnusualObjectDetector(const char* xmlFile) :
		_xmlFile(xmlFile),
		_coreRegistryGroup("core"),
		_registry( NULL),
		_imageStore( NULL),
		_resultManager( NULL),
		_hogStore( NULL ),

		_model( NULL),
		_imageSource( NULL),
		_scoreDistrubution( NULL ),
		_representationFunctionBuilder( NULL ),

		_programCounter(0),
		_imageCount(0),
		_newHog( NULL ),

		_mainThread(NULL),
		_shutdownRequested(false)
{

	_registry = new CSettingsRegistry(_xmlFile.c_str());
	_imageStore = new CImageStore(_registry);

	_programCounter = _registry->getUInt32(_coreRegistryGroup.c_str(), "programCounter");
	_imageCount = _registry->getUInt32(_coreRegistryGroup.c_str(), "imageCount");
	_imageDir = _registry->getString(_coreRegistryGroup.c_str(), "imageDir");

	std::string modelDir = std::string(_imageDir).append("/model");
	std::string scoresDistributionFilename = modelDir + std::string("/scores.dat");
	_scoreDistrubution = new CScoreDistribution(scoresDistributionFilename.c_str(), _imageCount, _programCounter);

	_model = new CModel(HOG_NUM_CELLS, _registry);

	_resultManager = ILiveResultManager::GetResultManager(_imageStore, _scoreDistrubution, _model);

	std::string hogStoreFilename = modelDir + std::string("/hogs.dat");
	_hogStore = new CHogStore(hogStoreFilename, _imageCount);

	_imageSource = IImageSource::GetSource(_registry);
	_representationFunctionBuilder = new CRepresentationFunctionBuilder(_model, _hogStore);

	initialiseWorkerThreads();

	_mainThread = new CThread(MainThread, this);
}

CUnusualObjectDetector::~CUnusualObjectDetector()
{
	_shutdownRequested = true;

	for (auto itr = _tasks.begin(); itr != _tasks.end(); ++itr)
	{
		(*itr)->shutdownRequested = true;
	}

	delete _mainThread;

	for (auto itr = _tasks.begin(); itr != _tasks.end(); ++itr)
	{
		delete (*itr)->thread;
		(*itr)->thread = NULL;
		delete *itr;
	}

	delete _scoreDistrubution;
	delete _imageSource;
	delete _model;
	delete _hogStore;
	delete _resultManager;
	delete _imageStore;
	delete _registry;
}

CUnusualObjectDetector::task_t::task_t() :
	self( NULL ),
	thread( NULL ),
	job( JOB_CORRELATE_HOGS ),
	done( true ),
	shutdownRequested( false )
{

}

CUnusualObjectDetector::task_t::~task_t()
{
	delete thread;
}

void CUnusualObjectDetector::initialiseWorkerThreads()
{
	//create tasks / worker threads
	uint32_t numThreads = _registry->getUInt32(_coreRegistryGroup.c_str(), "numThreads");

	uint32_t hogsPerThread = _imageCount / numThreads;
	uint32_t hogRemainder = _imageCount % hogsPerThread;
	uint32_t hogOffset = 0;

	uint32_t numClusters = 3 * _imageCount / 4;
	uint32_t clustersPerThread = numClusters / numThreads;
	uint32_t clusterRemainder = numClusters % numThreads;
	uint32_t clusterOffset = 0;

	for (uint32_t i = 0; i < numThreads; i++)
	{
		task_t* task = new task_t();

		task->self = this;

		task->hct.startIndex = hogOffset;
		hogOffset += hogsPerThread;

		task->smt.cBegin = clusterOffset;
		clusterOffset += clustersPerThread;

		//first thread gets any extra work
		if (i == 0)
		{
			hogOffset += hogRemainder;
			clusterOffset += clusterRemainder;
		}

		task->hct.endIndex = hogOffset;
		task->smt.cEnd = clusterOffset;

		//same for all threads
		task->smt.xBegin = numClusters;
		task->smt.xEnd = _imageCount;

		_tasks.push_back(task);

		_tasks[i]->thread = new CThread(WorkerThread, task);
	}
}

void CUnusualObjectDetector::MainThread(void* arg)
{
	static_cast<CUnusualObjectDetector*>(arg)->mainThreadFunction();
}

void CUnusualObjectDetector::mainThreadFunction()
{
	cv::Mat resizedImage;

	//first image is blank
	_imageSource->getImage();
	sleep(1);
	_imageSource->getImage();
	sleep(1);

	while (!_shutdownRequested)
	{
		if(_programCounter == _imageCount || (_programCounter != 0 && _programCounter % _imageCount == 0))
		{
			buildRepresentationFunction();
			if(_shutdownRequested)
			{
				break;
			}
		}

		cv::Mat image = _imageSource->getImage();

		if(image.rows != 512 || image.cols != 512)
		{
			cv::resize(image, resizedImage, cv::Size(512, 512));
		}
		else
		{
			resizedImage = image;
		}

		_resultManager->setSourceImage(_programCounter, resizedImage);
		_imageStore->saveImage(resizedImage, _programCounter);

		_newHog = new CHog(resizedImage, _programCounter);

		dispatchWorkerThreads(JOB_CORRELATE_HOGS);
		waitForWorkerThreadCompletion();

		//collate results
		uint32_t bestMatch = 0;
		float maxScore = 0.0f;
		for (auto itr = _tasks.begin(); itr != _tasks.end(); ++itr)
		{
			if((*itr)->hct.highestScore > maxScore)
			{
				maxScore = (*itr)->hct.highestScore;
				bestMatch = (*itr)->hct.bestMatch;
			}
		}

		printf("[%6u] Best match = %u, score = %f", _programCounter, _hogStore->at(bestMatch).getCreatedAt(), maxScore);

		bool isUnusual = _programCounter > _imageCount && _scoreDistrubution->isScoreLow(maxScore);
		_scoreDistrubution->logScore(maxScore);

		_resultManager->setMatchImage(_hogStore->at(bestMatch).getCreatedAt(), maxScore, isUnusual);
		if(isUnusual)
		{
			//save file
			std::string imagePath = _imageStore->fetchImagePath(_programCounter);
			std::string cmd = std::string("cp ").append(imagePath).append(" ").append(_imageDir).append("/unusual/");
			system(cmd.c_str());
		}

		if (_programCounter < _imageCount)
		{
			printf("\n");

			_hogStore->at(_programCounter) = *_newHog;
			_hogStore->write(_programCounter);
		}
		else
		{
			//find least relevant hog to replace
			uint32_t leastRelevantHogIndex = 0;
			uint32_t oldestZeroHitHog = _programCounter;
			float lowestRelevance = 1.0f;

			for(auto itr = _hogStore->begin(); itr != _hogStore->end(); ++itr)
			{
				uint32_t age = _programCounter - itr->getCreatedAt();
				if(age < _imageCount / 4)
				{
					continue;
				}

				if(itr->getNumHits() == 0 && itr->getCreatedAt() < oldestZeroHitHog)
				{
					lowestRelevance = 0.0f;
					oldestZeroHitHog = itr->getCreatedAt();
					leastRelevantHogIndex = itr - _hogStore->begin();
					continue;
				}

				float relevance = (float)itr->getNumHits() / (float)(age);

				if(relevance < lowestRelevance)
				{
					lowestRelevance = relevance;
					leastRelevantHogIndex = itr - _hogStore->begin();
				}
			}

			printf(" replacing: %u, relevance = %f\n", leastRelevantHogIndex, lowestRelevance);

			_hogStore->at(leastRelevantHogIndex) = *_newHog;
			_hogStore->write(leastRelevantHogIndex);

			_hogStore->at(bestMatch).setMostRecentMatch(_programCounter);
			_hogStore->at(bestMatch).incrementHits();
			_hogStore->write(bestMatch);
		}

		delete _newHog;
		_newHog = NULL;

		_programCounter++;
		_registry->setUInt32(_coreRegistryGroup.c_str(), "programCounter", _programCounter);
	}

}

void CUnusualObjectDetector::WorkerThread(void* arg)
{
	task_t* task = static_cast<task_t*>(arg);
	task->self->workerThreadFunction(task);
}

void CUnusualObjectDetector::workerThreadFunction(task_t* task)
{
	while(!task->shutdownRequested)
	{
		{
			//AutoMutex am(task->mutex);
			if( task->done)
			{
				usleep(IDLE_THREAD_SLEEP_US);
				continue;
			}
		}

		if(task->job == JOB_CORRELATE_HOGS)
		{
			correlateHogs(task);
		}
		else if(task->job == JOB_SCORE_MODEL)
		{
			_representationFunctionBuilder->scoreModel(task);
		}

		AutoMutex am(task->mutex);
		task->done = true;
	}
}

void CUnusualObjectDetector::dispatchWorkerThreads(jobType jobType)
{
	//worker thread dispatch
	for (auto itr = _tasks.begin(); itr != _tasks.end(); ++itr)
	{
		(*itr)->job = jobType;

		AutoMutex am((*itr)->mutex);
		(*itr)->done = false;
	}
}

void CUnusualObjectDetector::waitForWorkerThreadCompletion()
{
	//to be called from main thread
	bool allThreadsCompleted = false;
	while(!allThreadsCompleted && !_shutdownRequested)
	{
		uint32_t completedThreads = 0;
		for (auto itr = _tasks.begin(); itr != _tasks.end(); ++itr)
		{
			AutoMutex am((*itr)->mutex);
			if((*itr)->done)
			{
				completedThreads++;
			}
		}
		if(completedThreads == _tasks.size())
		{
			allThreadsCompleted = true;
		}
		else
		{
			usleep(IDLE_THREAD_SLEEP_US);
		}
	}
}

void CUnusualObjectDetector::correlateHogs(task_t* task)
{
	task->hct.highestScore = 0.0f;
	task->hct.bestMatch = 0;

	for (uint32_t i = task->hct.startIndex; i < task->hct.endIndex && !task->shutdownRequested; i++)
	{
		float score = CHog::Correlate(*_newHog, _hogStore->at(i), _model);

		if (score > task->hct.highestScore)
		{
			task->hct.highestScore = score;
			task->hct.bestMatch = i;
		}
	}
}

void CUnusualObjectDetector::buildRepresentationFunction()
{
	_model->resetModel();

	//sort hogs
	std::sort(_hogStore->begin(), _hogStore->end(), CHog::CompareCreated);

	uint32_t numSplittableLevels = _model->getNumLevels() - 1;

	float scoreToBeat = 0.0f;

	for (uint32_t level = 0; level < 2 && !_shutdownRequested; level++)
	{
		uint32_t edgeCellsThisLevel = 1 << level;
		for (uint32_t y = 0; y < edgeCellsThisLevel && !_shutdownRequested; y++)
		{
			for (uint32_t x = 0; x < edgeCellsThisLevel && !_shutdownRequested; x++)
			{
				//try splitting at this level to see if score increases
				_model->setHandleAtThisLevel(level, x, y, true);
			}
		}
	}

	dispatchWorkerThreads(JOB_SCORE_MODEL);
	waitForWorkerThreadCompletion();


	//read scores from worker threads
	for (auto itr = _tasks.begin(); itr != _tasks.end(); ++itr)
	{
		scoreToBeat = std::max(scoreToBeat, (*itr)->smt.score);
	}

	for (uint32_t level = 2; level < numSplittableLevels && !_shutdownRequested; level++)
	{
		uint32_t edgeCellsThisLevel = 1 << level;
		for (uint32_t y = 0; y < edgeCellsThisLevel && !_shutdownRequested; y++)
		{
			for (uint32_t x = 0; x < edgeCellsThisLevel && !_shutdownRequested; x++)
			{
				if (!_model->isHandledAtHigherLevel(level, x, y))
				{
					//try splitting at this level to see if score increases
					_model->setHandleAtThisLevel(level, x, y, true);

					float splitScore = 0.0f;
					dispatchWorkerThreads(JOB_SCORE_MODEL);
					waitForWorkerThreadCompletion();

					//read scores from worker threads
					for (auto itr = _tasks.begin(); itr != _tasks.end(); ++itr)
					{
						splitScore = std::max(splitScore, (*itr)->smt.score);
					}

					if(splitScore > scoreToBeat)
					{
						printf("    splitting at: level = %u, x = %2u, y = %2u [score = %f / %f]\n", level, x, y, splitScore, scoreToBeat);
						scoreToBeat = splitScore;
					}
					else
					{
						printf("not splitting at: level = %u, x = %2u, y = %2u [score = %f / %f]\n", level, x, y, splitScore, scoreToBeat);
						//undo
						_model->setHandleAtThisLevel(level, x, y, false);
					}
				}
			}
		}
	}

	_model->saveToRegistry();
}
