#ifndef THREAD_CLASS_HPP
#define THREAD_CLASS_HPP

#include <thread>
#include <mutex>

class opzThread
{
public:
	opzThread(unsigned int stackSize = 0);
	virtual ~opzThread();

	void			Begin();
	void			Join();
	
protected:
	virtual void	Run() = 0;
	virtual void	EndProcess() { }	//이 함수는 Run 쓰레드가 끝났을 때 호출됩니다.

private:
	static unsigned int ThreadFunc(void *args);
	
    std::thread _t;
    bool _suspended;
};

#define	LOCK(x)	opzLock __lock(x)

#endif
