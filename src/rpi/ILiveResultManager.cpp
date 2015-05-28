#include <ILiveResultManager.h>

#include "../core/CImageStore.h"
#include "CDisplay.h"

ILiveResultManager* ILiveResultManager::GetResultManager(CImageStore* imageStore, CScoreDistribution* scoresDistribution, CModel* model)
{
	return new CDisplay(imageStore, scoresDistribution, model);
}



