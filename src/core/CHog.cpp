#include "CHog.h"

#include <cmath>
#include "CModel.h"

#include <error.h>

#include <opencv2/core/mat.hpp>
#include <opencv2/core/operations.hpp>
#include <opencv2/imgproc/imgproc.hpp>

const uint32_t CHog::_magic = 0x2955a6a5;

namespace{
//for float -> uint16_t conversions
const float SCALING_VALUE = 65535.0f;
}

CHog::CHog(uint32_t cellSize, uint32_t numCellsPerSide, FILE* fh) :
		_cellSize( cellSize ),
		_cellsPerSide( numCellsPerSide ),
		_values( NULL ),
		_createdAt( 0 ),
		_lastBestMatch( 0 ),
		_numHits( 0 )
{
	_values = new uint16_t[_numBins * _cellsPerSide * _cellsPerSide];

	if (fh == NULL)
	{
		//fill with zeros
		memset(_values, 0, sizeof(_values[0]) * _numBins * _cellsPerSide * _cellsPerSide);
	}
	else
	{
		//read from file handle
		_values = new uint16_t[_numBins * _cellsPerSide * _cellsPerSide];

		uint32_t tmp = 0xdeadbeef;

		fread(&tmp, 1, sizeof(tmp), fh);

		if (tmp != _magic)
		{
			printf("%s::%s Error reading HOGs from file: data is corrupted (%08x != %08x)\n", __FILE__, __FUNCTION__, tmp, _magic);
			throw 1;
		}

		fread(&_createdAt, 1, sizeof(_createdAt), fh);
		fread(&_lastBestMatch, 1, sizeof(_lastBestMatch), fh);
		fread(&_numHits, 1, sizeof(_numHits), fh);

		fread(_values, 1, sizeof(_values[0]) * _numBins * _cellsPerSide * _cellsPerSide, fh);

	}
}

CHog::CHog(uint32_t cellSize, uint32_t numCellsPerSide, cv::Mat image, uint32_t programCounter) :
		_cellSize( cellSize ),
		_cellsPerSide( numCellsPerSide ),
		_values( NULL ),
		_createdAt( programCounter ),
		_lastBestMatch( programCounter ),
		_numHits( 0 )
{
	cv::Mat greyImage;
	cv::Mat smoothedImage;
	cv::Mat sobelXImage;
	cv::Mat sobelYImage;

	cv::cvtColor(image, greyImage, CV_RGB2GRAY);

	cv::GaussianBlur(greyImage, greyImage, cv::Size(7, 7), 0.5, 0.5);

	cv::Sobel(greyImage, sobelXImage, CV_16S, 1, 0, 3);
	cv::Sobel(greyImage, sobelYImage, CV_16S, 0, 1, 3);

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

					float angle = atan2f((float) reinterpret_cast<int16_t*>(sobelYImage.data)[py * width + px], (float) reinterpret_cast<int16_t*>(sobelXImage.data)[py * width + px]);

					float magnitude = sqrtf((float) ((float) sobelYImage.data[py * width + px] * (float) sobelYImage.data[py * width + px]) + ((float) sobelXImage.data[py * width + px] * (float) sobelXImage.data[py * width + px]));

					uint32_t index = (uint32_t) floor(4 * (0.99999f + angle / (float) M_PI));

					histogram[index] += magnitude;
				}
			}

			//normalise histograms
			float sum = 0.0f;
			for (uint32_t z = 0; z < _numBins; z++)
			{
				sum += histogram[z];
			}
			for (uint32_t z = 0; z < _numBins; z++)
			{
				_values[(sqy * _cellsPerSide + sqx) * _numBins + z] = (histogram[z] / sum) * SCALING_VALUE;
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
		printf("Both CHogs must have the same dimensions");
		throw 1;
	}
	return *this;
}

