#include "netThread.h"
#include <iostream>

namespace opznet
{

Greeter::Greeter(void) : opzThread(),
	_listener(),
	_udp_listener(),
	_guest(),
	_selector(),
	_nextId(0u),
	_game_name(),
	_secondary_port()
{
	WCHAR game_name[32];
	auto res = GetPrivateProfileString(L"GENERAL", L"NAME", L"UNKNOWN", game_name, 32, L"./game.ini");
	
	_game_name = game_name;

	_listener.setBlocking(true);
	_secondary_port	=
		static_cast<unsigned short>(
		GetPrivateProfileInt(L"GENERAL", L"SECONDARY_PORT", 55878, L"./game.ini"));
	
	if(_listener.listen(_secondary_port) != sf::Socket::Done)
	{
		auto new_msg = new server_msg_t;
		new_msg->type = SVMSG_BIND_FAILED;
		msg_queue.Push(new_msg);
	}
	else
	{
		_selector.add(_listener);
	}
	
	_udp_listener.setBlocking(false);
	if(_udp_listener.bind(_secondary_port) != sf::Socket::Done)
	{
		auto new_msg = new server_msg_t;
		new_msg->type = SVMSG_BIND_FAILED;
		msg_queue.Push(new_msg);		
	}
	else
	{
		_selector.add(_udp_listener);
	}
}

Greeter::~Greeter(void)
{
	for(auto i = _guest.begin(); i != _guest.end(); ++i)
		delete i->second;
}

void Greeter::Run()
{
	sf::Time wait_time = sf::seconds(1.f);

	while(1)
	{	
		_selector.wait(wait_time);

		if(_selector.isReady(_listener))
		{
			HandleTcpListener();
		}
		else if(_selector.isReady(_udp_listener))
		{
			HandleUdpListener();
		}
		else
		{
			for(auto i = _guest.begin(); i != _guest.end();)
			{
				auto &client_socket = *i->second;
				if(_selector.isReady(client_socket))
				{
					i = HandleGuest(i);
				}
				else
				{
					++i;
				}
			}
		}
	}
}

void Greeter::HandleTcpListener()
{
	sf::TcpSocket *client = new sf::TcpSocket;
	if(_listener.accept(*client) == sf::Socket::Done)
	{
		//Accept 성공
		_guest[_nextId++] = client;
		_selector.add(*client);
	}
	else
	{
		//실패
		delete client;
	}
}

void Greeter::HandleUdpListener()
{
 	unsigned short	client_port;
	sf::IpAddress	client_address;
	sf::Packet		packet;

	if(_udp_listener.receive(packet, client_address, client_port) != sf::Socket::Done)
		return;

	unsigned short header;
	if(!(packet >> header) || header != ANY_SERVER_THERE)
		return;

	wstring client_game_name;
	if(!(packet >> client_game_name) || client_game_name != _game_name)
		return;
	
	packet.clear();
	packet << CONST_HERE_IS_SERVER;
	packet << _game_name;
	packet << NetThread::S_GetServerName();
	packet << NetThread::S_GetNumOfClients();

	_udp_listener.send(packet, client_address, client_port);
}

#define NO_WAY delete &client_socket; \
			return _guest.erase(it);

map<ID, sf::TcpSocket *>::iterator Greeter::HandleGuest(map<ID, sf::TcpSocket *>::iterator it)
{
	auto			&client_socket = *it->second;
	unsigned short	client_udp_port;
	sf::Clock		clock;
	vector<int>		ping_time;
	sf::Packet		ping_packet;
	sf::Packet		recv_packet;
	wstring			client_game_name;
	wstring			client_name;
					
	if(client_socket.receive(recv_packet) != sf::Socket::Done)
	{
		NO_WAY;
	}

	if(!(recv_packet >> client_game_name))					//상대편 게임 이름
	{
		NO_WAY;
	}

	sf::Packet greeting_packet;

	greeting_packet << _game_name.c_str();
	greeting_packet << NetThread::S_GetServerName().c_str();
	greeting_packet << NetThread::S_GetNumOfClients();
	greeting_packet << it->first;

	if(client_socket.send(greeting_packet) != sf::Socket::Done) //게임 이름, 서버 이름
	{
		NO_WAY;
	}
					
	if(client_game_name != _game_name)
	{
		NO_WAY;
	}
					
	if(client_socket.receive(recv_packet) != sf::Socket::Done)
	{
		NO_WAY;
	}

	if(!(recv_packet >> client_name >> client_udp_port))
	{
		NO_WAY;
	}

	ping_time.resize(PING_PONG_TEST_NUMBER, 0);
	ping_packet << PING;
					
	for(int j = 0; j < PING_PONG_TEST_NUMBER; ++j)
	{
		clock.restart();
		if(client_socket.send(ping_packet) != sf::Socket::Done)
		{
			NO_WAY;
		}
		if(client_socket.receive(recv_packet) != sf::Socket::Done)
		{
			NO_WAY;
		}
		ping_time[j] = clock.getElapsedTime().asMilliseconds();
	}

	sort(ping_time.begin(), ping_time.end());

	int middle_delay_time = ping_time[PING_PONG_TEST_NUMBER / 2];	//중앙값
	sf::Packet time_info_packet;
	time_info_packet << middle_delay_time;
	time_info_packet << NetThread::S_GetServerTime();
	if(client_socket.send(time_info_packet) != sf::Socket::Done)
	{
		NO_WAY;
	}

	_selector.remove(client_socket);

	auto new_client =
		new Client(client_name, false,
		&client_socket, middle_delay_time,
		it->second->getRemoteAddress(),
		client_udp_port);
	NetThread::S_AddClient(it->first, new_client);
	
	return _guest.erase(it);
}

}