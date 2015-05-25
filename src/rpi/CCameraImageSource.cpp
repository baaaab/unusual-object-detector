#include "CCameraImageSource.h"

#include "picam/CCamera.h"
#include <opencv2/imgproc/imgproc.hpp>

CCameraImageSource::CCameraImageSource()
{
	_camera = CCamera::StartCamera(WIDTH, HEIGHT,30,1,true);

	/*_camera->setAWB(raspicam::RASPICAM_AWB_SUNLIGHT);
	_camera->setEncoding(raspicam::RASPICAM_ENCODING_RGB);
	_camera->setExposure(raspicam::RASPICAM_EXPOSURE_AUTO);
	_camera->setSharpness(100);
	_camera->setQuality(100);
	_camera->setVerticalFlip(true);
	_camera->setHorizontalFlip(false);
	_camera->setRotation(0);*/
}

CCameraImageSource::~CCameraImageSource()
{
	CCamera::StopCamera();
}

cv::Mat CCameraImageSource::getImage()
{
	uint8_t* rgba_data = new uint8_t[WIDTH * HEIGHT * 4];

	int ret = 0;
	if ((ret = _camera->ReadFrame(0, rgba_data, WIDTH * HEIGHT * 4)) < 0)
	{
		printf("%s::%s Error grabbing image from camera! (%d)\n", __FILE__, __FUNCTION__, ret);
		//throw 1;
	}

	cv::Mat tmp(HEIGHT, WIDTH, CV_8UC4, rgba_data);

	cv::Mat img(HEIGHT, WIDTH, CV_8UC3);

	cv::cvtColor(tmp, img, CV_RGBA2BGR);

	delete[] rgba_data;

	return img;
}

