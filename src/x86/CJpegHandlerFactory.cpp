#include "CJpegHandlerFactory.h"

#include <IJpegHandler.h>
#include "../jpeg/turbojpeg/CTurboJpegHandler.h"

CJpegHandlerFactory::CJpegHandlerFactory()
{
}

CJpegHandlerFactory::~CJpegHandlerFactory()
{
}

IJpegHandler* CJpegHandlerFactory::GetHandler()
{
	return new CTurboJpegHandler();
}

