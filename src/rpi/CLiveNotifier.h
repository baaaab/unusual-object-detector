#ifndef CLIVENOTIFIER_H_
#define CLIVENOTIFIER_H_

#include <ILiveResultManager.h>

class CImageStore;

class CLiveNotifier: public ILiveResultManager
{
public:
	CLiveNotifier(CImageStore* imageStore);
	virtual ~CLiveNotifier();

	void setSourceImage(uint32_t imageId, cv::Mat sourceImage);
	void setMatchImage(uint32_t imageId, float score, bool isUnusual);
};

#endif /* CLIVENOTIFIER_H_ */
