#include "ServerFinderThread.h"
#include "opznet/client.h"

#include <WinSock2.h>
#include <algorithm>

namespace opznet
{

void ServerFinderThread::S_Create()
{
	CREATE_SINGLETON(new ServerFinderThread());
}

void ServerFinderThread::S_Delete()
{
	DELETE_SINGLETON;
}

bool ServerFinderThread::S_BeginFind()
{
	LOCK(_instance->_state_mutex);
	if(_instance->_state != WAITING) return false;
	
	_instance->_state = FINDING;
	
	_instance->Resume();
	
	return true;
}

void ServerFinderThread::Run()
{
	while(1)
	{
		if(_state == WAITING)
			Suspend();
		else if(_state == FINDING)
		{
			Find();
		}
	}
}

ServerFinderThread *ServerFinderThread::_instance = nullptr;

#pragma warning(disable : 4351) // 배열의 요소 기본 초기화 관련 경고...

ServerFinderThread::ServerFinderThread(void) :
	opzThread(),
	_state(WAITING),
	_state_mutex(),
	_sock(),
	_secondary_port(),
	_game_name(),
	_broadcast_addresses()
{
#pragma warning(default : 4351)
	_instance = this;

	_sock.setBlocking(false);

	_secondary_port = 
		static_cast<unsigned short>(
		GetPrivateProfileInt(L"GENERAL", L"SECONDARY_PORT", 55878, L"./game.ini"));
	
	GetPrivateProfileString(L"GENERAL", L"NAME", L"UNKNOWN", _game_name, GAME_NAME_BUF_LENGTH, L"./game.ini");

	if(!GetBroadcastAddress(&_broadcast_addresses))
	{
#ifdef _DEBUG
	wprintf(L"[오류] 브로드캐스트 주소 읽기 실패\n");
#endif
	}
}

ServerFinderThread::~ServerFinderThread(void)
{
}

void ServerFinderThread::Find()
{
	sf::Packet packet;
	packet << CONST_ANY_SERVER_THERE;
	packet << _game_name;

	for(auto i = _broadcast_addresses.begin(); i != _broadcast_addresses.end(); ++i)
	{
		if(_sock.send(packet, *i, _secondary_port) != sf::Socket::Done)
		{
			auto new_msg = new client_msg_t;
			new_msg->type = CLMSG_UNEXPECTED_ERROR;
			msg_queue.Push(new_msg);
		}
	}

	sf::Clock clock;
	clock.restart();

	while(clock.getElapsedTime().asSeconds() < 5.f)
	{
		sf::Packet		recv_packet;
		sf::IpAddress	remote_address;
		unsigned short	remote_port;

		auto res = _sock.receive(recv_packet, remote_address, remote_port);
		if(res != sf::Socket::Done)
		{
			if(res == sf::Socket::Error)
			{
				auto new_msg = new client_msg_t;
				new_msg->type = CLMSG_UNEXPECTED_ERROR;
				msg_queue.Push(new_msg);
				break;
			}
			else
			{
				Sleep(500);
				continue;
			}
		}

		unsigned short header;
		if(!(recv_packet >> header))
			continue;
		if(header != CONST_HERE_IS_SERVER)
			continue;

		wstring server_game_name;
		if(!(recv_packet >> server_game_name))
			continue;
		if(wcscmp(server_game_name.c_str(), _game_name))
			continue;

		string address_string = remote_address.toString();

		auto new_msg = new client_msg_t;
		new_msg->type = CLMSG_LOCAL_SERVER_FOUND;
		new_msg->server_info.address.assign(address_string.begin(), address_string.end());
		if(!(recv_packet >> new_msg->server_info.name))
		{
			delete new_msg;
			continue;
		}
		if(!(recv_packet >> new_msg->server_info.num_of_client))
		{
			delete new_msg;
			continue;
		}
		msg_queue.Push(new_msg);
	}

	auto new_msg = new client_msg_t;
	new_msg->type = CLMSG_FINDING_LOCAL_SERVER_OVER;
	msg_queue.Push(new_msg);
	_state = WAITING;
}

}