#ifndef CAUTOSCREENLOCK_H_
#define CAUTOSCREENLOCK_H_

#include <SDL/SDL.h>

class CAutoScreenLock
{
public:
	CAutoScreenLock(SDL_Surface* screen);
	virtual ~CAutoScreenLock();

	void release();

private:
	void lockScreen();
	void unlockScreen();

	SDL_Surface* _screen;
};

#endif /* CAUTOSCREENLOCK_H_ */
