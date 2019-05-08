#ifndef __CImageRequestHandler_H_
#define __CImageRequestHandler_H_

#include "http/IhttpRequestHandler.h"

#include <boost/filesystem.hpp>
#include <string>

class CImageRequestHandler: public IHttpRequestHandler
{
public:
	CImageRequestHandler(std::string directory);
	virtual ~CImageRequestHandler();

	bool validPath(const char* path, const char* method);
	int handleRequest(struct MHD_Connection* connection, const char* url, const char* method, const char* upload_data, size_t* upload_data_size);

private:

	//return -1 on failure
	int32_t getImageIdFromRequestUrl(const char* requestUrl);

	boost::filesystem::path _imageDirectory;
};

#endif /* SRC_HTTP_CSTATICPAGEREQUESTHANDLER_H_ */
