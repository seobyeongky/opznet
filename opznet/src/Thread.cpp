#include "thread.h"

#include <cstdio>
#include <cassert>

opzThread::opzThread(unsigned int stackSize)
	: _t(&ThreadFunc, this),
	_suspended(true)
{
}

opzThread::~opzThread()
{
}

void opzThread::Join()
{
    assert(_t.joinable());
    _t.join();
}

void opzThread::Begin()
{
    _t.detach();
}

unsigned int __stdcall opzThread::ThreadFunc(void *args)
{
	opzThread *pThread = reinterpret_cast<opzThread*>(args);
#ifdef _DEBUG
	printf("[ThreadFunc] pThread : %d\n", pThread);
#endif
	if (pThread)
		pThread->Run();

	pThread->EndProcess();

    return 0;
} 