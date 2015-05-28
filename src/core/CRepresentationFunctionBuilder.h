#ifndef CREPRESENTATIONFUNCTIONBUILDER_H_
#define CREPRESENTATIONFUNCTIONBUILDER_H_

#include <vector>
#include "CUnusualObjectDetector.h"

class CModel;
class CHog;
class CHogStore;

class CRepresentationFunctionBuilder
{
public:
	CRepresentationFunctionBuilder(CModel* model, CHogStore* hogStore);
	virtual ~CRepresentationFunctionBuilder();

	void scoreModel(CUnusualObjectDetector::task_t* task);

private:
	CModel* _model;
	CHogStore* _hogStore;
};

#endif /* CREPRESENTATIONFUNCTIONBUILDER_H_ */
