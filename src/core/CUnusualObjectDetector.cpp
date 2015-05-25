#include "CUnusualObjectDetector.h"

#include <stdio.h>
#include <unistd.h>

#include <ILiveResultManager.h>
#include <IImageSource.h>
#include <settings.h>

#include "../utils/CSettingsRegistry.h"
#include "../utils/CTimer.h"

#include "CImageStore.h"
#include "CHog.h"
#include "CModel.h"
#include "CScoreDistribution.h"
#include "CThread.h"

#include <opencv2/opencv.hpp>

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

		_model( NULL),
		_imageSource( NULL),
		_scoreDistrubution( NULL ),

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

	std::string modelDir = _imageDir.append("/model");
	std::string scoresDistributionFilename = modelDir + std::string("/scores.dat");
	_scoreDistrubution = new CScoreDistribution(scoresDistributionFilename.c_str(), _imageCount);

	_resultManager = ILiveResultManager::GetResultManager(_imageStore, _scoreDistrubution);

	_hogs.resize(_imageCount);
	_hogStoreFilename = modelDir + std::string("/hogs.dat");
	FILE* fh = fopen(_hogStoreFilename.c_str(), "rb");
	if(!fh)
	{
		throw 1;
	}

	for (uint32_t i = 0; i < _imageCount; i++)
	{
		_hogs[i] = new CHog(HOG_CELL_SIZE, HOG_NUM_CELLS, fh);
	}
	fclose(fh);

	_model = new CModel(HOG_NUM_CELLS, _registry);
	_imageSource = IImageSource::GetSource(_registry);

	//create tasks / worker threads
	uint32_t numThreads =  _registry->getUInt32(_coreRegistryGroup.c_str(), "numThreads");


	uint32_t hogsPerThread = _imageCount / numThreads;
	uint32_t remainder = _imageCount % hogsPerThread;
	uint32_t offset = 0;

	for (uint32_t i = 0; i < numThreads; i++)
	{
		task_t* task = new task_t();

		task->self = this;

		task->threadId = i;

		task->startIndex = offset;
		offset += hogsPerThread;

		//first thread gets any extra work
		if (i == 0)
		{
			offset += remainder;
		}

		//for loop uses <
		task->endIndex = offset - 1;

		_tasks.push_back(task);

		_tasks[i]->thread = new CThread(WorkerThread, task);
	}

	_mainThread = new CThread(MainThread, this);
}

CUnusualObjectDetector::~CUnusualObjectDetector()
{
	_shutdownRequested = true;
	delete _mainThread;

	for (auto itr = _tasks.begin(); itr != _tasks.end(); ++itr)
	{
		(*itr)->shutdownRequested = true;
	}
	for (auto itr = _tasks.begin(); itr != _tasks.end(); ++itr)
	{
		delete (*itr)->thread;
		(*itr)->thread = NULL;
		delete *itr;
	}

	delete _scoreDistrubution;
	delete _imageSource;
	delete _model;

	for (uint32_t i = 0; i < _imageCount; i++)
	{
		delete _hogs[i];
	}

	delete _resultManager;
	delete _imageStore;
	delete _registry;

	delete _newHog;
}

void CUnusualObjectDetector::MainThread(void* arg)
{
	static_cast<CUnusualObjectDetector*>(arg)->mainThreadFunction();
}

