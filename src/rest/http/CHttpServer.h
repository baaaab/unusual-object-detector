#ifndef _CHttpServer_
#define _CHttpServer_

#include <microhttpd.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>

class IHttpRequestHandler;

class CHttpServer
{
public:
	CHttpServer();
	~CHttpServer();

	void addRequestHandler(std::string host, IHttpRequestHandler* handler);
	int start(uint16_t port);

private:
	struct MHD_Daemon* _daemon;

	/** List of controllers this server has. */
	std::map<std::string, std::vector<IHttpRequestHandler*>> _handlers;

	static int RequestHandler(void* cls, struct MHD_Connection* connection, const char* url, const char* method, const char* version, const char* upload_data, size_t* upload_data_size, void** ptr);
	static int GetHostName(void *cls, enum MHD_ValueKind kind, const char *key, const char *value);
};

#endif
