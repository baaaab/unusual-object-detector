#ifndef CUNUSUALOBJECTDETECTOR_H_
#define CUNUSUALOBJECTDETECTOR_H_

#include <cstdint>

#include "CHog.h"

namespace std
{
class thread;
} /* namespace std */

class CImageStore;
class CSettingsRegistry;
class ILiveResultManager;
class CModel;
class CImageStore;
class CHog;
class IImageSource;

class CUnusualObjectDetector
{
public:
	CUnusualObjectDetector(const char* xmlFile);
	virtual ~CUnusualObjectDetector();

private:

	static void ThreadFunction(void* arg);
	void threadFunction();

	std::string _xmlFile;
	std::string _coreRegistryGroup;

	CSettingsRegistry* _registry;
	CImageStore* _imageStore;
	ILiveResultManager* _resultManager;
	std::vector<CHog*> _hogs;
	CModel* _model;
	IImageSource* _imageSource;

	uint32_t _programCounter;
	uint32_t _imageCount;
	std::string _imageDir;
	std::string _hogStoreFilename;

	std::thread* _thread;

	bool _shutdownRequested;

};

#endif /* CUNUSUALOBJECTDETECTOR_H_ */
