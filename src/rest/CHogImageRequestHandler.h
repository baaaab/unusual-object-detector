#ifndef __CHogImageRequestHandler_H_
#define __CHogImageRequestHandler_H_

#include "http/IHttpRequestHandler.h"

#include <boost/filesystem.hpp>
#include <string>

class IExternalInterface;

class CHogImageRequestHandler: public IHttpRequestHandler
{
public:
	CHogImageRequestHandler(IExternalInterface* externalInterface, std::string directory);
	virtual ~CHogImageRequestHandler();

	bool validPath(const char* path, const char* method);
	int handleRequest(struct MHD_Connection* connection, const char* url, const char* method, const char* upload_data, size_t* upload_data_size);

private:

	//return -1 on failure
	int32_t getImageIdFromRequestUrl(const char* requestUrl);

	boost::filesystem::path _imageDirectory;
	IExternalInterface* _externalInterface;
};

#endif /* SRC_HTTP_CSTATICPAGEREQUESTHANDLER_H_ */
