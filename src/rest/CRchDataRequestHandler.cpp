#include "CRchDataRequestHandler.h"

#include <string.h>

#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include "../external_interface/IExternalInterface.h"

CRchDataRequestHandler::CRchDataRequestHandler(std::shared_ptr<IExternalInterface> externalInterface) :
	_externalInterface( externalInterface )
{

}

CRchDataRequestHandler::~CRchDataRequestHandler()
{

}

bool CRchDataRequestHandler::validPath(const char* path, const char* method)
{
	if (strcmp(method, "GET") == 0)
	{
		uint32_t imageId;
		return (sscanf(path, "/rch/%u", &imageId) == 1);
	}
	return false;
}

int CRchDataRequestHandler::handleRequest(struct MHD_Connection* connection, const char* url, const char* method, const char* upload_data, size_t* upload_data_size)
{
	uint32_t imageId;
	sscanf(url, "/rch/%u", &imageId);

	auto hog = _externalInterface->getRCH(imageId);

	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("rch");
	writer.StartArray();
	for(auto itr = hog.begin(); itr != hog.end(); ++itr)
	{
		writer.Double(*itr);
	}
	writer.EndArray();
	writer.EndObject();

	const char* json = s.GetString();
	uint32_t length = s.GetSize();

	struct MHD_Response * response = MHD_create_response_from_buffer(length, const_cast<char*>(json), MHD_RESPMEM_MUST_COPY);

	int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
	MHD_destroy_response(response);

	return ret;
}

