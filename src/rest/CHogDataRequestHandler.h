#ifndef SRC_REST_CHOGDATAREQUESTHANDLER_H_
#define SRC_REST_CHOGDATAREQUESTHANDLER_H_

#include "http/IHttpRequestHandler.h"

#include <memory>

class IExternalInterface;

class CHogDataRequestHandler: public IHttpRequestHandler
{
public:
	CHogDataRequestHandler(std::shared_ptr<IExternalInterface> externalInterface);
	virtual ~CHogDataRequestHandler();

	bool validPath(const char* path, const char* method);
	int handleRequest(struct MHD_Connection* connection, const char* url, const char* method, const char* upload_data, uint32_t* upload_data_size);

private:
	std::shared_ptr<IExternalInterface> _externalInterface;
};

#endif /* SRC_REST_CHOGDATAREQUESTHANDLER_H_ */