float CHog::Correlate(CHog* a, CHog* b, CModel* model)
{
	float score = 0;

	uint32_t numLevels = model->getNumLevels();
	uint32_t numHistograms = 0;

	for (uint32_t level = 0; level < numLevels; level++)
	{
		uint32_t edgeCellsThisLevel = 1 << level;
		uint32_t edgeCellsToMerge = (a->_cellsPerSide / edgeCellsThisLevel);
		for (uint32_t y = 0; y < edgeCellsThisLevel; y++)
		{
			for (uint32_t x = 0; x < edgeCellsThisLevel; x++)
			{
				if (!model->isHandledAtHigherLevel(level, x, y) && model->isHandledAtThisLevel(level, x, y))
				{
					//merge (_cellsPerSide / edgeCellsThisLevel) into one hog and correlate it

					numHistograms++;

					float histogramA[_numBins], histogramB[_numBins];
					for (uint32_t z = 0; z < _numBins; z++)
					{
						histogramA[z] = histogramB[z] = 0.0f;
					}

					for (uint32_t y2 = 0; y2 < edgeCellsToMerge; y2++)
					{
						for (uint32_t x2 = 0; x2 < edgeCellsToMerge; x2++)
						{
							for (uint32_t z = 0; z < _numBins; z++)
							{
								uint32_t index = ((y * edgeCellsToMerge + y2) * a->_cellsPerSide) + ((x * edgeCellsToMerge + x2)) * _numBins;
								histogramA[z] += (float)a->_values[index + z];
								histogramB[z] += (float)b->_values[index + z];
							}
						}
					}

					for (uint32_t z = 0; z < _numBins; z++)
					{
						//printf("HistogramA[%u] = %f, HistogramB[%u] = %f\n",  z, histogramA[z], z, histogramB[z]);
						score += ((float) histogramA[z] / SCALING_VALUE * (float) histogramB[z] / SCALING_VALUE) / (float)( edgeCellsToMerge * edgeCellsToMerge * edgeCellsToMerge * edgeCellsToMerge);
					}
				}
			}
		}

	}
	return score / numHistograms;
}

uint32_t CHog::write(FILE* fh)
{
	uint32_t bytesWritten = 0;
	bytesWritten += fwrite(&_magic, 1, sizeof(_magic), fh);
	bytesWritten += fwrite(&_createdAt, 1, sizeof(_createdAt), fh);
	bytesWritten += fwrite(&_lastBestMatch, 1, sizeof(_lastBestMatch), fh);
	bytesWritten += fwrite(&_numHits, 1, sizeof(_numHits), fh);
	uint32_t valueBytesWritten = 0;
	uint32_t bob = 0;
	while((bob = fwrite(_values + valueBytesWritten, 1, sizeof(_values[0]) * (uint32_t)_numBins * _cellsPerSide * _cellsPerSide - valueBytesWritten, fh)))
	{
		valueBytesWritten += bob;
	}
	bytesWritten += valueBytesWritten;

	return bytesWritten;
}

uint32_t CHog::getSizeBytes()
{
	return sizeof(_magic) + sizeof(_createdAt) + sizeof(_lastBestMatch) + sizeof(_numHits) + (sizeof(_values[0]) * (uint32_t)_numBins * _cellsPerSide * _cellsPerSide);
}

void CHog::setMostRecentMatch(uint32_t programCounter)
{
	_lastBestMatch = programCounter;
}

uint32_t CHog::getCreatedAt()
{
	return _createdAt;
}

uint32_t CHog::getLastBestMatch()
{
	return _lastBestMatch;
}

uint32_t CHog::getNumHits()
{
	return _numHits;
}

uint32_t CHog::incrementHits()
{
	return ++_numHits;
}

bool CHog::CompareCreated(CHog* left, CHog* right)
{
	return left->getCreatedAt() <= right->getCreatedAt();
}

const uint16_t* CHog::getHistogram(uint32_t x, uint32_t y) const
{
	return _values + ((y * _cellsPerSide + x) * _numBins);
}

