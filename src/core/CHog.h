#ifndef CHOG_H_
#define CHOG_H_

#include <inttypes.h>
#include <stdio.h>
#include <opencv2/core/core.hpp>

class CModel;

class CHog
{
public:
	CHog(uint32_t cellSize, uint32_t numCellsPerSide, FILE* fh);
	CHog(uint32_t cellSize, uint32_t numCellsPerSide, cv::Mat image, uint32_t programCounter);
	virtual ~CHog();

	CHog& operator=(const CHog &);

	static float Correlate(CHog* a, CHog* b, CModel* model);

	uint32_t write(FILE* fh);
	uint32_t getSizeBytes();

	void setMostRecentMatch(uint32_t programCounter);

	uint32_t getCreatedAt();
	uint32_t getLastBestMatch();

	uint32_t getNumHits();
	uint32_t incrementHits();

	static bool CompareCreated(CHog* left, CHog* right);

	const uint16_t* getHistogram(uint32_t x, uint32_t y) const;

private:
	uint32_t _cellSize;
	uint32_t _cellsPerSide;
	uint16_t* _values;
	static const uint16_t _numBins = 8;

	uint32_t _createdAt;
	uint32_t _lastBestMatch;
	uint32_t _numHits = 0;

	static const uint32_t _magic;

};

#endif /* CHOG_H_ */
