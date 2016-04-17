#ifndef SRC_HTTP_IHTTPREQUESTHANDLER_H_
#define SRC_HTTP_IHTTPREQUESTHANDLER_H_

#include <inttypes.h>
#include <microhttpd.h>

class IHttpRequestHandler
{
public:
	IHttpRequestHandler();
	virtual ~IHttpRequestHandler();

	virtual bool validPath(const char* path, const char* method) = 0;
	virtual int handleRequest(struct MHD_Connection* connection, const char* url, const char* method, const char* upload_data, uint32_t* upload_data_size) = 0;
};

#endif /* SRC_HTTP_IHTTPREQUESTHANDLER_H_ */
