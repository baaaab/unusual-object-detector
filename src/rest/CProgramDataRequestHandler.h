#ifndef SRC_REST_HTTP_CPROGRAMDATAREQUESTHANDLER_H_
#define SRC_REST_HTTP_CPROGRAMDATAREQUESTHANDLER_H_

#include "http/IHttpRequestHandler.h"
#include <ILiveResultManager.h>

#include <memory>

class IExternalInterface;

class CProgramDataRequestHandler: public IHttpRequestHandler, public ILiveResultManager
{
public:
	CProgramDataRequestHandler(std::shared_ptr<IExternalInterface> externalInterface);
	virtual ~CProgramDataRequestHandler();

	bool validPath(const char* path, const char* method);
	int handleRequest(struct MHD_Connection* connection, const char* url, const char* method, const char* upload_data, size_t* upload_data_size);

	void setMatchImage(uint32_t sourceImageId, uint32_t matchImageId, float score, bool isUnusual);

private:
	std::shared_ptr<IExternalInterface> _externalInterface;

	uint32_t _sourceImageId;
	uint32_t _matchImageId;
	float _score;
	bool _matchWasUnusual;

};

#endif /* SRC_REST_HTTP_CPROGRAMDATAREQUESTHANDLER_H_ */
