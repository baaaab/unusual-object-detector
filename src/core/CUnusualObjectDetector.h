#ifndef CUNUSUALOBJECTDETECTOR_H_
#define CUNUSUALOBJECTDETECTOR_H_

#include <string>
#include <vector>
#include <cstdint>
#include <mutex>
#include <vector>

class CImageStore;
class CSettingsRegistry;
class ILiveResultManager;
class CModel;
class CImageStore;
class CHog;
class IImageSource;
class CScoreDistribution;
class CThread;
class CRepresentationFunctionBuilder;

class CUnusualObjectDetector
{
public:
	CUnusualObjectDetector(const char* xmlFile);
	virtual ~CUnusualObjectDetector();

	enum jobType
	{
		JOB_CORRELATE_HOGS,
		JOB_SCORE_MODEL
	};

	struct hogCorrelateTask_t
	{
		uint32_t startIndex;
		uint32_t endIndex;

		float highestScore;
		uint32_t bestMatch;
	};

	struct scoreModelTask_t
	{
		std::vector<CHog*>::iterator cBegin;
		std::vector<CHog*>::iterator cEnd;
		std::vector<CHog*>::iterator xBegin;
		std::vector<CHog*>::iterator xEnd;

		float score;
	};

	struct task_t
	{
		task_t();
		~task_t();

		CUnusualObjectDetector* self;
		CThread* thread;

		jobType job;

		//mutex protects done
		std::recursive_mutex mutex;
		bool done;

		bool shutdownRequested;

		hogCorrelateTask_t hct;
		scoreModelTask_t smt;
	};

private:

	void initialiseWorkerThreads();

	static void MainThread(void* arg);
	void mainThreadFunction();

	static void WorkerThread(void* arg);
	void workerThreadFunction(task_t* task);
	void dispatchWorkerThreads(jobType jobType);
	void waitForWorkerThreadCompletion();

	void correlateHogs(task_t* task);
	void buildRepresentationFunction();

	std::string _xmlFile;
	std::string _coreRegistryGroup;

	CSettingsRegistry* _registry;
	CImageStore* _imageStore;
	ILiveResultManager* _resultManager;
	std::vector<CHog*> _hogs;
	CModel* _model;
	IImageSource* _imageSource;
	CScoreDistribution* _scoreDistrubution;
	CRepresentationFunctionBuilder* _representationFunctionBuilder;

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
