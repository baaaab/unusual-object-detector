#ifndef CHTTPIMAGESOURCE_H_
#define CHTTPIMAGESOURCE_H_

#include <opencv2/core/core.hpp>
#include <curl/curl.h>

#include <string>

#include <IImageSource.h>

class IJpegHandler;

class CSettingsRegistry;

class CHttpImageSource: public IImageSource
{
public:
	CHttpImageSource(CSettingsRegistry* registry);
	virtual ~CHttpImageSource();

	cv::Mat getImage();

private:
	static size_t WriteFunction(unsigned char* ptr, size_t size, size_t nmemb, void* stream);

	IJpegHandler* _jpegReader;
	std::string _url;
	CURL* _curl;
	std::vector<unsigned char> _buffer;
};

#endif /* CHTTPIMAGESOURCE_H_ */
