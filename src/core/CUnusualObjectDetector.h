#ifndef CUNUSUALOBJECTDETECTOR_H_
#define CUNUSUALOBJECTDETECTOR_H_

#include <string>
#include <vector>
#include <cstdint>
#include <mutex>
#include <vector>

#include "CHog.h"

class CImageStore;
class CSettingsRegistry;
class ILiveResultManager;
class CModel;
class CImageStore;
class CHog;
class IImageSource;
class CScoreDistribution;
class CThread;

class CUnusualObjectDetector
{
public:
	CUnusualObjectDetector(const char* xmlFile);
	virtual ~CUnusualObjectDetector();

private:

	struct task_t
	{
		task_t();
		~task_t();
		CUnusualObjectDetector* self;
		uint32_t startIndex;
		uint32_t endIndex;

		uint32_t threadId;

		float highestScore;
		uint32_t bestMatch;

		CThread* thread;

		//mutex protects done
		std::recursive_mutex mutex;
		bool done;

		bool shutdownRequested;

	};

	static void MainThread(void* arg);
	void mainThreadFunction();

	static void WorkerThread(void* arg);
	void workerThreadFunction(task_t* task);

	std::string _xmlFile;
	std::string _coreRegistryGroup;

	CSettingsRegistry* _registry;
	CImageStore* _imageStore;
	ILiveResultManager* _resultManager;
	std::vector<CHog*> _hogs;
	CModel* _model;
	IImageSource* _imageSource;
	CScoreDistribution* _scoreDistrubution;

	uint32_t _programCounter;
	uint32_t _imageCount;
	std::string _imageDir;
	std::string _hogStoreFilename;
	CHog* _newHog;

	CThread* _mainThread;

	std::vector<task_t*> _tasks;

	bool _shutdownRequested;

};

#endif /* CUNUSUALOBJECTDETECTOR_H_ */
