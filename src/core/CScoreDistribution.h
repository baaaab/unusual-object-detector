#ifndef CSCOREDISTRIBUTION_H_
#define CSCOREDISTRIBUTION_H_

#include <inttypes.h>
#include <cstdio>
#include <vector>

class CScoreDistribution
{
public:
	CScoreDistribution(const char* scoreFilename, uint32_t numScores);
	virtual ~CScoreDistribution();

	bool isScoreLow(float score);
	void logScore  (float score);

	void getScoreDistribution(uint32_t* index, std::vector<float>* scores) const;

private:
	FILE*              _fh;

	std::vector<float> _scores;
	uint32_t           _index;

	static const float PERCENTILE;

	void writeToFile();
};

#endif /* CSCOREDISTRIBUTION_H_ */
