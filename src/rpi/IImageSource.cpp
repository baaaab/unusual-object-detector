#include <IImageSource.h>

#include "CCameraImageSource.h"

IImageSource* IImageSource::GetSource(CSettingsRegistry* registry)
{
	return new CCameraImageSource();
}
