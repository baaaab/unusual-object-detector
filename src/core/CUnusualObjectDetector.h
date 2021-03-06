#ifndef CUNUSUALOBJECTDETECTOR_H_
#define CUNUSUALOBJECTDETECTOR_H_

#include <string>
#include <vector>
#include <cstdint>
#include <mutex>
#include <pthread.h>

class CImageStore;
class CSettingsRegistry;
class ILiveResultManager;
class CModel;
class CImageStore;
class CHog;
class IImageSource;
class CScoreDistribution;
class CRepresentationFunctionBuilder;
class CHogStore;
class CRestController;
class IExternalInterface;

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
		uint32_t cBegin;
		uint32_t cEnd;
		uint32_t xBegin;
		uint32_t xEnd;

		float score;
	};

	struct task_t
	{
		task_t();
		~task_t();

		CUnusualObjectDetector* self;
		pthread_t thread;

		jobType job;

		//mutex protects done
		std::recursive_mutex mutex;
		bool done;

		bool shutdownRequested;

		hogCorrelateTask_t hct;
		scoreModelTask_t smt;
	};

	void initialise();

	// GUI functions
	uint32_t           getProgramCounter    ();
	std::vector<float> getScoreDistribution ();
	CModel*            getModel             ();

	CImageStore*       getImageStore        ();

private:

	void initialiseWorkerThreads();

	static void* MainThreadFunction(void* arg);
	void mainThreadFunction();

	static void* WorkerThreadFunction(void* arg);
	void workerThreadFunction(task_t* task);

	void dispatchWorkerThreads(jobType jobType);
	void waitForWorkerThreadCompletion();

	void correlateHogs(task_t* task);
	void buildRepresentationFunction();

	std::string                     _xmlFile;
	std::string                     _coreRegistryGroup;

	CSettingsRegistry*              _registry;
	CImageStore*                    _imageStore;
	ILiveResultManager*             _resultManager;
	CHogStore*                      _hogStore;
	IExternalInterface*             _externalInterface;
	CRestController*                _restController;

	CModel*                         _model;
	IImageSource*                   _imageSource;
	CScoreDistribution*             _scoreDistrubution;
	CRepresentationFunctionBuilder* _representationFunctionBuilder;

	uint32_t                        _programCounter;
	uint32_t                        _imageCount;
	std::string                     _imageDir;
	bool                            _keepImages;
	CHog*                           _newHog;

	pthread_t                       _mainThread;

	std::vector<task_t*>            _tasks;

	bool                            _shutdownRequested;

};

#endif /* CUNUSUALOBJECTDETECTOR_H_ */
