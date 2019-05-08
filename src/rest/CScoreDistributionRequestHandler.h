#ifndef SRC_REST_CSCOREDISTRIBUTIONREQUESTHANDLER_H_
#define SRC_REST_CSCOREDISTRIBUTIONREQUESTHANDLER_H_

#include "http/IHttpRequestHandler.h"

#include <memory>

class IExternalInterface;

class CScoreDistributionRequestHandler : public IHttpRequestHandler
{
public:
	CScoreDistributionRequestHandler(std::shared_ptr<IExternalInterface> externalInterface);
	virtual ~CScoreDistributionRequestHandler();

	bool validPath(const char* path, const char* method);
	int handleRequest(struct MHD_Connection* connection, const char* url, const char* method, const char* upload_data, size_t* upload_data_size);

private:
	std::shared_ptr<IExternalInterface> _externalInterface;
};

#endif /* SRC_REST_CSCOREDISTRIBUTIONREQUESTHANDLER_H_ */
