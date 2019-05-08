#include "CRepresentationFunctionBuilder.h"

#include <algorithm>
#include <cstdint>

#include "CHog.h"
#include "CHogStore.h"

CRepresentationFunctionBuilder::CRepresentationFunctionBuilder(CModel* model, CHogStore* hogStore) :
	_model( model ),
	_hogStore( hogStore )
{
}

CRepresentationFunctionBuilder::~CRepresentationFunctionBuilder()
{
}

void CRepresentationFunctionBuilder::scoreModel(CUnusualObjectDetector::task_t* task)
{
	task->smt.score = 0.0f;
	float totalScore = 0.0f;

	for (uint32_t c = task->smt.cBegin; c < task->smt.cEnd && !task->shutdownRequested; c++)
	{
		float bestScore = 0.0f;
		for (uint32_t x = task->smt.xBegin; x < task->smt.xEnd && !task->shutdownRequested; x++)
		{
			bestScore = std::max(bestScore, CHog::MeasureSimilarity(_hogStore->at(c), _hogStore->at(x), _model));
		}
		totalScore += bestScore;
	}
	task->smt.score = totalScore / (float) (task->smt.cEnd - task->smt.cBegin);
}

