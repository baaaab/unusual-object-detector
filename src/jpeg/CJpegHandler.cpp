#include "CJpegHandler.h"

#include "jpgd.h"
#include "jpge.h"

CJpegHandler::CJpegHandler()
{
	// TODO Auto-generated constructor stub

}

CJpegHandler::~CJpegHandler()
{
	// TODO Auto-generated destructor stub
}

cv::Mat CJpegHandler::decompress(uint8_t* buffer, uint32_t size)
{
	int32_t actualComps, width, height;
	uint8_t* raw = jpgd::decompress_jpeg_image_from_memory(buffer, size, &width, &height, &actualComps, 3);

	//does not copy data
	cv::Mat tmp(height, width, CV_8UC3, raw);

	//copies data to new internal buffer
	cv::Mat image = tmp.clone();

	delete[] raw;

	return image;
}

std::vector<uint8_t> CJpegHandler::compress(const cv::Mat image)
{
	int32_t bufSize = image.step[0] * image.rows;

	uint8_t* compressedImage = new uint8_t[bufSize];

	jpge::params options;
	options.m_quality = 65;
	options.m_subsampling = jpge::H2V2;
	options.m_two_pass_flag = true;

	jpge::compress_image_to_jpeg_file_in_memory(compressedImage, bufSize, image.cols, image.rows, image.channels(), image.data, options);

	std::vector<uint8_t> vec = std::vector<uint8_t>(compressedImage, compressedImage + bufSize);

	delete[] compressedImage;

	return vec;
}

