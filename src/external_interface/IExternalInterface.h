#ifndef SRC_EXTERNAL_INTERFACE_IEXTERNALINTERFACE_H_
#define SRC_EXTERNAL_INTERFACE_IEXTERNALINTERFACE_H_

#include <inttypes.h>
#include <vector>

class IExternalInterface
{
public:
	IExternalInterface();
	virtual ~IExternalInterface();

	virtual uint32_t                       getImageWidth        () = 0;
	virtual uint32_t                       getImageHeight       () = 0;
	virtual uint32_t                       getMaxNumCellsPerSide() = 0;

	virtual std::vector<float>             getScoreDistribution () = 0;
	virtual std::vector<std::vector<bool>> getModel             () = 0;

	virtual std::vector<uint16_t>          getHog               (uint32_t imageId) = 0;
	virtual std::vector<float>             getRCH               (uint32_t imageId) = 0;
};

#endif /* SRC_EXTERNAL_INTERFACE_IEXTERNALINTERFACE_H_ */
