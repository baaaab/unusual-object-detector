#ifndef IJPEG_H_
#define IJPEG_H_

#include <opencv2/core/core.hpp>
#include <cstdint>

class IJpegHandler
{
public:
	IJpegHandler(){};
	virtual ~IJpegHandler(){};

	virtual cv::Mat decompress(uint8_t* buffer, uint32_t size) = 0;
	virtual std::vector<uint8_t> compress(const cv::Mat) = 0;

};
#endif /* IJPEG_H_ */
