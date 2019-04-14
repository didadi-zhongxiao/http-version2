#ifndef THREAD_WORK_H
#define THREAD_WORK_H
#include <pthread.h>
#include <deque>
//#include <string>
//pthread_mutex_t mutx;
class ThreadWork
{
public:
	ThreadWork() {};
	ThreadWork(int num);
	void addDeque(int fd);
private:
	int nThreads;
//	int listenfd;
	void threadsMake(int i);
	static void * threadMain(void *arg);
	static void work(int fd);
	std::deque<int> clientSockets;
};


#endif // 
