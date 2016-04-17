#ifndef SRC_DEBUG_CDIRECTORYIMAGESOURCE_H_
#define SRC_DEBUG_CDIRECTORYIMAGESOURCE_H_

#include <list>

#include <IImageSource.h>

class IJpegHandler;

class CDirectoryImageSource : public IImageSource
{
public:
	CDirectoryImageSource(const char* directory);
	virtual ~CDirectoryImageSource();

	cv::Mat getImage();

private:
	std::list<std::string> getImages(const char* directory);

	std::list<std::string> _files;
	IJpegHandler* _jpegHandler;
};

#endif /* SRC_DEBUG_CDIRECTORYIMAGESOURCE_H_ */
