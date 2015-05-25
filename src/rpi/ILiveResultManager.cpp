#include <ILiveResultManager.h>

#include "../core/CImageStore.h"
#include "CDisplay.h"

ILiveResultManager* ILiveResultManager::GetResultManager(CImageStore* imageStore, CScoreDistribution* scoresDistribution)
{
	return new CDisplay(imageStore, scoresDistribution);
}



