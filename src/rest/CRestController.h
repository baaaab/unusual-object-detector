#ifndef SRC_REST_CRESTCONTROLLER_H_
#define SRC_REST_CRESTCONTROLLER_H_

#include <ILiveResultManager.h>

#include <memory>

class IExternalInterface;
class CHttpServer;
class CSettingsRegistry;
class CStaticPageRequestHandler;
class CProgramDataRequestHandler;
class CHogDataRequestHandler;
class CRchDataRequestHandler;
class CScoreDistributionRequestHandler;
class CImageRequestHandler;

class CRestController : public ILiveResultManager
{
public:
	CRestController(CSettingsRegistry* settings, std::shared_ptr<IExternalInterface> externalInterface);
	~CRestController();

	void initialise();

	void setMatchImage(uint32_t sourceImageId, uint32_t matchImageId, float score, bool isUnusual);

private:
	CSettingsRegistry* _settings;
	std::shared_ptr<IExternalInterface> _externalInterface;
	CHttpServer* _server;

	CStaticPageRequestHandler* _staticHtmlRequestHandler;
	CImageRequestHandler* _imageRequestHandler;
	CProgramDataRequestHandler* _programDataHandler;
	CHogDataRequestHandler* _hogDataRequestHandler;
	CRchDataRequestHandler* _rchDataRequestHandler;
	CScoreDistributionRequestHandler* _scoreDistributionRequestHandler;

	bool _initialised;

};

#endif /* SRC_REST_CRESTCONTROLLER_H_ */
