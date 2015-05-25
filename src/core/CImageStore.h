#ifndef CIMAGESTORE_H_
#define CIMAGESTORE_H_

#include <opencv2/core/core.hpp>
#include <string>

#include "../utils/CSettingsRegistry.h"
#include <IJpegHandler.h>

class CImageStore
{
public:
	CImageStore(CSettingsRegistry* registry);
	virtual ~CImageStore();

	void saveImage(cv::Mat image, uint32_t programCounter);

	std::string fetchImagePath(uint32_t programCounter);
	cv::Mat fetchImage(uint32_t programCounter);

private:
	IJpegHandler* _jpegHandler;
	std::string _imageDir;
};

#endif /* CIMAGESTORE_H_ */
