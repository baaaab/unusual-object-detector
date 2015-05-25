#include "CImageStats.h"

CImageStats::CImageStats(cv::Mat image) :
	_image( image )
{
}

CImageStats::~CImageStats()
{
}

float CImageStats::getAverageBrightness()
{
	uint32_t numPixels = _image.cols * _image.rows;
	uint32_t numPixelsChecked = 0;
	uint32_t accumulator = 0;

	for(uint32_t pixel = 0; pixel < numPixels; pixel += PIXEL_STRIDE)
	{
		numPixelsChecked++;
		accumulator +=_image.data[pixel] + _image.data[pixel+1] + _image.data[pixel + 2];
	}

	float brightness = accumulator / (255 * 3 * numPixels);

	printf("%s::%s image brightness = %f\n", __FILE__, __FUNCTION__, brightness);

	return brightness;
}

