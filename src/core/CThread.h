#ifndef CTHREAD_H_
#define CTHREAD_H_

namespace boost
{
	class thread;
}

class CThread
{
public:
	CThread(void (*start_routine) (void *), void* arg);
	virtual ~CThread();

private:
	boost::thread* _thread;
};

#endif /* CTHREAD_H_ */
