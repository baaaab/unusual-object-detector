#ifndef IIMAGEFETCHER_H_
#define IIMAGEFETCHER_H_

#include <opencv/cv.h>

class IImageFetcher
{
public:

	IImageFetcher();
	virtual ~IImageFetcher();

	virtual char* getImage() = 0;
};

#endif /* IIMAGEFETCHER_H_ */
