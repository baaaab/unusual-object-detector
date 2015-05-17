#ifndef CMODEL_H_
#define CMODEL_H_

#include <string>
#include <vector>

#include "../utils/CSettingsRegistry.h"

class CModel
{
public:
	CModel(uint32_t numCellsPerSide, CSettingsRegistry* registry);
	virtual ~CModel();

	uint32_t getNumLevels();
	uint32_t getNumCellsPerSide();

	bool handleAtThisLevel(uint32_t level, uint32_t x, uint32_t y);

	void saveToRegistry();

private:

	uint32_t _numCellsPerSide;
	uint32_t _log2NumCellsPerSide;
	std::vector<bool>* _levels;

	CSettingsRegistry* _registry;
	std::string _registryGroup;
};

#endif /* CMODEL_H_ */
