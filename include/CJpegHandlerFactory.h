#ifndef CJPEGHANDLERFACTORY_H_
#define CJPEGHANDLERFACTORY_H_

class IJpegHandler;

class CJpegHandlerFactory
{
public:
	CJpegHandlerFactory();
	virtual ~CJpegHandlerFactory();

	static IJpegHandler* GetHandler();
};

#endif /* CJPEGHANDLERFACTORY_H_ */
