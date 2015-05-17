#include "CHog.h"

#include <math.h>
#include <CModel.h>

#include <error.h>

const uint32_t CHog::_magic = 0x2955a6a5;

CHog::CHog(uint32_t cellSize, uint32_t numCellsPerSide, FILE* fh) :
		_cellSize(cellSize),
		_cellsPerSide(numCellsPerSide),
		_values( NULL),
		_createdAt(0),
		_lastBestMatch(0),
		_active(true)
{
	if (fh == NULL)
	{
		//allocate and fill with zeros
		_values = new uint16_t[_numBins * _cellsPerSide * _cellsPerSide];
		memset(_values, 0, sizeof(_values[0]) * _numBins * _cellsPerSide * _cellsPerSide);
	}
	else
	{
		//allocate and read from file handle
		_values = new uint16_t[_numBins * _cellsPerSide * _cellsPerSide];

		uint32_t tmp;

		fread(&tmp, 1, sizeof(tmp), fh);

		if (tmp != _magic)
		{
			printf("%s::%s Error reading HOGs from file: data is corrupted\n", __FILE__, __FUNCTION__);
			throw 1;
		}

		fread(&_createdAt, 1, sizeof(_createdAt), fh);
		fread(&_lastBestMatch, 1, sizeof(_lastBestMatch), fh);
		fread(&_active, 1, sizeof(_active), fh);

		fread(_values, 1, sizeof(_values[0]) * _numBins * _cellsPerSide * _cellsPerSide, fh);

	}
}

CHog::CHog(uint32_t cellSize, uint32_t numCellsPerSide, cv::Mat sobelX, cv::Mat sobelY, uint32_t programCounter) :
		_cellSize(cellSize),
		_cellsPerSide(numCellsPerSide),
		_values( NULL),
		_createdAt(programCounter),
		_lastBestMatch(programCounter),
		_active(true)
{
	_values = new uint16_t[_numBins * _cellsPerSide * _cellsPerSide];

	uint32_t width = _cellSize * _cellsPerSide;
	for (uint32_t sqy = 0; sqy < numCellsPerSide; sqy++)
	{
		for (uint32_t sqx = 0; sqx < numCellsPerSide; sqx++)
		{
			float histogram[8];
			for (uint32_t x = 0; x < _numBins; x++)
			{
				histogram[x] = 0.0f;
			}
			for (uint32_t y = 0; y < _numBins; y++)
			{
				for (uint32_t x = 0; x < _numBins; x++)
				{
					uint32_t px = sqx * _numBins + x;
					uint32_t py = sqy * _numBins + y;

					float angle = atan2f((float) sobelY.data[py * width + px], (float) sobelX.data[py * width + px]);

					float magnitude = sqrtf((float) ((float) sobelY.data[py * width + px] * (float) sobelY.data[py * width + px]) + ((float) sobelX.data[py * width + px] * (float) sobelX.data[py * width + px]));

					uint32_t index = (uint32_t) floor(4 * (0.99999 + angle / (float) M_PI));

					histogram[index] += magnitude;
				}
			}
			for (uint32_t x = 0; x < _numBins; x++)
			{
				_values[(sqy * _cellsPerSide + sqx) * _numBins + x] = histogram[x];
			}
		}
	}
}

CHog::~CHog()
{
	delete[] _values;
}

CHog& CHog::operator=(const CHog& other)
{
	if (other._cellSize == _cellSize && other._cellsPerSide == _cellsPerSide)
	{
		memcpy(_values, other._values, _numBins * _cellsPerSide * _cellsPerSide);
	}
	else
	{
		printf("Both CHogs but have the same dimensions");
		throw 1;
	}
	return *this;
}

float CHog::Correlate(CHog* a, CHog* b, CModel* model)
{
	float score = 0;

	uint32_t numLevels = model->getNumLevels();

	for (uint32_t level = 0; level < numLevels; level++)
	{
		uint32_t edgeCellsThisLevel = 1 << level;
		for (uint32_t y = 0; y < edgeCellsThisLevel; y++)
		{
			for (uint32_t x = 0; x < edgeCellsThisLevel; x++)
			{
				if (model->handleAtThisLevel(level, x, y))
				{

					//merge (_cellsPerSide / edgeCellsThisLevel) into one hog and correlate it
					uint32_t edgeCellsToMerge = (a->_cellsPerSide / edgeCellsThisLevel);

					float historgramA[_numBins], historgramB[_numBins];
					for (uint32_t z = 0; z < _numBins; z++)
					{
						historgramA[z] = historgramB[z] = 0.0f;
					}

					for (uint32_t y2 = 0; y2 < edgeCellsToMerge; y2++)
					{
						for (uint32_t x2 = 0; x2 < edgeCellsToMerge; x2++)
						{
							for (uint32_t z = 0; z < _numBins; z++)
							{
								historgramA[z] = a->_values[((y * edgeCellsToMerge * a->_cellsPerSide) + (x * edgeCellsToMerge) + (y2 * a->_cellsPerSide) + x2) * _numBins + z];
								historgramB[z] = b->_values[((y * edgeCellsToMerge * a->_cellsPerSide) + (x * edgeCellsToMerge) + (y2 * a->_cellsPerSide) + x2) * _numBins + z];
							}
						}
					}
					for (uint32_t z = 0; z < _numBins; z++)
					{
						score += ((float) historgramA[z] * (float) historgramB[z]) / ((float) edgeCellsToMerge * (float) edgeCellsToMerge);
					}
				}
			}
		}

	}
	return score;
}

uint32_t CHog::write(FILE* fh)
{
	uint32_t bytesWritten = 0;
	bytesWritten += fwrite(&_magic, 1, sizeof(_magic), fh);
	bytesWritten += fwrite(&_createdAt, 1, sizeof(_createdAt), fh);
	bytesWritten += fwrite(&_lastBestMatch, 1, sizeof(_lastBestMatch), fh);
	bytesWritten += fwrite(&_active, 1, sizeof(_active), fh);
	uint32_t valueBytesWritten = 0;
	uint32_t bob = 0;
	while((bob = fwrite(_values + valueBytesWritten, 1, sizeof(_values[0]) * _numBins * _cellsPerSide * _cellsPerSide - valueBytesWritten, fh)))
	{
		valueBytesWritten += bob;
	}
	bytesWritten += valueBytesWritten;
	return bytesWritten;
}

uint32_t CHog::getSizeBytes()
{
	return sizeof(_magic) + sizeof(_createdAt) + sizeof(_lastBestMatch) + sizeof(_active) + (sizeof(_values[0]) * (uint32_t)_numBins * _cellsPerSide * _cellsPerSide);
}

void CHog::setMostRecentMatch(uint32_t programCounter)
{
	_lastBestMatch = programCounter;
}

void CHog::setActive(bool isActive)
{
	_active = isActive;
}

uint32_t CHog::getCreatedAt()
{
	return _createdAt;
}

uint32_t CHog::getLastBestMatch()
{
	return _lastBestMatch;
}

bool CHog::isActive()
{
	return _active;
}

