#include "CExternalInterface.h"

#include "../core/CUnusualObjectDetector.h"
#include "../core/CImageStore.h"
#include "../core/CHog.h"
#include "../core/CModel.h"

#include <settings.h>

CExternalInterface::CExternalInterface(CUnusualObjectDetector* unusualObjectDetector) :
		_unusualObjectDetector( unusualObjectDetector )
{

}

CExternalInterface::~CExternalInterface()
{

}

uint32_t CExternalInterface::getImageWidth()
{
	return IMAGE_WIDTH;
}

uint32_t CExternalInterface::getImageHeight()
{
	return IMAGE_HEIGHT;
}

uint32_t CExternalInterface::getMaxNumCellsPerSide()
{
	return HOG_NUM_CELLS;
}

std::vector<float> CExternalInterface::getScoreDistribution()
{
	return _unusualObjectDetector->getScoreDistribution();
}

std::vector<std::vector<bool> > CExternalInterface::getModel()
{
	return _unusualObjectDetector->getModel()->getModel();
}

std::vector<uint16_t> CExternalInterface::getHog(uint32_t imageId)
{
	CImageStore* imageStore = _unusualObjectDetector->getImageStore();
	cv::Mat im = imageStore->fetchImage(imageId);
	CHog hog(im, imageId);
	return hog.getHOG();
}

std::vector<float> CExternalInterface::getRCH(uint32_t imageId)
{
	CImageStore* imageStore = _unusualObjectDetector->getImageStore();
	cv::Mat im = imageStore->fetchImage(imageId);
	CHog hog(im, imageId);
	CModel* model = _unusualObjectDetector->getModel();
	hog.computeRCH(model);
	return hog.getRCH();
}
