#include "CHog.h"

#include <opencv2/core/operations.hpp>
#include <opencv2/core/types_c.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>
#include <cmath>
#include <cstdint>
#ifdef __ARM_NEON__
#include <arm_neon.h>
#endif

#include "../../include/settings.h"
#include "CModel.h"

namespace
{
//for float -> uint16_t conversions
const float SCALING_VALUE = 65535.0f;
const uint32_t MAGIC = 0x2955a6a5;
float minSum = 999999999999999999.0f;
float maxSum = 0;
}

CHog::CHog() :
		_createdAt(0),
		_lastBestMatch(0),
		_numHits(0)
{
	_values.resize(HOG_NUM_BINS * HOG_NUM_CELLS * HOG_NUM_CELLS);
}

CHog::CHog(FILE* fh) :
		_createdAt(0),
		_lastBestMatch(0),
		_numHits(0)
{
	_values.resize(HOG_NUM_BINS * HOG_NUM_CELLS * HOG_NUM_CELLS);

	if (fh == NULL)
	{
		//fill with zeros
		std::fill(_values.begin(), _values.end(), 0);
	}
	else
	{
		if(!read(fh))
		{
			printf("%s::%s Error reading hog data file\n", __FILE__, __FUNCTION__);
			throw 1;
		};
	}
}

CHog::CHog(cv::Mat image, uint32_t programCounter) :
		_createdAt(programCounter),
		_lastBestMatch(programCounter),
		_numHits(0)
{
	_values.resize(HOG_NUM_BINS * HOG_NUM_CELLS * HOG_NUM_CELLS);

	cv::Mat greyImage;
	cv::Mat smoothedImage;
	cv::Mat sobelXImage;
	cv::Mat sobelYImage;

	cv::cvtColor(image, greyImage, CV_RGB2GRAY);

	cv::GaussianBlur(greyImage, greyImage, cv::Size(7, 7), 0.5, 0.5);

	cv::Sobel(greyImage, sobelXImage, CV_16S, 1, 0, 3);
	cv::Sobel(greyImage, sobelYImage, CV_16S, 0, 1, 3);

	for (uint32_t sqy = 0; sqy < HOG_NUM_CELLS; sqy++)
	{
		for (uint32_t sqx = 0; sqx < HOG_NUM_CELLS; sqx++)
		{
			float histogram[8];
			for (uint32_t z = 0; z < HOG_NUM_BINS; z++)
			{
				histogram[z] = 0.0f;
			}
			for (uint32_t y = 0; y < HOG_CELL_SIZE; y++)
			{
				for (uint32_t x = 0; x < HOG_CELL_SIZE; x++)
				{
					uint32_t px = sqx * HOG_CELL_SIZE + x;
					uint32_t py = sqy * HOG_CELL_SIZE + y;

					int16_t sobelXPixel = sobelXImage.at<int16_t>(py, px);
					int16_t sobelYPixel = sobelYImage.at<int16_t>(py, px);
					float angle = atan2f(sobelYPixel, sobelXPixel);

					float magnitude = sqrtf(((float)sobelYPixel * (float)sobelYPixel) + ((float)sobelXPixel * (float)sobelXPixel));

					uint32_t index = (uint32_t) floor((HOG_NUM_BINS/2) * (0.99999f + angle / (float) M_PI));

					histogram[index] += magnitude;
				}
			}

			//normalise histograms
			float sum = 0.0f;
			for (uint32_t z = 0; z < HOG_NUM_BINS; z++)
			{
				sum += histogram[z];
			}
			if(sum < minSum)
			{
				minSum = sum;

			}
			//printf("sum %f\n", sum);
			if(sum > maxSum)
			{
				maxSum = sum;
				//printf("New max: %f\n", maxSum);
			}
			sum = std::max(500.0f, sum);

			for (uint32_t z = 0; z < HOG_NUM_BINS; z++)
			{
				_values[(sqy * HOG_NUM_CELLS + sqx) * HOG_NUM_BINS + z] = (histogram[z] / sum) * SCALING_VALUE;
			}
		}
	}
}

CHog::~CHog()
{

}

void CHog::computeRCH(CModel* model)
{
	//clear old rch
	_rch = std::vector<float>();

	uint32_t numLevels = model->getNumLevels();

	for (uint32_t level = 0; level < numLevels; level++)
	{
		uint32_t edgeCellsThisLevel = 1 << level;
		uint32_t edgeCellsToMerge = (HOG_NUM_CELLS / edgeCellsThisLevel);

		for (uint32_t y = 0; y < edgeCellsThisLevel; y++)
		{
			for (uint32_t x = 0; x < edgeCellsThisLevel; x++)
			{
				if (model->isHandledAtThisLevel(level, x, y) && !model->isHandledAtHigherLevel(level, x, y))
				{
					//merge (HOG_NUM_CELLS / edgeCellsThisLevel) into one hog and correlate it

					float histogram[HOG_NUM_BINS];
					for (uint32_t z = 0; z < HOG_NUM_BINS; z++)
					{
						histogram[z] = 0.0f;
					}

					for (uint32_t y2 = 0; y2 < edgeCellsToMerge; y2++)
					{
						for (uint32_t x2 = 0; x2 < edgeCellsToMerge; x2++)
						{
							uint32_t index = (((y * edgeCellsToMerge + y2) * HOG_NUM_CELLS) + (x * edgeCellsToMerge + x2)) * HOG_NUM_BINS;

							for (uint32_t z = 0; z < HOG_NUM_BINS; z++)
							{
								histogram[z] += (float) (_values[index + z]);
							}
						}
					}

					for (uint32_t z = 0; z < HOG_NUM_BINS; z++)
					{
						_rch.push_back( histogram[z] / (edgeCellsToMerge * edgeCellsToMerge));
					}
				}
			}
		}
	}

	if(_rch.size() == 0)
	{
		printf("RCH is zero length\n");
	}
}

