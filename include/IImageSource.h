#ifndef IIMAGESOURCE_H_
#define IIMAGESOURCE_H_

#include <opencv2/core/core.hpp>

class CSettingsRegistry;

class IImageSource
{
public:
	IImageSource(){};
	virtual ~IImageSource(){};

	static IImageSource* GetSource(CSettingsRegistry* registry);

	virtual cv::Mat getImage() = 0;

};

#endif /* IIMAGESOURCE_H_ */
