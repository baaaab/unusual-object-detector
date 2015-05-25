#include <ILiveResultManager.h>

#include "../core/CImageStore.h"

#include "CLiveNotifier.h"

ILiveResultManager* ILiveResultManager::GetResultManager(CImageStore* imageStore)
{
	return new CLiveNotifier(imageStore);
}



