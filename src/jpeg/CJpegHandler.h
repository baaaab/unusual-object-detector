#ifndef CLIBJPEGHANDLER_H_
#define CLIBJPEGHANDLER_H_

#include <IJpegHandler.h>

class CJpegHandler: public IJpegHandler
{
public:
	CJpegHandler();
	virtual ~CJpegHandler();

	cv::Mat decompress(uint8_t* buffer, uint32_t size);
	std::vector<uint8_t> compress(const cv::Mat);
};

#endif /* CLIBJPEGHANDLER_H_ */
