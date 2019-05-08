#include "CProgramDataRequestHandler.h"

#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include "../external_interface/IExternalInterface.h"

CProgramDataRequestHandler::CProgramDataRequestHandler(std::shared_ptr<IExternalInterface> externalInterface) :
	_externalInterface( externalInterface ),
	_sourceImageId( 0 ),
	_matchImageId( 0 ),
	_score( 0.0f ),
	_matchWasUnusual( false )
{

}

CProgramDataRequestHandler::~CProgramDataRequestHandler()
{

}

bool CProgramDataRequestHandler::validPath(const char* path, const char* method)
{
	if (strcmp(method, "GET") == 0)
	{
		return strcmp(path, "/programData") == 0;
	}
	return false;
}

int CProgramDataRequestHandler::handleRequest(struct MHD_Connection* connection, const char* url, const char* method, const char* upload_data, size_t* upload_data_size)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("sourceImageId");
	writer.Uint(_sourceImageId);
	writer.Key("matchImageId");
	writer.Uint(_matchImageId);
	writer.Key("score");
	writer.Double(_score);
	writer.Key("wasUnusual");
	writer.Bool(_matchWasUnusual);
	writer.Key("imageWidth");
	writer.Uint(_externalInterface->getImageWidth());
	writer.Key("imageHeight");
	writer.Uint(_externalInterface->getImageHeight());
	writer.Key("maxNumCellsPerSide");
	writer.Uint(_externalInterface->getMaxNumCellsPerSide());
	writer.Key("model");
	writer.StartArray();
	auto model = _externalInterface->getModel();
	for (auto itr = model.begin(); itr != model.end(); ++itr)
	{
		writer.StartArray();
		for (auto itr2 = itr->begin(); itr2 != itr->end(); ++itr2)
		{
			writer.Bool(*itr2);
		}
		writer.EndArray();
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

void CProgramDataRequestHandler::setMatchImage(uint32_t sourceImageId, uint32_t matchImageId, float score, bool isUnusual)
{
	_sourceImageId   = sourceImageId;
	_matchImageId    = matchImageId;
	_score           = score;
	_matchWasUnusual = isUnusual;
}
