#include "CScoreDistributionRequestHandler.h"

#include <string.h>

#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include "../external_interface/IExternalInterface.h"

CScoreDistributionRequestHandler::CScoreDistributionRequestHandler(std::shared_ptr<IExternalInterface> externalInterface) :
_externalInterface( externalInterface )
{

}

CScoreDistributionRequestHandler::~CScoreDistributionRequestHandler()
{

}

bool CScoreDistributionRequestHandler::validPath(const char* path, const char* method)
{
	if (strcmp(method, "GET") == 0)
	{
		return (strcmp(path, "/scoreDistribution") == 0);
	}
	return false;
}

int CScoreDistributionRequestHandler::handleRequest(struct MHD_Connection* connection, const char* url, const char* method, const char* upload_data, size_t* upload_data_size)
{
	std::vector<float> scoreDistribution = _externalInterface->getScoreDistribution();

	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("scoreDistribution");
	writer.StartArray();
	for(auto itr = scoreDistribution.begin(); itr != scoreDistribution.end(); ++itr)
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

