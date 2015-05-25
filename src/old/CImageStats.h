#ifndef CIMAGESTATS_H_
#define CIMAGESTATS_H_

#include <opencv2/core/core.hpp>

class CImageStats
{
public:
	CImageStats(cv::Mat image);
	virtual ~CImageStats();

	float getAverageBrightness();

private:

	cv::Mat _image;
	static const uint32_t PIXEL_STRIDE = 37;


};

#endif /* CIMAGESTATS_H_ */
