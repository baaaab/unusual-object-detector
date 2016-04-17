#ifndef SRC_EXTERNAL_INTERFACE_CEXTERNALINTERFACE_H_
#define SRC_EXTERNAL_INTERFACE_CEXTERNALINTERFACE_H_

#include "IExternalInterface.h"

#include <vector>

class CUnusualObjectDetector;

class CExternalInterface : public IExternalInterface
{
public:
	CExternalInterface(CUnusualObjectDetector* unusualObjectDetector);
	~CExternalInterface();

	//constants
	uint32_t                       getImageWidth        ();
	uint32_t                       getImageHeight       ();
	uint32_t                       getMaxNumCellsPerSide();

	std::vector<float>             getScoreDistribution ();
	std::vector<std::vector<bool>> getModel             ();

	std::vector<uint16_t>          getHog               (uint32_t imageId);
	std::vector<uint16_t>          getRCH               (uint32_t imageId);

private:

	CUnusualObjectDetector* _unusualObjectDetector;


};

#endif /* SRC_EXTERNAL_INTERFACE_CEXTERNALINTERFACE_H_ */
