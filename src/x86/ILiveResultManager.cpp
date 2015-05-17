#include <ILiveResultManager.h>

#include "../core/CImageStore.h"

ILiveResultManager* ILiveResultManager::GetResultManager(CImageStore* imageStore)
{
	return new CDisplay(imageStore);
}



