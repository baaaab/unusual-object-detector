#include "CHogImageRequestHandler.h"

#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "../external_interface/IExternalInterface.h"
#include "../core/CHog.h"
#include "settings.h"
#include "IJpegHandler.h"
#include "CJpegHandlerFactory.h"

CHogImageRequestHandler::CHogImageRequestHandler(IExternalInterface* externalInterface, std::string directory) :
		_imageDirectory( boost::filesystem::path(directory) / "images" ),
		_externalInterface(externalInterface)
{

}

CHogImageRequestHandler::~CHogImageRequestHandler()
{
}

bool CHogImageRequestHandler::validPath(const char* path, const char* method)
{
	if (strcmp(method, "GET") == 0)
	{
		return getImageIdFromRequestUrl(path) != -1 ? true : false;
	}
	return false;
}

int CHogImageRequestHandler::handleRequest(struct MHD_Connection* connection, const char* url, const char* method, const char* upload_data, size_t* upload_data_size)
{
	int32_t imageId = getImageIdFromRequestUrl(url);
	auto hog = _externalInterface->getHog(imageId);
	boost::filesystem::path requestedPath = _imageDirectory / std::to_string(imageId / 100) / std::to_string(imageId).append(".jpg");

	struct stat64 buf;
	if(stat64(requestedPath.string().c_str(), &buf) == -1)
	{
		printf("%s::%s Requested missing image: %s\n", __FILE__, __FUNCTION__, requestedPath.string().c_str());
		perror("stat");
		return MHD_NO;
	}

	cv::Mat image = cv::imread(requestedPath.string().c_str(), cv::IMREAD_COLOR);

	float scalingFactor = 0;
	for (uint32_t i = 0; i < hog.size(); i++)
	{
		scalingFactor = std::max(scalingFactor, (float)hog[i]);
	}

	for (uint32_t y = 0; y < HOG_NUM_CELLS; y++)
	{
		for (uint32_t x = 0; x < HOG_NUM_CELLS; x++)
		{
			uint32_t hogOffset = (y * HOG_NUM_CELLS * 8) + (x * 8);

			for (uint32_t i = 0; i < 8; i++)
			{
				float height = (hog[hogOffset + i] / scalingFactor) * 8;
				uint32_t heightInPx = roundf(height);
				for(uint32_t z = 0;z<heightInPx;z++)
				{
					image.data[((y+z) * image.cols + (x+i)) * 3 + 0] = 0;
					image.data[((y+z) * image.cols + (x+i)) * 3 + 1] = 0;
					image.data[((y+z) * image.cols + (x+i)) * 3 + 2] = 255;
				}
			}
		}
	}

	IJpegHandler* jpegHandler = CJpegHandlerFactory::GetHandler();
	std::vector<uint8_t> jpeg = jpegHandler->compress(image);

	struct MHD_Response * response = MHD_create_response_from_data(jpeg.size(), jpeg.data(), false, true);

	int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
	MHD_destroy_response(response);

	delete jpegHandler;

	return ret;
}

int32_t CHogImageRequestHandler::getImageIdFromRequestUrl(const char* path)
{
	//format
	int32_t imageId = 0;
	if(sscanf(path, "/hogImageProvider/%d.jpg", &imageId) == 1)
	{
		return imageId;
	}
	else
	{
		return -1;
	}
}

