#include "netThread.h"

namespace opznet
{

Client::Client(const wstring &name, bool is_member,
			   sf::TcpSocket *socket, int delay_time,
			   const sf::IpAddress &address,
			   unsigned short udp_port)
	: _name(name), _is_member(is_member), _socket(socket),
	_last_ping_time(0), _delay_time(0), _address(address),
	_udp_port(udp_port)
{
}

Client::~Client()
{
	delete _socket;
}

void NetThread::S_Create(const std::wstring &server_name)
{
	CREATE_SINGLETON(new NetThread(server_name));

	_instance->Begin();
}

void NetThread::S_Delete()
{
	DELETE_SINGLETON;
}

bool NetThread::S_GetClientInfo(ID id, client_t *client)
{
	if(_instance->_clients.find(id) == _instance->_clients.end())
	{
		//없는 경우입니다.
		return false;
	}

	auto info = _instance->_clients[id];
	client->name = info->_name;
	client->is_member = info->_is_member;
	
	return true;
}

NetThread *NetThread::_instance = nullptr;

NetThread::NetThread(const std::wstring &server_name) : opzThread(),
	_greeter(),
	_server_name(server_name),
	_selector(),
	_clients(),
	_clock(),
	_current_time(0),
	_last_ping_time(0),
	_udp_socket(),
	_primary_port()
{
	_instance = this;

	_greeter = new Greeter();
	_greeter->Begin();

	_clock.restart();

	_udp_socket.setBlocking(false);

	_primary_port =
		static_cast<unsigned short>(GetPrivateProfileInt(L"GENERAL", L"PRIMARY_PORT", 55878, L"./game.ini"));

	if(_udp_socket.bind(_primary_port) != sf::Socket::Done)
	{
		auto new_msg = new server_msg_t;
		new_msg->type = SVMSG_BIND_FAILED;
		msg_queue.Push(new_msg);
	}
}

NetThread::~NetThread()
{
	for(auto i = _clients.begin(); i != _clients.end(); ++i)
		delete i->second;

	delete _greeter;
}

void NetThread::Run()
{
#ifdef _DEBUG
	wprintf(L"오파츠넷 쓰레드 실행\n");
#endif
	sf::Time wait_time = sf::milliseconds(1);
	_last_ping_time = _clock.getElapsedTime().asMilliseconds();

	while(1)
	{
		opzProfiler::S_FrameBegin();
		opzProfiler::S_SampleBegin(L"wait");
		_current_time = _clock.getElapsedTime().asMilliseconds();

		if(_selector.wait(wait_time))
		{
			for(auto i = _clients.begin(); i != _clients.end(); ++i)
			{
				auto &client = *i->second;
				if(_selector.isReady(*client._socket))
				{
					sf::Packet packet;
					auto res = client._socket->receive(packet); 
					if(res == sf::Socket::Done)
					{
						HandleReceivedPacket(i->first, client, packet);
					}
					else
					{
						HandleGoneClient(i->first);
					}
					break;
				}
			}
		}
		opzProfiler::S_SampleEnd(L"wait");
		
		opzProfiler::S_SampleBegin(L"handle_garbages");
		HandleGarbages();
		opzProfiler::S_SampleEnd(L"handle_garbages");
		
		opzProfiler::S_SampleBegin(L"update_delay");
		UpdateDelayTime();
		opzProfiler::S_SampleEnd(L"update_delay");
		
		opzProfiler::S_SampleBegin(L"handle_ss_msg");
		HandleServersideMsg();
		opzProfiler::S_SampleEnd(L"handle_ss_msg");
		
		opzProfiler::S_SampleBegin(L"handle_newbie");
		HandleNewClient();
		opzProfiler::S_SampleEnd(L"handle_newbie");

		/*
		COORD pos = {5, 10};
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
		wprintf(opzProfiler::S_GetBuffer());
		*/
		
		opzProfiler::S_FrameEnd();
	}
}

void NetThread::HandleReceivedPacket(ID client_id, Client &client, sf::Packet &packet)
{
	unsigned short	header;
	packet >> header;

	switch(header)
	{
		case PONG:
			client._delay_time = (_current_time - client._last_ping_time) / 2;
			{
				auto new_msg = new server_msg_t;
				new_msg->type = SVMSG_CLIENT_DELAY_TIME_UPDATED;
				new_msg->client_info.id = client_id;
				new_msg->client_info.is_member = client._is_member;
				new_msg->client_info.name = client._name;
				new_msg->delay_time = client._delay_time;
				msg_queue.Push(new_msg);
			}
			break;

		case CLIENTSIDE_USER_LEVEL_MSG:
		{
			auto new_msg = new server_msg_t;
			new_msg->type = SVMSG_PACKET_RECV;
			string buf;
			if(!(packet >> buf))
			{
				delete new_msg;
				break;
			}
			new_msg->packet.Clear();
			new_msg->packet.Append(buf.c_str(), buf.size());

			new_msg->client_info.id = client_id;
			new_msg->client_info.is_member = client._is_member;
			new_msg->client_info.name = client._name;

			msg_queue.Push(new_msg);
			break;
		}

		case GOOD_BYE:
		{
			HandleGoneClient(client_id);
			break;
		}
	}
}

void NetThread::HandleGoneClient(ID client_id)
{
	auto new_msg = new server_msg_t;
	new_msg->type = SVMSG_CLIENT_GONE;
	new_msg->client_info.id = client_id;
	auto client = _clients[client_id];
	new_msg->client_info.is_member = client->_is_member;
	new_msg->client_info.name = client->_name;
	
	_clients.erase(client_id);
	_selector.remove(*client->_socket);
	client->_socket->disconnect();
	
	delete client;

	msg_queue.Push(new_msg);
	
	sf::Packet alert_gone_packet;
			
	alert_gone_packet << CONST_CLIENT_GONE;
	alert_gone_packet << client_id;
	for(auto i = _clients.begin(); i != _clients.end(); ++i)
	{
		if(i->second->_socket->send(alert_gone_packet) != sf::Socket::Done)
		{
			HandleGoneClient(i->first);
		}
	}
}

void NetThread::HandleGarbages()
{
	while(!garbages.CheckEmpty())
	{
		server_msg_t *msg;
		garbages.Pop(&msg);
		delete msg;
	}
}

void NetThread::UpdateDelayTime()
{
	if(_current_time - _last_ping_time > 1000)
	{
		_last_ping_time = _current_time;
		if(!_clients.empty())
		{
			int max = -1;
			map<ID, Client *>::iterator max_one;
			for(auto i = _clients.begin(); i != _clients.end(); ++i)
			{
				if(max <= i->second->_last_ping_time)
				{
					max = i->second->_last_ping_time;
					max_one = i;
				}
			}

			sf::Packet ping_packet;
			ping_packet << CONST_PING;
			max_one->second->_last_ping_time = _current_time;

			if(max_one->second->_socket->send(ping_packet) != sf::Socket::Done)
			{
				HandleGoneClient(max_one->first);
			}
		}
	}
}

void NetThread::HandleServersideMsg()
{
	while(!tcp_uni_packet_queue.CheckEmpty())
	{
		unicast_packet_t *last_one;
		tcp_uni_packet_queue.Pop(&last_one);
		map<ID, Client *>::iterator client_it;
		if((client_it = _clients.find(last_one->clientId)) != _clients.end())
		{
			sf::Packet msg_packet;
			msg_packet << CONST_SERVERSIDE_USER_LEVEL_MSG;
			msg_packet << last_one->data;
			if(client_it->second->_socket->send(msg_packet) != sf::Socket::Done)
			{
				HandleGoneClient(last_one->clientId);
			}
			delete last_one;
		}
	}

	while(!udp_uni_packet_queue.CheckEmpty())
	{
		unicast_packet_t *last_one;
		udp_uni_packet_queue.Pop(&last_one);
		map<ID, Client *>::iterator client_it;
		if((client_it = _clients.find(last_one->clientId)) != _clients.end())
		{
			sf::Packet msg_packet;
			msg_packet << CONST_SERVERSIDE_USER_LEVEL_MSG;
			msg_packet << last_one->data;
			
			auto client = client_it->second;

			_udp_socket.send(
				msg_packet,
				client->_address,
				client->_udp_port);
			delete last_one;			
		}
	}

	while(!multi_packet_queue.CheckEmpty())
	{
		multicast_packet_t *last_one;
		multi_packet_queue.Pop(&last_one);

	}
}

void NetThread::HandleNewClient()
{
	while(!_new_client_queue.CheckEmpty())
	{
		pair<ID, Client *> last_one;
		_new_client_queue.Pop(&last_one);
		ID		id = last_one.first;
		Client	*new_client = last_one.second;

		sf::Packet client_intro_packet;
		client_intro_packet << CONST_CLIENT_INTRO;
		client_intro_packet << id;
		client_intro_packet << new_client->_name;
		client_intro_packet << new_client->_is_member;

		//기존 클라이언트들에게 새로운 클라이언트의 정보를 알려줍니다.
		for(auto i = _clients.begin(); i != _clients.end(); ++i)
		{
			if(i->second->_socket->send(client_intro_packet) != sf::Socket::Done)
			{
				return;
			}
		}
	
		//새 클라이언트에게 기존 클라이언트들의 정보를 알려줍니다.
		for(auto i = _clients.begin(); i != _clients.end(); ++i)
		{
			sf::Packet intro;
			intro << CONST_CLIENT_INTRO;
			intro << i->first;
			intro << i->second->_name;
			intro << i->second->_is_member;
			if(new_client->_socket->send(intro) != sf::Socket::Done)
			{
				HandleGoneClient(i->first);
			}
		}

		_clients[id] = new_client;
		_selector.add(*new_client->_socket);

		auto new_msg = new server_msg_t;
		new_msg->type = SVMSG_NEW_CLIENT;
		new_msg->client_info.id = id;
		new_msg->client_info.is_member = new_client->_is_member;
		new_msg->client_info.name = new_client->_name;
		msg_queue.Push(new_msg);
	}
}

}