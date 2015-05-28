#include "CModel.h"

#include "../utils/CSettingsRegistry.h"

CModel::CModel(uint32_t numCellsPerSide, CSettingsRegistry* registry) :
		_numCellsPerSide(numCellsPerSide),
		_log2NumCellsPerSide(0),
		_levels( NULL),
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

	// dont need storage for final level;
	_levels = new std::vector<bool>[_log2NumCellsPerSide];

	if (registry == NULL)
	{
		throw 1;
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
	if (!modelInitialised)
	{
		for (uint32_t level = 0; level < _log2NumCellsPerSide; level++)
		{
			uint32_t edgeCellsThisLevel = 1 << level;

			_levels[level].resize(edgeCellsThisLevel * edgeCellsThisLevel);

			for (uint32_t j = 0; j < (edgeCellsThisLevel * edgeCellsThisLevel); j++)
			{
				_levels[level][j] = true;
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

			_levels[level].resize(edgeCellsThisLevel * edgeCellsThisLevel);

			for (uint32_t j = 0; j < (edgeCellsThisLevel * edgeCellsThisLevel); j++)
			{
				try
				{
					_levels[level][j] = saved.at(j) == '1';
				}
				catch (...)
				{
					_levels[level][j] = 0;
				}
			}
		}
	}
}

CModel::~CModel()
{
	delete[] _levels;
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

		//printf("\tL=%u, edge=%2u, index = %5u, value = %u\n", i, edgeCellsThisLevel, (y >> (level - i)) * edgeCellsThisLevel + (x >> (level - i)), _levels[i][(y >> (level - i)) * edgeCellsThisLevel + (x >> (level - i))]?1:0);

		if (!_levels[i][(y >> (level - i)) * edgeCellsThisLevel + (x >> (level - i))])
		{
			//this was handled at an earlier level
			return true;
		}
	}

	return false;
}

bool  CModel::isHandledAtThisLevel(uint32_t level, uint32_t x, uint32_t y)
{
	uint32_t edgeCellsThisLevel = 1 << level;

	//printf("\tL=%u, edge=%2u, index = %5u, value = %u\n", level, edgeCellsThisLevel, y * edgeCellsThisLevel + x, _levels[level][y * edgeCellsThisLevel + x]?1:0);

	return level == _log2NumCellsPerSide || _levels[level][y * edgeCellsThisLevel + x] == false;
}

void CModel::resetModel()
{
	//resets model for calculating representation function
	//namely handle everything at level 0

	for (uint32_t level = 0; level < _log2NumCellsPerSide; level++)
	{
		uint32_t edgeCellsThisLevel = 1 << level;

		for (uint32_t j = 0; j < (edgeCellsThisLevel * edgeCellsThisLevel); j++)
		{
			_levels[level][j] = false;
		}
	}
}

void CModel::setHandleAtThisLevel(uint32_t level, uint32_t x, uint32_t y, bool value)
{
	if (level >= _log2NumCellsPerSide) return;

	uint32_t edgeCellsThisLevel = 1 << level;
	_levels[level][y * edgeCellsThisLevel + x] = value;
}

void CModel::saveToRegistry()
{
	for (uint32_t level = 0; level < _log2NumCellsPerSide; level++)
	{
		std::string stringRepresentation;

		uint32_t edgeCellsThisLevel = 1 << level;
		for (uint32_t j = 0; j < (edgeCellsThisLevel * edgeCellsThisLevel); j++)
		{
			try
			{
				stringRepresentation.append(_levels[level][j] ? "1" : "0");
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
