#ifndef CREPRESENTATIONFUNCTIONBUILDER_H_
#define CREPRESENTATIONFUNCTIONBUILDER_H_

#include <vector>
#include "CUnusualObjectDetector.h"

class CModel;
class CHog;

class CRepresentationFunctionBuilder
{
public:
	CRepresentationFunctionBuilder(CModel* model, std::vector<CHog*>::iterator hogBegin, std::vector<CHog*>::iterator hogEnd);
	virtual ~CRepresentationFunctionBuilder();

	void scoreModel(CUnusualObjectDetector::task_t* task);

private:
	CModel* _model;
	std::vector<CHog*>::iterator _hogBegin;
	std::vector<CHog*>::iterator _hogEnd;
};

#endif /* CREPRESENTATIONFUNCTIONBUILDER_H_ */
