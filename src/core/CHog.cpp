#include "CHog.h"

#include <opencv2/core/operations.hpp>
#include <opencv2/core/types_c.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>
#include <cmath>
#include <cstdint>

#include "../../include/settings.h"
#include "CModel.h"

namespace{
//for float -> uint16_t conversions
const float SCALING_VALUE = 65535.0f;
const uint32_t MAGIC = 0x2955a6a5;
}

CHog::CHog() :
		_values( NULL),
		_createdAt(0),
		_lastBestMatch(0),
		_numHits(0)
{
	_values = new uint16_t[HOG_NUM_BINS * HOG_NUM_CELLS * HOG_NUM_CELLS];
}

CHog::CHog(FILE* fh) :
		_values( NULL ),
		_createdAt( 0 ),
		_lastBestMatch( 0 ),
		_numHits( 0 )
{
	_values = new uint16_t[HOG_NUM_BINS * HOG_NUM_CELLS * HOG_NUM_CELLS];

	if (fh == NULL)
	{
		//fill with zeros
		memset(_values, 0, sizeof(*_values) * HOG_NUM_BINS * HOG_NUM_CELLS * HOG_NUM_CELLS);
	}
	else
	{
		uint32_t tmp = 0xdeadbeef;

		fread(&tmp, 1, sizeof(tmp), fh);

		if (tmp != MAGIC)
		{
			printf("%s::%s Error reading HOGs from file: data is corrupted (%08x != %08x)\n", __FILE__, __FUNCTION__, tmp, MAGIC);
			throw 1;
		}

		fread(&_createdAt, 1, sizeof(_createdAt), fh);
		fread(&_lastBestMatch, 1, sizeof(_lastBestMatch), fh);
		fread(&_numHits, 1, sizeof(_numHits), fh);

		fread(_values, sizeof(*_values), HOG_NUM_BINS * HOG_NUM_CELLS * HOG_NUM_CELLS, fh);
	}
}

CHog::CHog(cv::Mat image, uint32_t programCounter) :
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

	_values = new uint16_t[HOG_NUM_BINS * HOG_NUM_CELLS * HOG_NUM_CELLS];

	uint32_t width = HOG_CELL_SIZE * HOG_NUM_CELLS;
	for (uint32_t sqy = 0; sqy < HOG_NUM_CELLS; sqy++)
	{
		for (uint32_t sqx = 0; sqx < HOG_NUM_CELLS; sqx++)
		{
			float histogram[8];
			for (uint32_t x = 0; x < HOG_NUM_BINS; x++)
			{
				histogram[x] = 0.0f;
			}
			for (uint32_t y = 0; y < HOG_NUM_BINS; y++)
			{
				for (uint32_t x = 0; x < HOG_NUM_BINS; x++)
				{
					uint32_t px = sqx * HOG_NUM_BINS + x;
					uint32_t py = sqy * HOG_NUM_BINS + y;

					float angle = atan2f((float) reinterpret_cast<int16_t*>(sobelYImage.data)[py * width + px], (float) reinterpret_cast<int16_t*>(sobelXImage.data)[py * width + px]);

					float magnitude = sqrtf((float) ((float) sobelYImage.data[py * width + px] * (float) sobelYImage.data[py * width + px]) + ((float) sobelXImage.data[py * width + px] * (float) sobelXImage.data[py * width + px]));

					uint32_t index = (uint32_t) floor(4 * (0.99999f + angle / (float) M_PI));

					histogram[index] += magnitude;
				}
			}

			//normalise histograms
			float sum = 0.0f;
			for (uint32_t z = 0; z < HOG_NUM_BINS; z++)
			{
				sum += histogram[z];
			}
			for (uint32_t z = 0; z < HOG_NUM_BINS; z++)
			{
				_values[(sqy * HOG_NUM_CELLS + sqx) * HOG_NUM_BINS + z] = (histogram[z] / sum) * SCALING_VALUE;
			}
		}
	}
}

