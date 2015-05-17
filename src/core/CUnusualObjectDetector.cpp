#include "CUnusualObjectDetector.h"

#include <ILiveResultManager.h>
#include <IImageSource.h>
#include <settings.h>

#include "../utils/CSettingsRegistry.h"
#include "../utils/CTimer.h"

#include "CImageStore.h"
#include "CHog.h"
#include "CModel.h"

#include <opencv2/opencv.hpp>
#include <thread>

CUnusualObjectDetector::CUnusualObjectDetector(const char* xmlFile) :
		_xmlFile(xmlFile),
		_coreRegistryGroup("core"),
		_registry( NULL),
		_imageStore( NULL),
		_resultManager( NULL),

		_model( NULL),
		_imageSource( NULL),

		_programCounter(0),
		_imageCount(0),
		_thread(NULL),
		_shutdownRequested(false)
{
	_registry = new CSettingsRegistry(_xmlFile.c_str());
	_imageStore = new CImageStore(_registry);
	_resultManager = ILiveResultManager::GetResultManager(_imageStore);

	_programCounter = _registry->getUInt32(_coreRegistryGroup.c_str(), "programCounter");
	_imageCount = _registry->getUInt32(_coreRegistryGroup.c_str(), "imageCount");
	_imageDir = _registry->getString(_coreRegistryGroup.c_str(), "imageDir");

	std::string modelDir = _imageDir.append("/model");

	_hogs.reserve(_imageCount);
	_hogStoreFilename = modelDir + std::string("/hogs.dat");
	FILE* fh = fopen(_hogStoreFilename.c_str(), "rb");
	for (uint32_t i = 0; i < _imageCount; i++)
	{
		_hogs[i] = new CHog(HOG_CELL_SIZE, HOG_NUM_CELLS, fh);
	}
	fclose(fh);

	_model = new CModel(HOG_NUM_CELLS, _registry);

	_imageSource = IImageSource::GetSource(_registry);

	_thread = new std::thread(ThreadFunction, this);
}

CUnusualObjectDetector::~CUnusualObjectDetector()
{

	printf("%s::%s\n", __FILE__, __FUNCTION__);

	_shutdownRequested = true;

	delete _thread;

	delete _imageSource;
	delete _model;

	for (uint32_t i = 0; i < _imageCount; i++)
	{
		delete _hogs[i];
	}

	delete _resultManager;
	delete _imageStore;
	delete _registry;

}

void CUnusualObjectDetector::ThreadFunction(void* arg)
{
	static_cast<CUnusualObjectDetector*>(arg)->threadFunction();
}

void CUnusualObjectDetector::threadFunction()
{
	cv::Mat resizedImage;
	cv::Mat greyImage;
	cv::Mat smoothedImage;
	cv::Mat sobelXImage;
	cv::Mat sobelYImage;

	while (!_shutdownRequested)
	{
		cv::Mat image = _imageSource->getImage();

		cv::resize(image, resizedImage, cv::Size(512, 512));

		_resultManager->setSourceImage(_programCounter, resizedImage);
		_imageStore->saveImage(resizedImage, _programCounter);

		cv::cvtColor(resizedImage, greyImage, CV_BGR2GRAY);

		cv::Sobel(greyImage, sobelXImage, CV_16S, 1, 0, 3);
		cv::Sobel(greyImage, sobelYImage, CV_16S, 0, 1, 3);

		CHog* newHog = new CHog(HOG_CELL_SIZE, HOG_NUM_CELLS, sobelXImage, sobelYImage, _programCounter);

		float maxScore = 0.0f;
		uint32_t bestMatch = 0;

		uint32_t leastRelevantHog = _imageCount;
		uint32_t oldestMatch = _programCounter;

		for (uint32_t i = 0; i < _imageCount; i++)
		{
			float score = CHog::Correlate(newHog, _hogs[i], _model);

			if (score > maxScore)
			{
				maxScore = score;
				bestMatch = i;
			}

			if ((_programCounter - _hogs[i]->getCreatedAt() > 10) && _hogs[i]->getLastBestMatch() < oldestMatch)
			{
				leastRelevantHog = i;
				oldestMatch = _hogs[i]->getLastBestMatch();
			}
		}

		printf("Best match = %u, score = %f\n", _hogs[bestMatch]->getCreatedAt(), maxScore);

		_resultManager->setMatchImage(_hogs[bestMatch]->getCreatedAt(), maxScore, false);

		if (_programCounter < _imageCount)
		{
			_hogs[_programCounter] = newHog;
			printf("Filling HOG : %u\n", _programCounter);

			FILE* fh = fopen(_hogStoreFilename.c_str(), "wb");
			if (fh)
			{
				fseek(fh, _programCounter * _hogs[_programCounter]->getSizeBytes(), SEEK_SET);
				_hogs[_programCounter]->write(fh);
			}
			fclose(fh);
		}
		else
		{
			_hogs[bestMatch]->setMostRecentMatch(_programCounter);

			CHog* oldHog = _hogs[leastRelevantHog];
			_hogs[leastRelevantHog] = newHog;
			delete oldHog;

			FILE* fh = fopen(_hogStoreFilename.c_str(), "wb");
			if (fh)
			{
				fseek(fh, leastRelevantHog * newHog->getSizeBytes(), SEEK_SET);
				newHog->write(fh);

				fseek(fh, bestMatch * newHog->getSizeBytes(), SEEK_SET);
				_hogs[bestMatch]->write(fh);
			}
			fclose(fh);

		}

		_programCounter++;
		_registry->setUInt32(_coreRegistryGroup.c_str(), "programCounter", _programCounter);

	}

}

