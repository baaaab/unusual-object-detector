#include "CRestController.h"

#include "http/CHttpServer.h"

#include "CStaticPageRequestHandler.h"
#include "CImageRequestHandler.h"
#include "CProgramDataRequestHandler.h"
#include "CHogDataRequestHandler.h"
#include "CRchDataRequestHandler.h"
#include "CScoreDistributionRequestHandler.h"

#include "../utils/CSettingsRegistry.h"
#include "../external_interface/IExternalInterface.h"

CRestController::CRestController(CSettingsRegistry* settings, std::shared_ptr<IExternalInterface> externalInterface) :
		_settings( settings ),
		_externalInterface( externalInterface ),
		_server( NULL ),
		_staticHtmlRequestHandler( NULL ),
		_imageRequestHandler( NULL ),
		_programDataHandler( NULL ),
		_hogDataRequestHandler( NULL ),
		_rchDataRequestHandler( NULL ),
		_scoreDistributionRequestHandler( NULL ),
		_initialised( false )
{

}

CRestController::~CRestController()
{
	delete _server;

	delete _staticHtmlRequestHandler;
	delete _imageRequestHandler;
	delete _programDataHandler;
	delete _hogDataRequestHandler;
	delete _rchDataRequestHandler;
	delete _scoreDistributionRequestHandler;
}

void CRestController::initialise()
{
	_staticHtmlRequestHandler = new CStaticPageRequestHandler();
	_staticHtmlRequestHandler->setRootDirectory("../html");

	_imageRequestHandler = new CImageRequestHandler(_settings->getString("core", "imageDir"));

	_programDataHandler = new CProgramDataRequestHandler(_externalInterface);
	_hogDataRequestHandler = new CHogDataRequestHandler(_externalInterface);
	_rchDataRequestHandler = new CRchDataRequestHandler(_externalInterface);
	_scoreDistributionRequestHandler = new CScoreDistributionRequestHandler(_externalInterface);

	_server = new CHttpServer();
	_server->addRequestHandler("*", _scoreDistributionRequestHandler);
	_server->addRequestHandler("*", _programDataHandler);
	_server->addRequestHandler("*", _hogDataRequestHandler);
	_server->addRequestHandler("*", _rchDataRequestHandler);
	_server->addRequestHandler("*", _imageRequestHandler);
	_server->addRequestHandler("*", _staticHtmlRequestHandler);

	_server->start(8080);

	_initialised = true;
}


void CRestController::setMatchImage(uint32_t sourceImageId, uint32_t matchImageId, float score, bool isUnusual)
{
	if(_initialised)
	{
		_programDataHandler->setMatchImage(sourceImageId, matchImageId, score, isUnusual);
	}
}