void CUnusualObjectDetector::mainThreadFunction()
{
	cv::Mat resizedImage;
	cv::Mat greyImage;
	cv::Mat smoothedImage;
	cv::Mat sobelXImage;
	cv::Mat sobelYImage;

	//first image is blank
	_imageSource->getImage();
	sleep(1);
	_imageSource->getImage();
	sleep(1);

	while (!_shutdownRequested)
	{
		cv::Mat image = _imageSource->getImage();

		if(image.rows != 515 || image.cols != 512)
		{
			cv::resize(image, resizedImage, cv::Size(512, 512));
		}

		_resultManager->setSourceImage(_programCounter, resizedImage);
		_imageStore->saveImage(resizedImage, _programCounter);

		cv::cvtColor(resizedImage, greyImage, CV_BGR2GRAY);

		cv::GaussianBlur(greyImage, greyImage, cv::Size(7, 7), 0.5, 0.5);

		cv::Sobel(greyImage, sobelXImage, CV_16S, 1, 0, 3);
		cv::Sobel(greyImage, sobelYImage, CV_16S, 0, 1, 3);

		_newHog = new CHog(HOG_CELL_SIZE, HOG_NUM_CELLS, sobelXImage, sobelYImage, _programCounter);

		//worker thread dispatch
		for (auto itr = _tasks.begin(); itr != _tasks.end(); ++itr)
		{
			AutoMutex am((*itr)->mutex);
			(*itr)->done = false;
		}

		//wait for worker threads to finish
		bool allThreadsCompleted = false;
		while(!allThreadsCompleted)
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

		//collate results
		uint32_t bestMatch = 0;
		float maxScore = 0.0f;
		for (auto itr = _tasks.begin(); itr != _tasks.end(); ++itr)
		{
			if((*itr)->highestScore > maxScore)
			{
				maxScore = (*itr)->highestScore;
				bestMatch = (*itr)->bestMatch;
			}
		}

		printf("Best match = %u, score = %f\n", _hogs[bestMatch]->getCreatedAt(), maxScore);

		bool isUnusual = _programCounter > _imageCount && _scoreDistrubution->isScoreLow(maxScore);
		_scoreDistrubution->logScore(maxScore);

		_resultManager->setMatchImage(_hogs[bestMatch]->getCreatedAt(), maxScore, isUnusual);

		if (_programCounter < _imageCount)
		{
			_hogs[_programCounter] = _newHog;

			FILE* fh = fopen(_hogStoreFilename.c_str(), "r+b");
			if (fh)
			{
				fseek(fh, _programCounter * _hogs[_programCounter]->getSizeBytes(), SEEK_SET);
				_hogs[_programCounter]->write(fh);
			}
			fclose(fh);
		}
		else
		{
			//find least relevant hog to replace
			uint32_t leastRelevantHogIndex = 0;
			uint32_t oldestLastMatched = _programCounter;

			for(auto itr = _hogs.begin(); itr != _hogs.end(); ++itr)
			{
				if((*itr)->getLastBestMatch() < oldestLastMatched)
				{
					oldestLastMatched = (*itr)->getLastBestMatch();
					leastRelevantHogIndex = itr - _hogs.begin();
				}
			}

			printf("Replacing least relevant hog: %d\n", leastRelevantHogIndex);

			CHog* oldHog = _hogs[leastRelevantHogIndex];
			_hogs[leastRelevantHogIndex] = _newHog;
			delete oldHog;

			FILE* fh = fopen(_hogStoreFilename.c_str(), "r+b");
			if (fh)
			{
				fseek(fh, leastRelevantHogIndex * _newHog->getSizeBytes(), SEEK_SET);
				_newHog->write(fh);

				_hogs[bestMatch]->setMostRecentMatch(_programCounter);
				fseek(fh, bestMatch * _newHog->getSizeBytes(), SEEK_SET);
				_hogs[bestMatch]->write(fh);
			}
			fclose(fh);

		}

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
			AutoMutex am(task->mutex);
			if( task->done)
			{
				usleep(IDLE_THREAD_SLEEP_US);
				continue;
			}
		}

		task->highestScore = 0.0f;
		task->bestMatch = 0;

		for (uint32_t i = task->startIndex; i < task->endIndex && !task->shutdownRequested; i++)
		{
			float score = CHog::Correlate(_newHog, _hogs[i], _model);

			if (score > task->highestScore)
			{
				task->highestScore = score;
				task->bestMatch = i;
			}
		}

		AutoMutex am(task->mutex);
		task->done = true;
	}
}

CUnusualObjectDetector::task_t::task_t() :
	self( NULL ),
	startIndex( 0 ),
	endIndex( 0 ),
	threadId( 0 ),
	highestScore( 0.0f ),
	bestMatch( 0 ),
	thread( NULL ),
	done( true ),
	shutdownRequested( false )
{

}

CUnusualObjectDetector::task_t::~task_t()
{
	delete thread;
}
