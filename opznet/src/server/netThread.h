#pragma once

#include "internal.h"
#include "opznet/server.h"

namespace opznet
{
	//#define CLIENT_TYPE	0u		//for id mgr

	class Client
	{
	public:
		Client(const wstring &name, bool is_member,
			sf::TcpSocket *socket, int delay_time,
			const sf::IpAddress &address,
			unsigned short udp_port);
		~Client();

		wstring			_name;
		bool			_is_member;
		sf::TcpSocket	*_socket;
		sf::IpAddress	_address;

		int				_last_ping_time;
		int				_delay_time;
		unsigned short	_udp_port;
	};

	class Greeter :	public opzThread
	{
	public:
		Greeter(void);
		~Greeter(void);

	protected:
		void Run();

	private:
		sf::TcpListener				_listener;
		sf::UdpSocket				_udp_listener;
		map<ID, sf::TcpSocket *>	_guest;
		sf::SocketSelector			_selector;
		unsigned short				_secondary_port;

		ID							_nextId;

		wstring						_game_name;

		void								HandleTcpListener();
		void								HandleUdpListener();
		map<ID, sf::TcpSocket *>::iterator	HandleGuest(map<ID, sf::TcpSocket *>::iterator it);
	};

	class NetThread : public opzThread
	{
	public:
		static void				S_Create(const std::wstring &server_name);
		static void				S_Delete();

		static bool				S_GetClientInfo(ID id, client_t *client);
		static int				S_GetServerTime(){return _instance->_clock.getElapsedTime().asMilliseconds(); }
		static const wstring	&S_GetServerName(){return _instance->_server_name;}
		static void				S_AddClient(ID id, Client *new_client){_instance->_new_client_queue.Push(pair<ID, Client *>(id, new_client));}
		static void				S_GetPingTime();
		static size_t			S_GetNumOfClients(){return _instance->_clients.size(); }

	protected:
		void Run();

	private:
		static NetThread					*_instance;

		Greeter								*_greeter;

		wstring								_server_name;
		sf::SocketSelector					_selector;
		map<ID, Client *>					_clients;
		sf::Clock							_clock;
		int									_current_time;
		int									_last_ping_time;
		sf::UdpSocket						_udp_socket;
		unsigned short						_primary_port;
		opzSafeQueue<pair<ID, Client *>>	_new_client_queue;

		NetThread(const std::wstring &server_name);
		~NetThread();

		void	HandleReceivedPacket(ID client_id, Client &client, sf::Packet &packet);
		void	HandleGoneClient(ID client_id);
		void	HandleGarbages();
		void	UpdateDelayTime();
		void	HandleServersideMsg();
		void	HandleNewClient();
	};
}