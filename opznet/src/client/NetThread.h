#pragma once

#include "internal.h"

#include "opznet/client.h"

namespace opznet
{

class NetThread : public opzThread
{
public:
	static void S_Create();
	static void S_Delete();

	static bool S_GetClientInfo(ID id, client_t *client);

	static bool S_RequestConnectToServer(const std::wstring &server_address, const std::wstring &user_name);
	static bool S_Disconnect(){return _instance->Disconnect(); }

protected:
	void		Run();

private:
	static NetThread		*_instance;
	client_t				_user;
	sf::TcpSocket			_tcp_sock;
	sf::UdpSocket			_udp_sock;
	std::map<ID, client_t *> _clients;
	enum
	{
		WAITING,
		CONNECTING,
		CONNECTED
	}_state;
	opzMutex				_mutex;
	sf::IpAddress			*_server_address;
	sf::Packet				_pong_msg;
	bool					_disconnect_switch;
	unsigned short			_udp_local_port;

	class ClientClock
	{
	public:
		ClientClock();
		~ClientClock();

		void	Restart(int cur_time);
		int		GetTime();

	private:
		sf::Clock	_clock;
		int			_delay;
	}_client_clock;

				NetThread(void);
				~NetThread(void);

	bool		ConnectToServer(const std::wstring &server_address, const std::wstring &user_name);
	void		CommunicateWithServer();
	void		HandleClientsideUserRequests();
	void		HandleRecvPacket(sf::Packet &recv_packet);
	
	bool		Disconnect();
};

}