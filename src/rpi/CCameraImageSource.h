#ifndef CCAMERAIMAGESOURCE_H_
#define CCAMERAIMAGESOURCE_H_

#include <IImageSource.h>

namespace raspicam
{
	class RaspiCam_Still_Cv;
}

class CCameraImageSource: public IImageSource
{
public:
	CCameraImageSource();
	virtual ~CCameraImageSource();

	cv::Mat getImage();

private:
	raspicam::RaspiCam_Still_Cv* _camera;
};

#endif /* CCAMERAIMAGESOURCE_H_ */
