#include "CThread.h"

#include <boost/thread.hpp>

CThread::CThread(void (*start_routine) (void *), void* arg) :
_thread( NULL )
{
	_thread = new boost::thread(start_routine, arg);
}

CThread::~CThread()
{
	_thread->join();
	delete _thread;
}

