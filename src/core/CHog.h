#ifndef CHOG_H_
#define CHOG_H_

#include <opencv2/core/core.hpp>
#include <cstdint>
#include <cstdio>
#include <vector>

class CModel;

class CHog
{
	friend class CHogStore;
public:

	CHog();
	CHog(cv::Mat image, uint32_t programCounter);
	CHog& operator=(const CHog& other);
	CHog(const CHog& other);
	virtual ~CHog();

	void computeRCH(CModel* model);

	static float Correlate(CHog& a, CHog& b, CModel* model);
	void setMostRecentMatch(uint32_t programCounter);

	uint32_t getCreatedAt();
	uint32_t getLastBestMatch();

	uint32_t getNumHits();
	uint32_t incrementHits();

	static bool CompareCreated(const CHog& left, const CHog& right);

	const uint16_t* getHistogram(uint32_t x, uint32_t y) const;

	bool read(FILE* fh);
	uint32_t write(FILE* fh);

private:
	//accessible form CHogStore
	CHog(FILE* fh);
	void replace(CHog& other);
	static uint32_t GetSizeBytes();

	std::vector<uint16_t> _rch;
	uint16_t* _values;
	uint32_t _createdAt;
	uint32_t _lastBestMatch;
	uint32_t _numHits;

};

#endif /* CHOG_H_ */
