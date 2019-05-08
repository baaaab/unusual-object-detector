#ifndef SRC_REST_CUnusualImageListRequestHandler_H_
#define SRC_REST_CUnusualImageListRequestHandler_H_

#include "http/IHttpRequestHandler.h"

class IExternalInterface;

class CUnusualImageListRequestHandler: public IHttpRequestHandler
{
public:
	CUnusualImageListRequestHandler(IExternalInterface* externalInterface);
	virtual ~CUnusualImageListRequestHandler();

	bool validPath(const char* path, const char* method);
	int handleRequest(struct MHD_Connection* connection, const char* url, const char* method, const char* upload_data, size_t* upload_data_size);

private:
	IExternalInterface* _externalInterface;
};

#endif /* SRC_REST_CUnusualImageListRequestHandler_H_ */
