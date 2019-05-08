#ifndef SRC_REST_CRESTCONTROLLER_H_
#define SRC_REST_CRESTCONTROLLER_H_

#include <ILiveResultManager.h>

class IExternalInterface;
class CHttpServer;
class CSettingsRegistry;
class CStaticPageRequestHandler;
class CProgramDataRequestHandler;
class CHogDataRequestHandler;
class CRchDataRequestHandler;
class CScoreDistributionRequestHandler;
class CImageRequestHandler;
class CHogImageRequestHandler;
class CUnusualImageListRequestHandler;

class CRestController : public ILiveResultManager
{
public:
	CRestController(CSettingsRegistry* settings, IExternalInterface* externalInterface);
	~CRestController();

	void initialise();

	void setMatchImage(uint32_t sourceImageId, uint32_t matchImageId, float score, bool isUnusual);

private:
	CSettingsRegistry* _settings;
	IExternalInterface* _externalInterface;
	CHttpServer* _server;

	CStaticPageRequestHandler* _staticHtmlRequestHandler;
	CImageRequestHandler* _imageRequestHandler;
	CHogImageRequestHandler* _hogImageRequestHandler;
	CProgramDataRequestHandler* _programDataHandler;
	CHogDataRequestHandler* _hogDataRequestHandler;
	CRchDataRequestHandler* _rchDataRequestHandler;
	CScoreDistributionRequestHandler* _scoreDistributionRequestHandler;
	CUnusualImageListRequestHandler* _unusualImageListRequestHandler;

	bool _initialised;

};

#endif /* SRC_REST_CRESTCONTROLLER_H_ */
