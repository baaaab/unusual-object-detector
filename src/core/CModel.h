#ifndef CMODEL_H_
#define CMODEL_H_

#include <string>
#include <vector>

class CSettingsRegistry;

class CModel
{
public:
	CModel(uint32_t numCellsPerSide, CSettingsRegistry* registry);
	virtual ~CModel();

	uint32_t getNumLevels();
	uint32_t getNumCellsPerSide();

	bool isHandledAtHigherLevel(uint32_t level, uint32_t x, uint32_t y);
	bool isHandledAtThisLevel(uint32_t level, uint32_t x, uint32_t y);

	void resetModel();
	void setHandleAtThisLevel(uint32_t level, uint32_t x, uint32_t y, bool value);

	std::vector<std::vector<bool>> getModel() const;

	void saveToRegistry();

private:

	uint32_t getLevelsBaseAddress(uint32_t level);

	uint32_t _numCellsPerSide;
	uint32_t _log2NumCellsPerSide;
	std::vector<bool> _levels;

	CSettingsRegistry* _registry;
	std::string _registryGroup;
};

#endif /* CMODEL_H_ */
