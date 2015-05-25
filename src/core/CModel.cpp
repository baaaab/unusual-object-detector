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

			_levels[level].reserve(edgeCellsThisLevel * edgeCellsThisLevel);

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

			_levels[level].reserve(edgeCellsThisLevel * edgeCellsThisLevel);

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
	return _log2NumCellsPerSide;
}

uint32_t CModel::getNumCellsPerSide()
{
	return _numCellsPerSide;
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

bool CModel::handleAtThisLevel(uint32_t level, uint32_t x, uint32_t y)
{
	if (level == 5)
		return true;

	for (uint32_t i = 0; i < level; i++)
	{
		uint32_t edgeCellsThisLevel = 1 << i;
		if (!_levels[i][(y >> (level - i)) * edgeCellsThisLevel + (x >> (level - i))])
		{
			//this was handled at an earlier level
			return false;
		}
	}
	return _levels[level][(y << _log2NumCellsPerSide) + x] == false;
}

