#pragma once

#define	OPZNET_SERVER_DLL

#ifdef _DEBUG
	#define OPZ_DEBUG_MEMORY
#else
	#define OPZ_REDIRECT_NEWDELETE
#endif

#include "../common.h"
#include "../profiler.h"
#include "opznet/server.h"

namespace opznet
{
	struct unicast_packet_t
	{
		string		data;
		ID			clientId;
	};

	struct multicast_packet_t
	{
		string		data;
		vector<ID>	client_id_vector;
	};

	class Client;

	extern opzSafeQueue<server_msg_t *>			msg_queue;
	extern opzSafeQueue<unicast_packet_t *>		tcp_uni_packet_queue;
	extern opzSafeQueue<unicast_packet_t *>		udp_uni_packet_queue;
	extern opzSafeQueue<multicast_packet_t *>	multi_packet_queue;
	extern opzSafeQueue<server_msg_t *>			garbages;
}