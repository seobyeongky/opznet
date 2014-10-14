#include "internal.h"
#include <opznet/client.h>
#include "NetThread.h"
#include "ServerFinderThread.h"

namespace opznet
{
	class opznetClient
	{
	public:
		static void S_Create();
		static void S_Delete();

	private:
		static opznetClient *_instance;

		opznetClient();
		~opznetClient();
	};

	void opznetClient::S_Create()
	{
		CREATE_SINGLETON(new opznetClient);
	}

	void opznetClient::S_Delete()
	{
		DELETE_SINGLETON;
	}

	opznetClient *opznetClient::_instance = nullptr;

	opznetClient::opznetClient()
	{
		_instance = this;
		
		_wsetlocale(LC_ALL, L"korean");

//		Mem_Init();

		NetThread::S_Create();

		ServerFinderThread::S_Create();

#ifdef _DEBUG
	wprintf(L"오파츠넷 클라이언트 초기화\n");
#endif
	}

	opznetClient::~opznetClient()
	{
		ServerFinderThread::S_Delete();

		NetThread::S_Delete();

//		Mem_Shutdown();
	}

	bool BeginClient()
	{
		opznetClient::S_Create();

		return true;
	}

	void EndClient()
	{
		opznetClient::S_Delete();
	}
	
	/*
		DLL_IMPL
	*/
	opzSafeQueue<client_msg_t *>	msg_queue;
	opzSafeQueue<string *>			letters;
	opzSafeQueue<client_msg_t *>	garbages;

	bool GetClientMsg(client_msg_t *msg)
	{
		if(msg_queue.CheckEmpty())
			return false;
		
		client_msg_t *last;
		msg_queue.Pop(&last);
		msg->type = last->type;
		switch(msg->type)
		{
		case CLMSG_CLIENT_INTRODUCTION:
			msg->client_info = last->client_info;
			break;

		case CLMSG_PACKET_RECV:
			msg->packet = last->packet;
			break;

		case CLMSG_CLIENT_GONE:
			msg->client_info = last->client_info;
			break;

		case CLMSG_CONNECTED:
			msg->my_id = last->my_id;
			msg->server_info = last->server_info;
			break;

		case CLMSG_LOCAL_SERVER_FOUND:
			msg->server_info = last->server_info;
			break;
		}

		garbages.Push(last);

		return true;
	}

	bool RequestConnectToServer(LPCWSTR server_address, LPCWSTR user_name)
	{
		return NetThread::S_RequestConnectToServer(server_address, user_name);
	}
	
	void SafeSend(const Packet &packet)
	{
		letters.Push(new string(reinterpret_cast<const char *>(packet.GetData()), packet.GetByteSize()));
	}

	bool DisconnectToServer()
	{
		return NetThread::S_Disconnect();
	}

	bool RequestSearchLocalNetServers()
	{
		return ServerFinderThread::S_BeginFind();
	}
}

BOOL WINAPI DllMain(HINSTANCE hInst,DWORD dwReason,LPVOID lpRes)
{
	return TRUE;
}