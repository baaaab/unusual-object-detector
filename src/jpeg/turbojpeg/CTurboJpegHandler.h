#ifndef CJPEGHANDLER_H_
#define CJPEGHANDLER_H_

#include <opencv2/core/core.hpp>
#include <vector>
#include <turbojpeg.h>

class CTurboJpegHandler : IJpegHandler
{
public:
	CTurboJpegHandler();
	~CTurboJpegHandler();

	cv::Mat decompress(unsigned char* buffer, uint32_t size);
	std::vector<unsigned char> compress(const cv::Mat);

private:
	tjhandle _jpegCompressor;
	tjhandle _jpegDecompressor;
};

#endif /* CJPEGHANDLER_H_ */