CHog::CHog(const CHog& other)
{
	_createdAt = other._createdAt;
	_lastBestMatch = other._lastBestMatch;
	_numHits = other._numHits;

	_values = new uint16_t[HOG_NUM_BINS * HOG_NUM_CELLS * HOG_NUM_CELLS];

	memcpy(_values, other._values, HOG_NUM_BINS * HOG_NUM_CELLS * HOG_NUM_CELLS * sizeof(*_values));
}

CHog& CHog::operator=(const CHog& other)
{
	_createdAt = other._createdAt;
	_lastBestMatch = other._lastBestMatch;
	_numHits = other._numHits;

	memcpy(_values, other._values, HOG_NUM_BINS * HOG_NUM_CELLS * HOG_NUM_CELLS * sizeof(*_values));

	return *this;
}

CHog::~CHog()
{
	delete[] _values;
}

float CHog::Correlate(CHog& a, CHog& b, CModel* model)
{
	float score = 0;

	uint32_t numLevels = model->getNumLevels();
	uint32_t numHistograms = 0;

	for (uint32_t level = 0; level < numLevels; level++)
	{
		uint32_t edgeCellsThisLevel = 1 << level;
		uint32_t edgeCellsToMerge = (HOG_NUM_CELLS / edgeCellsThisLevel);
		for (uint32_t y = 0; y < edgeCellsThisLevel; y++)
		{
			for (uint32_t x = 0; x < edgeCellsThisLevel; x++)
			{
				if (!model->isHandledAtHigherLevel(level, x, y) && model->isHandledAtThisLevel(level, x, y))
				{
					//merge (HOG_NUM_CELLS / edgeCellsThisLevel) into one hog and correlate it

					numHistograms++;

					float histogramA[HOG_NUM_BINS], histogramB[HOG_NUM_BINS];
					for (uint32_t z = 0; z < HOG_NUM_BINS; z++)
					{
						histogramA[z] = histogramB[z] = 0.0f;
					}

					for (uint32_t y2 = 0; y2 < edgeCellsToMerge; y2++)
					{
						for (uint32_t x2 = 0; x2 < edgeCellsToMerge; x2++)
						{
							uint32_t index = ((y * edgeCellsToMerge + y2) * HOG_NUM_CELLS) + ((x * edgeCellsToMerge + x2)) * HOG_NUM_BINS;

							for (uint32_t z = 0; z < HOG_NUM_BINS; z++)
							{
								histogramA[z] += (float)(a._values[index + z]);
								histogramB[z] += (float)(b._values[index + z]);
							}
						}
					}

					for (uint32_t z = 0; z < HOG_NUM_BINS; z++)
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
	bytesWritten += fwrite(&MAGIC, 1, sizeof(MAGIC), fh);
	bytesWritten += fwrite(&_createdAt, 1, sizeof(_createdAt), fh);
	bytesWritten += fwrite(&_lastBestMatch, 1, sizeof(_lastBestMatch), fh);
	bytesWritten += fwrite(&_numHits, 1, sizeof(_numHits), fh);
	uint32_t valueBytesWritten = 0;
	uint32_t bob = 0;
	while((bob = fwrite(_values + valueBytesWritten, 1, sizeof(_values[0]) * (uint32_t)HOG_NUM_BINS * HOG_NUM_CELLS * HOG_NUM_CELLS - valueBytesWritten, fh)))
	{
		valueBytesWritten += bob;
	}
	bytesWritten += valueBytesWritten;

	return bytesWritten;
}

uint32_t CHog::GetSizeBytes()
{
	return sizeof(MAGIC) + sizeof(_createdAt) + sizeof(_lastBestMatch) + sizeof(_numHits) + (sizeof(*_values) * HOG_NUM_BINS * HOG_NUM_CELLS * HOG_NUM_CELLS);
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

bool CHog::CompareCreated(const CHog& left, const CHog& right)
{
	return left._createdAt <= right._createdAt;
}

const uint16_t* CHog::getHistogram(uint32_t x, uint32_t y) const
{
	return _values + ((y * HOG_NUM_CELLS + x) * HOG_NUM_BINS);
}

