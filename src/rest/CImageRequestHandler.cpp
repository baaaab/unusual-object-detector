#include "CImageRequestHandler.h"

#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

CImageRequestHandler::CImageRequestHandler(std::string directory) :
		_imageDirectory( boost::filesystem::path(directory) / "images" )
{

}

CImageRequestHandler::~CImageRequestHandler()
{
}

bool CImageRequestHandler::validPath(const char* path, const char* method)
{
	if (strcmp(method, "GET") == 0)
	{
		return getImageIdFromRequestUrl(path) != -1 ? true : false;
	}
	return false;
}

int CImageRequestHandler::handleRequest(struct MHD_Connection* connection, const char* url, const char* method, const char* upload_data, size_t* upload_data_size)
{
	int32_t imageId = getImageIdFromRequestUrl(url);
	boost::filesystem::path requestedPath = _imageDirectory / std::to_string(imageId / 100) / std::to_string(imageId).append(".jpg");

	struct stat64 buf;
	if(stat64(requestedPath.string().c_str(), &buf) == -1)
	{
		printf("%s::%s Requested missing image: %s\n", __FILE__, __FUNCTION__, requestedPath.string().c_str());
		perror("stat");
		return MHD_NO;
	}

	int fd = open(requestedPath.string().c_str(), O_RDONLY);

	struct MHD_Response * response = MHD_create_response_from_fd(buf.st_size, fd);

	int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
	MHD_destroy_response(response);

	return ret;
}

int32_t CImageRequestHandler::getImageIdFromRequestUrl(const char* path)
{
	//format
	int32_t imageId = 0;
	if(sscanf(path, "/imageProvider/%d.jpg", &imageId) == 1)
	{
		return imageId;
	}
	else
	{
		return -1;
	}
}

