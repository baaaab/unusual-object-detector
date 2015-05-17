#include "CTurboJpegHandler.h"

CTurboJpegHandler::CTurboJpegHandler()
{
	_jpegCompressor = tjInitCompress();
	_jpegDecompressor = tjInitDecompress();
}

CTurboJpegHandler::~CTurboJpegHandler()
{
	tjDestroy(_jpegDecompressor);
	tjDestroy(_jpegCompressor);
}

cv::Mat CTurboJpegHandler::decompress(unsigned char* buffer, uint32_t size)
{
	int32_t subSampl, width, height;
	tjDecompressHeader2(_jpegDecompressor, buffer, size, &width, &height, &subSampl);

	unsigned char* raw = new unsigned char[3 * width * height];

	tjDecompress2(_jpegDecompressor, buffer, size, raw, width, 0, height, TJPF_BGR, TJFLAG_FASTDCT);

	//does not copy data
	cv::Mat tmp(height, width, CV_8UC3, raw);

	//copies data to new internal buffer
	cv::Mat image = tmp.clone();

	delete[] raw;

	return image;
}

std::vector<unsigned char> CTurboJpegHandler::compress(const cv::Mat image)
{
	unsigned char* compressedImage = NULL;
	unsigned long jpegSize = 0;

	tjCompress2(_jpegCompressor, image.data, image.cols, 0, image.rows, TJPF_BGR, &compressedImage, &jpegSize, TJSAMP_444, 75, 0);

	const char* err = tjGetErrorStr();

	std::vector<unsigned char> vec = std::vector<unsigned char>(compressedImage, compressedImage + jpegSize);

	tjFree(compressedImage);

	return vec;
}
