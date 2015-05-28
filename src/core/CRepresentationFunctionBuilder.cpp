#include "CRepresentationFunctionBuilder.h"

#include "CHog.h"

CRepresentationFunctionBuilder::CRepresentationFunctionBuilder(CModel* model, std::vector<CHog*>::iterator hogBegin, std::vector<CHog*>::iterator hogEnd) :
	_model( model ),
	_hogBegin( hogBegin ),
	_hogEnd( hogEnd )
{
}

CRepresentationFunctionBuilder::~CRepresentationFunctionBuilder()
{
}

void CRepresentationFunctionBuilder::scoreModel(CUnusualObjectDetector::task_t* task)
{
	task->smt.score = 0.0f;
	float totalScore = 0.0f;

	for (auto cItr = task->smt.cBegin; cItr != task->smt.cEnd && !task->shutdownRequested; ++cItr)
	{
		float bestScore = 0.0f;
		for (auto xItr = task->smt.xBegin; xItr != task->smt.xEnd && !task->shutdownRequested; ++xItr)
		{
			bestScore = std::max(bestScore, CHog::Correlate(*cItr, *xItr, _model));
		}
		totalScore += bestScore;
	}
	task->smt.score = totalScore / (float) (task->smt.cEnd - task->smt.cBegin);
}

