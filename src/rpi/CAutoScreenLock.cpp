#include "CAutoScreenLock.h"

CAutoScreenLock::CAutoScreenLock(SDL_Surface* screen) :
		_screen(screen)
{
	lockScreen();
}

CAutoScreenLock::~CAutoScreenLock()
{
	if (_screen)
	{
		unlockScreen();
	}
}

void CAutoScreenLock::release()
{
	if (_screen)
	{
		unlockScreen();
		_screen = NULL;
	}
}

void CAutoScreenLock::lockScreen()
{
	if (SDL_MUSTLOCK(_screen))
	{
		SDL_LockSurface(_screen);
	}
}

void CAutoScreenLock::unlockScreen()
{
	if (SDL_MUSTLOCK(_screen))
	{
		SDL_UnlockSurface(_screen);
	}
}

