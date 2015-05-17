#ifndef CTIMER_H_
#define CTIMER_H_

#include <inttypes.h>

class CTimer
{
public:
	CTimer();
	virtual ~CTimer();

	uint32_t getElapsedUSecs();
	uint32_t reset();

private:

	uint32_t _startSeconds;
	uint32_t _startUSeconds;

};

#endif /* CTIMER_H_ */
