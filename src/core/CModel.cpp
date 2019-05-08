#include "CModel.h"

#include "../utils/CSettingsRegistry.h"

CModel::CModel(uint32_t numCellsPerSide, CSettingsRegistry* registry) :
		_numCellsPerSide(numCellsPerSide),
		_log2NumCellsPerSide(0),
		_registry(registry),
		_registryGroup("model")
{
	if (_numCellsPerSide == 0 || (_numCellsPerSide & (_numCellsPerSide - 1)))
	{
		printf("%s::%s Invalid configuration: %u is not a power of two, exiting\n", __FILE__, __FUNCTION__, _numCellsPerSide);
	}

	while (numCellsPerSide >>= 1)
	{
		_log2NumCellsPerSide++;
	}

	std::string testRegEntry("level0");
	bool modelInitialised = true;

	try
	{
		_registry->getString(_registryGroup.c_str(), testRegEntry.c_str());
	}
	catch (...)
	{
		modelInitialised = false;
	}

	_levels.resize(getLevelsBaseAddress(_log2NumCellsPerSide));

	if (!modelInitialised)
	{
		for (uint32_t level = 0; level < _log2NumCellsPerSide; level++)
		{
			uint32_t edgeCellsThisLevel = 1 << level;

			uint32_t levelsBaseAddress = getLevelsBaseAddress(level);

			for (uint32_t j = 0; j < (edgeCellsThisLevel * edgeCellsThisLevel); j++)
			{
				_levels[levelsBaseAddress+j] = true;
			}
		}
	}
	else
	{
		for (uint32_t level = 0; level < _log2NumCellsPerSide; level++)
		{
			std::string registryEntry = std::string("level").append(std::to_string(level));
			std::string saved = _registry->getString(_registryGroup.c_str(), registryEntry.c_str());
			uint32_t edgeCellsThisLevel = 1 << level;
			uint32_t levelsBaseAddress = getLevelsBaseAddress(level);

			for (uint32_t j = 0; j < (edgeCellsThisLevel * edgeCellsThisLevel); j++)
			{
				try
				{
					_levels[levelsBaseAddress + j] = (saved.at(j) == '1');
				}
				catch (...)
				{
					_levels[levelsBaseAddress + j] = false;
				}
			}
		}
	}
}

CModel::~CModel()
{

}

uint32_t CModel::getNumLevels()
{
	return _log2NumCellsPerSide + 1;
}

uint32_t CModel::getNumCellsPerSide()
{
	return _numCellsPerSide;
}

bool CModel::isHandledAtHigherLevel(uint32_t level, uint32_t x, uint32_t y)
{
	//printf("%s::%s level = %u, x = %u, y = %u\n", __FILE__, __FUNCTION__, level, x, y);
	//check this section in all earlier levels (must be 1)
	for (uint32_t i = 0; i < level; i++)
	{
		uint32_t edgeCellsThisLevel = 1 << i;
		uint32_t levelsBaseAddress = getLevelsBaseAddress(i);

		//printf("\tL=%u, edge=%2u, index = %5u, value = %u\n", i, edgeCellsThisLevel, (y >> (level - i)) * edgeCellsThisLevel + (x >> (level - i)), _levels[levelsBaseAddress + ((y >> (level - i)) * edgeCellsThisLevel + (x >> (level - i)))]?1:0);

		if (!_levels[levelsBaseAddress + ((y >> (level - i)) * edgeCellsThisLevel + (x >> (level - i)))])
		{
			//this was handled at an earlier level
			return true;
		}
	}

	return false;
}

bool  CModel::isHandledAtThisLevel(uint32_t level, uint32_t x, uint32_t y)
{
	if(level == _log2NumCellsPerSide)
	{
		return true;
	}
	uint32_t edgeCellsThisLevel = 1 << level;
	uint32_t levelsBaseAddress = getLevelsBaseAddress(level);

	//printf("\tL=%u, edge=%2u, index = %5u, value = %u\n", level, edgeCellsThisLevel, y * edgeCellsThisLevel + x, _levels[levelsBaseAddress + (y * edgeCellsThisLevel + x)]?1:0);

	return _levels[levelsBaseAddress + (y * edgeCellsThisLevel + x)] == false;
}

void CModel::resetModel()
{
	//resets model for calculating representation function
	//namely handle everything at level 0

	for (uint32_t level = 0; level < _log2NumCellsPerSide; level++)
	{
		uint32_t edgeCellsThisLevel = 1 << level;
		uint32_t levelsBaseAddress = getLevelsBaseAddress(level);

		for (uint32_t j = 0; j < (edgeCellsThisLevel * edgeCellsThisLevel); j++)
		{
			_levels[levelsBaseAddress + j] = false;
		}
	}
}

void CModel::setHandleAtThisLevel(uint32_t level, uint32_t x, uint32_t y, bool value)
{
	if (level >= _log2NumCellsPerSide) return;

	uint32_t edgeCellsThisLevel = 1 << level;
	uint32_t levelsBaseAddress = getLevelsBaseAddress(level);
	_levels[levelsBaseAddress + (y * edgeCellsThisLevel + x)] = value;
}

std::vector<std::vector<bool>> CModel::getModel() const
{
	std::vector<std::vector<bool>> ret;
	for (uint32_t level = 0; level < _log2NumCellsPerSide; level++)
	{
		std::vector<bool> n;
		auto start = _levels.begin() + getLevelsBaseAddress(level);
		auto end = _levels.begin() + getLevelsBaseAddress(level + 1);
		n.insert(n.end(), start, end);
		ret.push_back(n);
	}
	return ret;
}

void CModel::saveToRegistry()
{
	for (uint32_t level = 0; level < _log2NumCellsPerSide; level++)
	{
		std::string stringRepresentation;
		uint32_t levelsBaseAddress = getLevelsBaseAddress(level);

		uint32_t edgeCellsThisLevel = 1 << level;
		for (uint32_t j = 0; j < (edgeCellsThisLevel * edgeCellsThisLevel); j++)
		{
			try
			{
				stringRepresentation.append(_levels[levelsBaseAddress + j] ? "1" : "0");
			}
			catch (...)
			{
				stringRepresentation.append("0");
			}
		}

		std::string registryEntry = std::string("level").append(std::to_string(level));
		_registry->setString(_registryGroup.c_str(), registryEntry.c_str(), stringRepresentation);
	}
}

uint32_t CModel::getLevelsBaseAddress(uint32_t level) const
{
	uint32_t baseAddress = 0;
	while(level)
	{
		level--;
		uint32_t edgeCellsThisLevel = 1 << level;
		baseAddress += edgeCellsThisLevel * edgeCellsThisLevel;
	}
	return baseAddress;
}
