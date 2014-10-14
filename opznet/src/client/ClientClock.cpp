#include "NetThread.h"

namespace opznet
{
	NetThread::ClientClock::ClientClock()
		: _clock(), _delay()
	{
	}

	NetThread::ClientClock::~ClientClock()
	{
	}

	void NetThread::ClientClock::Restart(int cur_time)
	{
		_delay = cur_time;
		_clock.restart();
	}

	int NetThread::ClientClock::GetTime()
	{
		return _delay + _clock.getElapsedTime().asMilliseconds();
	}
}