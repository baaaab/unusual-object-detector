#include "CJpegHandlerFactory.h"

#include "../jpeg/jpeg-compressor/CJpegHandler.h"

CJpegHandlerFactory::CJpegHandlerFactory()
{
}

CJpegHandlerFactory::~CJpegHandlerFactory()
{
}

IJpegHandler* CJpegHandlerFactory::GetHandler()
{
	return new CJpegHandler();
}

