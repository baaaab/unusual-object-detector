#include "CHttpServer.h"

#include "IHttpRequestHandler.h"
#include <algorithm>
#include <stdio.h>
#include <string.h>

namespace
{
	const char* NOT_FOUND = "Error 404: Resource not found";
}

CHttpServer::CHttpServer() :
		_daemon( NULL )
{

}

CHttpServer::~CHttpServer()
{
	if(_daemon)
	{
		MHD_stop_daemon(_daemon);
	}
}

void CHttpServer::addRequestHandler(std::string host, IHttpRequestHandler* controller)
{
	std::transform(host.begin(), host.end(), host.begin(), ::tolower);

	_handlers[host].push_back(controller);
}

int CHttpServer::start(uint16_t port)
{
	_daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY | MHD_USE_PEDANTIC_CHECKS | MHD_USE_DEBUG, port, NULL, NULL, &RequestHandler, this, MHD_OPTION_THREAD_POOL_SIZE, 4, MHD_OPTION_END);

	if (!_daemon)
	{
		printf("%s::%s Failed to start libmicrohttpd on port: %d\n", __FILE__, __FUNCTION__, port);
		throw 1;
	}

	return 0;
}

int CHttpServer::RequestHandler(void* cls, struct MHD_Connection* connection, const char* url, const char* method, const char* version, const char* upload_data, size_t* upload_data_size, void** ptr)
{
	CHttpServer* server = static_cast<CHttpServer*>(cls);

	std::string host;

	MHD_get_connection_values(connection, MHD_HEADER_KIND, GetHostName, &host);

	std::transform(host.begin(), host.end(), host.begin(), ::tolower);

	auto mapItr = server->_handlers.find(host);

	if(mapItr == server->_handlers.end())
	{
		//look for a default host
		mapItr = server->_handlers.find("*");
		if(mapItr == server->_handlers.end())
		{
			fprintf(stderr, "%s::%s unrecognised host: %s\n", __FILE__, __FUNCTION__, host.c_str());
			return MHD_NO;
		}
	}

	IHttpRequestHandler* handler = NULL;
	for (auto vItr = mapItr->second.begin(); vItr != mapItr->second.end(); ++vItr)
	{
		if ((*vItr)->validPath(url, method))
		{
			handler = *vItr;
			break;
		}
	}

	if (!handler)
	{
		struct MHD_Response* response = MHD_create_response_from_buffer(strlen(NOT_FOUND), const_cast<char*>(NOT_FOUND), MHD_RESPMEM_PERSISTENT);
		return MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
	}

	return handler->handleRequest(connection, url, method, upload_data, upload_data_size);
}

int CHttpServer::GetHostName(void *cls, enum MHD_ValueKind kind, const char *key, const char *value)
{
	std::string* hostPtr = static_cast<std::string*>(cls);
	if(strcasecmp(key, "host") == 0)
	{
		*hostPtr = value;
		auto pos = hostPtr->find(':');
		if(pos != std::string::npos)
		{
			*hostPtr = hostPtr->substr(0, pos);
		}
		return MHD_NO;
	}

	return MHD_YES;
}

