#include <IImageSource.h>

#include "CHttpImageSource.h"

IImageSource* IImageSource::GetSource(CSettingsRegistry* registry)
{
	return new CHttpImageSource(registry);
}
