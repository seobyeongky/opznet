#pragma once

#include "internal.h"

namespace opznet
{

	bool GetBroadcastAddress(list<sf::IpAddress> *address_list);

class ServerFinderThread : opzThread
{
public:
	static void S_Create();
	static void S_Delete();

	static bool S_BeginFind();

protected:
	void Run();

private:
	static ServerFinderThread *_instance;

	enum
	{
		WAITING,
		FINDING
	}					_state;

	opzMutex			_state_mutex;

	sf::UdpSocket		_sock;

	unsigned short		_secondary_port;
	WCHAR				_game_name[GAME_NAME_BUF_LENGTH];

	list<sf::IpAddress>	_broadcast_addresses;
	
	ServerFinderThread(void);
	~ServerFinderThread(void);

	void Find();
};

}