float multiplyAccumulateArrays(float* a, float* b, uint32_t length)
{
	float sum = 0;
	for (uint32_t h = 0; h < length; h ++)
	{
		sum += 65536.0f - fabsf(a[h] - b[h]);
	}
	return sum;
}

float __inline multiplyAccumulateArrays4(float* a, float* b, uint32_t length)
{
#ifdef __ARM_NEON__
	float32x4_t sn = vdupq_n_f32(0.0f);
	for (uint32_t h = 0; h < length; h += 8)
	{
		float32x4_t an = vld1q_f32(a + h);
		float32x4_t bn = vld1q_f32(b + h);
		sn = vfmaq_f32(sn, an, bn);

		an = vld1q_f32(a + h + 4);
		bn = vld1q_f32(b + h + 4);
		sn = vfmaq_f32(sn, an, bn);
	}

	float32x2_t l = vget_low_f32(sn);
	float32x2_t h = vget_high_f32(sn);
	float32x2_t s = vadd_f32(l, h);

	return  vget_lane_f32(s, 0) + vget_lane_f32(s, 1);
#else
	return multiplyAccumulateArrays(a, b, length);
#endif
}

float CHog::Correlate(CHog& a, CHog& b, CModel* model)
{
	if (a._rch.size() == 0 || b._rch.size() == 0 || a._rch.size() != b._rch.size())
	{
		printf("%s::%s invalid RCH lengths: %u vs %u\n", __FILE__, __FUNCTION__, (uint32_t)a._rch.size(), (uint32_t)b._rch.size());
		throw 1;
	}

	float score = multiplyAccumulateArrays(a._rch.data(), b._rch.data(), a._rch.size());

	return (float)score / (a._rch.size() * SCALING_VALUE * SCALING_VALUE);
}

float sumArraySquareDifferences(float* a, float* b, uint32_t length)
{
	float sum = 0;
	for (uint32_t h = 0; h < length; h ++)
	{
		sum += pow(a[h] - b[h], 2);
	}

	return sum;
}

float CHog::MeasureSimilarity(CHog& a, CHog& b, CModel* model)
{
	if (a._rch.size() == 0 || b._rch.size() == 0 || a._rch.size() != b._rch.size())
	{
		printf("%s::%s invalid RCH lengths: %u vs %u\n", __FILE__, __FUNCTION__, (uint32_t)a._rch.size(), (uint32_t)b._rch.size());
		throw 1;
	}

	float score = sumArraySquareDifferences(a._rch.data(), b._rch.data(), a._rch.size());

	return (float)score / (a._rch.size() * SCALING_VALUE * SCALING_VALUE);
}

bool CHog::read(FILE* fh)
{
	uint32_t tmp = 0xdeadbeef;

	bool failed = false;

	failed = failed || (fread(&tmp, 1, sizeof(tmp), fh) != sizeof(tmp));

	if (failed || tmp != MAGIC)
	{
		printf("%s::%s Error reading HOGs from file: data is corrupted (%08x != %08x)\n", __FILE__, __FUNCTION__, tmp, MAGIC);
		throw 1;
	}



	failed = failed || (fread(&_createdAt, 1, sizeof(_createdAt), fh) != sizeof(_createdAt));
	failed = failed || (fread(&_lastBestMatch, 1, sizeof(_lastBestMatch), fh) != sizeof(_lastBestMatch));
	failed = failed || (fread(&_numHits, 1, sizeof(_numHits), fh) != sizeof(_numHits));

	if(failed)
	{
		printf("%s::%s Error reading HOGs from file: read error\n", __FILE__, __FUNCTION__);
		return false;
	}

	return fread(_values.data(), sizeof(_values[0]), HOG_NUM_BINS * HOG_NUM_CELLS * HOG_NUM_CELLS, fh) == HOG_NUM_BINS * HOG_NUM_CELLS * HOG_NUM_CELLS;
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
	while ((bob = fwrite(_values.data() + valueBytesWritten, 1, sizeof(_values[0]) * (uint32_t) HOG_NUM_BINS * HOG_NUM_CELLS * HOG_NUM_CELLS - valueBytesWritten, fh)))
	{
		valueBytesWritten += bob;
	}
	bytesWritten += valueBytesWritten;

	return bytesWritten;
}

std::vector<uint16_t> CHog::getHOG()
{
	return _values;
}

std::vector<float> CHog::getRCH()
{
	return _rch;
}

uint32_t CHog::GetSizeBytes()
{
	return sizeof(MAGIC) + sizeof(_createdAt) + sizeof(_lastBestMatch) + sizeof(_numHits) + (sizeof(_values[0]) * HOG_NUM_BINS * HOG_NUM_CELLS * HOG_NUM_CELLS);
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
	return _values.data() + ((y * HOG_NUM_CELLS + x) * HOG_NUM_BINS);
}

