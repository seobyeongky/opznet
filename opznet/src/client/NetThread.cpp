#include "NetThread.h"

#include <iostream>

namespace opznet
{

const sf::Time connect_wait_time = sf::seconds(5.f);

void NetThread::S_Create()
{
	CREATE_SINGLETON(new NetThread);
}

void NetThread::S_Delete()
{
	DELETE_SINGLETON;
}

bool NetThread::S_GetClientInfo(ID id, client_t *client)
{
	LOCK(_instance->_mutex);

	if(_instance->_clients.find(id) == _instance->_clients.end())
	{
		//없는 경우입니다.
		return false;
	}

	auto info = _instance->_clients[id];
	*client = *info;
	
	return true;
}

bool NetThread::S_RequestConnectToServer(const std::wstring &server_address, const std::wstring &user_name)
{
	LOCK(_instance->_mutex);

	return _instance->ConnectToServer(server_address, user_name);
}

#define NO_WAY client_msg_t *new_msg = new client_msg_t; \
				new_msg->type = CLMSG_UNEXPECTED_ERROR; \
				msg_queue.Push(new_msg); \
				_state = WAITING;


void NetThread::Run()
{
	while(1)
	{
		switch(_state)
		{
		case WAITING:
			Suspend();
			break;

		case CONNECTING:
		{
			_tcp_sock.setBlocking(true);

			unsigned short secondary_port = static_cast<unsigned short>(
				GetPrivateProfileInt(L"GENERAL", L"SECONDARY_PORT", 0, L"./game.ini"));

			auto res = _tcp_sock.connect(*_server_address, secondary_port, connect_wait_time);	// 1. Connect
			if(res == sf::Socket::Status::Error)
			{
				client_msg_t *new_msg = new client_msg_t;
				new_msg->type = CLMSG_CONNECT_TRY_TIMEOUT;
				msg_queue.Push(new_msg);
				_state = WAITING;
				break;
			}

			assert(res == sf::Socket::Status::Done);
			
			sf::Packet greeting_packet;
			WCHAR game_name[GAME_NAME_BUF_LENGTH];
			GetPrivateProfileString(L"GENERAL", L"NAME", L"UNKNOWN", game_name, 64, L"./game.ini");
			greeting_packet <<  game_name;
			auto ptr = (char *)greeting_packet.getData();

			if(_tcp_sock.send(greeting_packet) != sf::Socket::Done)					// 2. Send Greeting
			{
				NO_WAY; break;
			}

			sf::Packet recv_packet;
			if(_tcp_sock.receive(recv_packet) != sf::Socket::Status::Done)			// 3. Recv Greeting
			{
				NO_WAY;	break;
			}

			wstring server_game_name;
			if(!(recv_packet >> server_game_name))	// 4. Extract Server's game name
			{
				NO_WAY;	break;
			}

			if(wcscmp(server_game_name.c_str(), game_name) != 0)
			{
				NO_WAY;	break;
			}

			sf::Packet info_packet;
			info_packet << _user.name.c_str();
			info_packet << _udp_local_port;
			
			if(_tcp_sock.send(info_packet) != sf::Socket::Done)
			{
				NO_WAY; break;
			}
			
			client_msg_t *new_msg = new client_msg_t;
			
			new_msg->type = CLMSG_CONNECTED;
			if(!(recv_packet
				>> new_msg->server_info.name
				>> new_msg->server_info.num_of_client
				>> _user.id))		// 5. Extract Server's Info
			{
				delete new_msg;
				NO_WAY; break;
			}
			
			string server_address_string = _server_address->toString();
			new_msg->server_info.address.assign(
				server_address_string.begin(),
				server_address_string.end());

			for(int i = 0; i < PING_PONG_TEST_NUMBER; ++i)			// 6. Ping Pong Test
			{
				if(_tcp_sock.receive(recv_packet) != sf::Socket::Done)
				{
					delete new_msg;
					NO_WAY; break;
				}
				if(_tcp_sock.send(_pong_msg) != sf::Socket::Done)
				{
					delete new_msg;
					NO_WAY; break;
				}
			}

			if(_tcp_sock.receive(recv_packet) != sf::Socket::Done)
			{
				delete new_msg;
				NO_WAY; break;
			}
			int delay, server_time;
			if(!(recv_packet >> delay >> server_time))
			{
				delete new_msg;
				NO_WAY; break;
			}
			new_msg->my_id = _user.id;

			_client_clock.Restart(delay + server_time);
			
			{
				LOCK(_mutex);
				
				msg_queue.Push(new_msg);
			
				_state = CONNECTED;
			}

			break;
		}
			
		case CONNECTED:
			CommunicateWithServer();
			break;
		}
	}
}

NetThread *NetThread::_instance = nullptr;

NetThread::NetThread(void)
	: _user(),
	_tcp_sock(),
	_udp_sock(),
	_clients(),
	_state(WAITING),
	_mutex(),
	_server_address(nullptr),
	_pong_msg(),
	_disconnect_switch(false),
	_client_clock(),
	_udp_local_port()
{
	_instance = this;

	_tcp_sock.setBlocking(true);

	_udp_sock.setBlocking(false);
	_udp_sock.bind(sf::Socket::AnyPort);
	_udp_local_port = _udp_sock.getLocalPort();

	_pong_msg << CONST_PONG;
}

NetThread::~NetThread(void)
{
	for(auto i = _clients.begin(); i != _clients.end(); ++i)
		delete i->second;

	if(_server_address)
		delete _server_address;
}

bool NetThread::ConnectToServer(const std::wstring &server_address, const std::wstring &user_name)
{
	if(_state != WAITING) return false;
	string addr;
	addr.assign(server_address.begin(), server_address.end());
	_server_address = new sf::IpAddress(addr.c_str());
	_user.name = user_name;
	_state = CONNECTING;
	
	Resume();

	return true;
}

void NetThread::CommunicateWithServer()
{
	sf::IpAddress	server_address;
	unsigned short	server_primary_port;

	_tcp_sock.setBlocking(false);

	do
	{
		HandleClientsideUserRequests();
	
		sf::Packet	recv_packet;
		bool		loop_over = false;

		do
		{
			auto res = _tcp_sock.receive(recv_packet);
			switch(res)
			{
			case sf::Socket::Done:
				HandleRecvPacket(recv_packet);
				break;
		
			case sf::Socket::NotReady:
				loop_over = true;
				break;
		
			case sf::Socket::Error:
				_disconnect_switch = true;
				loop_over = true;
				break;
			}
		}while(!loop_over);

		loop_over = false;
		do
		{
			auto res = _udp_sock.receive(recv_packet, server_address, server_primary_port);
			switch(res)
			{
			case sf::Socket::Done:
				HandleRecvPacket(recv_packet);
				break;
		
			case sf::Socket::NotReady:
				loop_over = true;
				break;
		
			case sf::Socket::Error:
				_disconnect_switch = true;
				loop_over = true;
				break;
			}
		}while(!loop_over);

		while(!garbages.CheckEmpty())
		{
			client_msg_t *msg;
			garbages.Pop(&msg);
			delete msg;
		}

		if(_disconnect_switch) break;

		Sleep(1);

	}while(true);

	{
		LOCK(_mutex);
		client_msg_t *new_msg = new client_msg_t;
		new_msg->type = CLMSG_DISCONNECTED;
		msg_queue.Push(new_msg);

		sf::Packet good_bye_packet;
		good_bye_packet << CONST_GOOD_BYE;
		_tcp_sock.send(good_bye_packet);
		_tcp_sock.disconnect();
		
		_state = WAITING;
		_disconnect_switch = false;
	}
}

void NetThread::HandleClientsideUserRequests()
{
	while(!letters.CheckEmpty())
	{
		sf::Packet packet;

		packet << CONST_CLIENTSIDE_USER_LEVEL_MSG;
		string *str;
		letters.Pop(&str);
		packet << *str;
		delete str;
		if(_tcp_sock.send(packet) != sf::Socket::Done)
			_disconnect_switch = true;
	}
}

void NetThread::HandleRecvPacket(sf::Packet &recv_packet)
{
	unsigned short header;
	recv_packet >> header;

	switch(header)
	{
		case PING:
		if(_tcp_sock.send(_pong_msg) != sf::Socket::Done)
			_disconnect_switch = true;
		break;

		case CLIENT_INTRO:
		{
			LOCK(_mutex);

			client_t *new_client = new client_t;
				
			if(!(recv_packet >> new_client->id >> new_client->name >> new_client->is_member))
			{
				delete new_client;
				break;
			}
				
			_clients[new_client->id] = new_client;

			client_msg_t *new_msg = new client_msg_t;
			new_msg->type = CLMSG_CLIENT_INTRODUCTION;
			new_msg->client_info = *new_client;
			msg_queue.Push(new_msg);
		}
		break;

		case CLIENT_GONE:
		{
			LOCK(_mutex);

			ID id;
			if(!(recv_packet >> id))
				break;
				
			client_msg_t *new_msg = new client_msg_t;
			new_msg->type = CLMSG_CLIENT_GONE;
			new_msg->client_info = *_clients[id];
			msg_queue.Push(new_msg);
		}
		break;

		case SERVERSIDE_USER_LEVEL_MSG:
		{
			client_msg_t *new_msg = new client_msg_t;
			new_msg->type = CLMSG_PACKET_RECV;
			string buf;
			if(!(recv_packet >> buf))
			{
				delete new_msg;
				break;
			}
			new_msg->packet.Clear();
			new_msg->packet.Append(buf.c_str(), buf.size());
			msg_queue.Push(new_msg);
		}
	}
}

bool NetThread::Disconnect()
{
	LOCK(_mutex);

	if(_state != CONNECTED)
		return false;

	_disconnect_switch = true;

	return true;
}

}