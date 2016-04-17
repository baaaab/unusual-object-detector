#ifndef ILIVERESULTMANAGER_H_
#define ILIVERESULTMANAGER_H_

#include <inttypes.h>

class ILiveResultManager
{
public:
	ILiveResultManager(){};
	virtual ~ILiveResultManager(){};

	virtual void setMatchImage(uint32_t sourceImageId, uint32_t matchImageId, float score, bool isUnusual) = 0;
};

#endif /* ILIVERESULTMANAGER_H_ */
