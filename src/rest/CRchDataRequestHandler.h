#ifndef SRC_REST_CRCHDATAREQUESTHANDLER_H_
#define SRC_REST_CRCHDATAREQUESTHANDLER_H_

#include "http/IHttpRequestHandler.h"

#include <memory>

class IExternalInterface;

class CRchDataRequestHandler: public IHttpRequestHandler
{
public:
	CRchDataRequestHandler(std::shared_ptr<IExternalInterface> externalInterface);
	virtual ~CRchDataRequestHandler();

	bool validPath(const char* path, const char* method);
	int handleRequest(struct MHD_Connection* connection, const char* url, const char* method, const char* upload_data, size_t* upload_data_size);

private:
	std::shared_ptr<IExternalInterface> _externalInterface;
};

#endif /* SRC_REST_CHOGDATAREQUESTHANDLER_H_ */
