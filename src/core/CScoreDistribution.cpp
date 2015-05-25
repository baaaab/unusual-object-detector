#include "CScoreDistribution.h"

#include <string.h>
#include <stdio.h>
#include <algorithm>

const float CScoreDistribution::PERCENTILE = 10.0f;

CScoreDistribution::CScoreDistribution(const char* scoreFilename, uint32_t numScores) :
	_fh( NULL ),
	_index( 0 )
{
	_scores.resize(numScores);
	_fh = fopen(scoreFilename, "r+b");
	if(!_fh)
	{
		printf("%s::%s Failed to open %s\n", __FILE__, __FUNCTION__, scoreFilename);
	}

	//fill array from file
	fseek(_fh, 0, SEEK_SET);
	uint32_t numRead = fread(_scores.data(), sizeof(float), numScores, _fh);
	if (numRead != numScores)
	{
		printf("%s::%s Scores file corrupted (%u / %u)\n", __FILE__, __FUNCTION__, numRead, numScores);
		throw 1;
	}

	//move any zeroes to the beginning to be overwritten
	std::sort(_scores.begin(), _scores.end());
}

CScoreDistribution::~CScoreDistribution()
{
	fclose(_fh);
}

bool CScoreDistribution::isScoreLow(float score)
{
	std::vector<float> sorted = _scores;
	std::sort(sorted.begin(), sorted.end());

	auto firstGreaterThanScore = std::lower_bound(sorted.begin(), sorted.end(), score);

	float percentile = 100.0f * (firstGreaterThanScore - sorted.begin()) / (float)_scores.size();

	return percentile < PERCENTILE;
}

void CScoreDistribution::logScore(float score)
{
	_scores.at(_index) = score;

	writeToFile();

	_index = (_index + 1) % _scores.size();
}

void CScoreDistribution::getScoreDistribution(uint32_t* index, std::vector<float>* scores) const
{
	*index = _index;
	*scores = _scores;
}

void CScoreDistribution::writeToFile()
{
	//write value at index
	fseek(_fh, sizeof(float) * _index, SEEK_SET);
	uint32_t numWrites = fwrite(&_scores.at(_index), sizeof(float), 1, _fh);

	if(numWrites != 1)
	{
		printf("%s::%s write failed\n", __FILE__, __FUNCTION__);
		perror("");
	}
}


