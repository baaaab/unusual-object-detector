#ifndef CHOG_H_
#define CHOG_H_

#include <inttypes.h>
#include <cstdio>
#include <opencv2/core/core.hpp>

class CModel;

class CHog
{
public:
	CHog(uint32_t cellSize, uint32_t numCellsPerSide, FILE* fh);
	CHog(uint32_t cellSize, uint32_t numCellsPerSide, cv::Mat sobelX, cv::Mat sobelY, uint32_t programCounter);
	~CHog();

	CHog& operator=(const CHog &);

	static float Correlate(CHog* a, CHog* b, CModel* model);

	uint32_t write(FILE* fh);
	uint32_t getSizeBytes();

	void setMostRecentMatch(uint32_t programCounter);
	void setActive(bool isActive);

	uint32_t getCreatedAt();
	uint32_t getLastBestMatch();
	bool isActive();

private:
	uint32_t _cellSize;
	uint32_t _cellsPerSide;
	uint16_t* _values;
	static const uint16_t _numBins = 8;

	uint32_t _createdAt;
	uint32_t _lastBestMatch;
	bool _active;

	static const uint32_t _magic;

};

#endif /* CHOG_H_ */
