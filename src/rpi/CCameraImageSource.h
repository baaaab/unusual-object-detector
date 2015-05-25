#ifndef CCAMERAIMAGESOURCE_H_
#define CCAMERAIMAGESOURCE_H_

#include <IImageSource.h>

class CCamera;

class CCameraImageSource: public IImageSource
{
public:
	CCameraImageSource();
	virtual ~CCameraImageSource();

	cv::Mat getImage();

private:
	CCamera* _camera;

	static const uint32_t WIDTH = 512;
	static const uint32_t HEIGHT = 512;
};

#endif /* CCAMERAIMAGESOURCE_H_ */
