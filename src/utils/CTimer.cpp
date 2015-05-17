#include "CTimer.h"

#include <time.h>
#include <sys/time.h>

CTimer::CTimer()
{
	struct timeval now;
	gettimeofday(&now, NULL);

	_startSeconds = now.tv_sec;
	_startUSeconds = now.tv_usec;

}

CTimer::~CTimer()
{
}

uint32_t CTimer::reset()
{
	struct timeval now;
	gettimeofday(&now, NULL);

	uint32_t nowSeconds = now.tv_sec;
	uint32_t nowUSeconds = now.tv_usec;

	uint32_t ret = (nowSeconds - _startSeconds) * 1000000 + (nowUSeconds - _startUSeconds);

	_startSeconds = nowSeconds;
	_startUSeconds = nowUSeconds;

	return ret;
}

uint32_t CTimer::getElapsedUSecs()
{
	struct timeval now;
	gettimeofday(&now, NULL);

	uint32_t nowSeconds = now.tv_sec;
	uint32_t nowUSeconds = now.tv_usec;

	return (nowSeconds - _startSeconds) * 1000000 + (nowUSeconds - _startUSeconds);
}

