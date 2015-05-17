#include "CCameraImageSource.h"

#include <raspicam_still_cv.h>

CCameraImageSource::CCameraImageSource()
{
	_camera = new raspicam::RaspiCam_Still_Cv();

	_camera->set(CV_CAP_PROP_FRAME_WIDTH, 512);
	_camera->set(CV_CAP_PROP_FRAME_HEIGHT, 512);
}

CCameraImageSource::~CCameraImageSource()
{
	delete _camera;
}

cv::Mat CCameraImageSource::getImage()
{
	_camera->open();
	cv::Mat image;
	if (!_camera->grab())
	{
		throw 1;
	}
	_camera->retrieve(image);

	return image;
}

