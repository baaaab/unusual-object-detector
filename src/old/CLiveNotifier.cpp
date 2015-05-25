#include "CLiveNotifier.h"

#include "../core/CImageStore.h"

CLiveNotifier::CLiveNotifier(CImageStore* imageStore) :
	_imageStore( imageStore )
{

}

CLiveNotifier::~CLiveNotifier()
{
}

void CLiveNotifier::setSourceImage(uint32_t imageId, cv::Mat sourceImage)
{

}

void CLiveNotifier::setMatchImage(uint32_t imageId, float score, bool isUnusual)
{
	if(isUnusual)
	{
		std::string imagePath = _imageStore->fetchImagePath(imageId);

		std::string cmd = "scp -p 443 " + imagePath + " pi@isitcyclingweather.com:/var/www/catcam";

		system(cmd.c_str());

		printf("Unusual image found\n");
	}

}

