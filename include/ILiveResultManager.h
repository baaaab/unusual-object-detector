#ifndef ILIVERESULTMANAGER_H_
#define ILIVERESULTMANAGER_H_

#include <inttypes.h>
#include <opencv2/opencv.hpp>

class CImageStore;

class ILiveResultManager
{
public:
	ILiveResultManager(){};
	virtual ~ILiveResultManager(){};

	static ILiveResultManager* GetResultManager(CImageStore* imageStore);

	virtual void setSourceImage(uint32_t imageId, cv::Mat sourceImage) = 0;
	virtual void setMatchImage(uint32_t imageId, float score, bool isUnusual) = 0;
};

#endif /* ILIVERESULTMANAGER_H_ */
