#include "CJpegHandlerFactory.h"

#include "IJpegHandler.h"
#include "../jpeg/CJpegHandler.h"

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

