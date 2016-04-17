#include "CHttpImageSource.h"

#include <string>
#include <CJpegHandlerFactory.h>
#include "../utils/CSettingsRegistry.h"
#include <IJpegHandler.h>

CHttpImageSource::CHttpImageSource(CSettingsRegistry* registry) :
		_jpegReader( NULL),
		_curl( NULL)
{
	_url = registry->getString("web", "imageUrl");

	_curl = curl_easy_init();
	if (!_curl)
	{
		printf("Error starting curl");
		exit(1);
	}
	curl_easy_setopt(_curl, CURLOPT_URL, _url.c_str());
	curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, WriteFunction);
	curl_easy_setopt(_curl, CURLOPT_WRITEDATA, static_cast<void*>(this));

	_jpegReader = CJpegHandlerFactory::GetHandler();
}

CHttpImageSource::~CHttpImageSource()
{
	delete _jpegReader;
	curl_easy_cleanup(_curl);
}

cv::Mat CHttpImageSource::getImage()
{
	_buffer.clear();

	curl_easy_setopt(_curl, CURLOPT_URL, _url.c_str());
	curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, WriteFunction);
	curl_easy_setopt(_curl, CURLOPT_WRITEDATA, static_cast<void*>(this));

	curl_easy_perform(_curl);

	return _jpegReader->decompress(&_buffer[0], _buffer.size());
}

size_t CHttpImageSource::WriteFunction(unsigned char* ptr, size_t size, size_t nmemb, void* stream)
{
	CHttpImageSource* p = static_cast<CHttpImageSource*>(stream);

	p->_buffer.insert(p->_buffer.end(), ptr, ptr + (size * nmemb));

	return (size * nmemb);
}

