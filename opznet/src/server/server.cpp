#include "internal.h"
#include "opznet/server.h"
#include "netThread.h"

#include <clocale>

namespace opznet
{

class opznetServer
{
public:
	static void	S_Create(const std::wstring &server_name);
	static void	S_Delete();

private:
	static opznetServer	*_instance;

	opznetServer(const std::wstring &server_name);
	~opznetServer();
};

/*
	opznetServer public
*/
void opznetServer::S_Create(const std::wstring &server_name)
{
	CREATE_SINGLETON(new opznetServer(server_name));
}

void opznetServer::S_Delete()
{
	DELETE_SINGLETON;
}

/*
	opznetServer private
*/
opznetServer *opznetServer::_instance = nullptr;

opznetServer::opznetServer(const std::wstring &server_name)
{
	_instance = this;
	
	_wsetlocale(LC_ALL, L"korean");

//	Mem_Init();
	NetThread::S_Create(server_name);

	opzProfiler::S_Create();
	
#ifdef _DEBUG
	wprintf(L"오파츠넷 서버 시작\n");
#endif
}

opznetServer::~opznetServer()
{
	opzProfiler::S_Delete();

	NetThread::S_Delete();
	
//	Mem_Shutdown();
}

BOOL WINAPI DllMain(HINSTANCE hInst,DWORD dwReason,LPVOID lpRes)
{
	return TRUE;
}

bool BeginServer(const std::wstring &server_name)
{
	opznetServer::S_Create(server_name);

	return true;
}

void EndServer()
{
	opznetServer::S_Delete();
}

/*
	DLL_IMPL
*/
opzSafeQueue<server_msg_t *>		msg_queue;
opzSafeQueue<unicast_packet_t *>	tcp_uni_packet_queue;
opzSafeQueue<unicast_packet_t *>	udp_uni_packet_queue;
opzSafeQueue<multicast_packet_t *>	multi_packet_queue;
opzSafeQueue<server_msg_t *>		garbages;

bool	GetServerMsg(server_msg_t *msg)
{
	if(msg_queue.CheckEmpty())
		return false;

	server_msg_t *last_one;
	msg_queue.Pop(&last_one);
	
	msg->type = last_one->type;

	switch(last_one->type)
	{
	case SVMSG_NEW_CLIENT:
		msg->client_info = last_one->client_info;
		break;

	case SVMSG_CLIENT_GONE:
		msg->client_info = last_one->client_info;
		break;

	case SVMSG_PACKET_RECV:
		msg->client_info = last_one->client_info;
		msg->packet = last_one->packet;
		break;

	case SVMSG_CLIENT_DELAY_TIME_UPDATED:
		msg->client_info = last_one->client_info;
		break;
	}

	garbages.Push(last_one);

	return true;
}

void	SafeSend(ID client_id, const Packet &packet)
{
	unicast_packet_t *uni_packet = new unicast_packet_t;
	uni_packet->clientId = client_id;
	uni_packet->data.assign(reinterpret_cast<const char *>(packet.GetData()), packet.GetByteSize());

	tcp_uni_packet_queue.Push(uni_packet);
}

void	UnsafeSend(ID client_id, const Packet &packet)
{
	unicast_packet_t *uni_packet = new unicast_packet_t;
	uni_packet->clientId = client_id;
	uni_packet->data.assign(reinterpret_cast<const char *>(packet.GetData()), packet.GetByteSize());

	udp_uni_packet_queue.Push(uni_packet);
}

void	UnsafeSendMultiTarget(std::vector<ID> &client_id_vector, const char * data, size_t byte_size)
{
	auto multi_packet = new multicast_packet_t;
	multi_packet->client_id_vector = client_id_vector;
	multi_packet->data = data;

	multi_packet_queue.Push(multi_packet);
}

}	// opznet namespace