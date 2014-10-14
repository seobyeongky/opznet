#include <opznet/server.h>
#include <iostream>
#include <list>
#include <map>
#include <Windows.h>

#ifdef _DEBUG
	#pragma comment(lib, "opznet-server-d.lib")
#else
	#pragma comment(lib, "opznet-server.lib")
#endif

using namespace opznet;
using namespace std;

typedef void ( * HANDLER)();

list<pair<client_t, int>>	clients;
server_msg_t				server_msg;

void gotoxy(int x, int y)
{
	COORD Pos = {x, y};

	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), Pos);
}

void PrintClient(pair<client_t, int> &client)
{
	wcout << client.first.id << L" : " << client.first.name;
	
	if(client.second >= 0)
		wcout << L"(지연시간 : " << client.second << L")";
	
	wcout << endl;
}

void PrintClients()
{
	int count = 0;
	for(auto i = clients.begin(); i != clients.end(); ++i, ++count)
	{
		gotoxy(0, count);
		PrintClient(*i);	
	}
}

void HandleNewClient()
{
	clients.push_back(pair<client_t, int>(server_msg.client_info, -1));

	PrintClients();
}

void HandleGoneClient()
{	
	_wsystem(L"cls");

	for(auto i = clients.begin(); i != clients.end(); ++i)
	{
		if(i->first.id == server_msg.client_info.id)
		{
			clients.erase(i);
			break;
		}
	}

	PrintClients();
}

void UpdateDelayTime()
{
	int count = 0;
	for(auto i = clients.begin(); i != clients.end(); ++i, ++count)
	{
		if(i->first.id == server_msg.client_info.id)
		{
			i->second = server_msg.delay_time;
			gotoxy(0, count);
			PrintClient(*i);
			break;
		}
	}
}

void HandleReceivedPacket()
{
	wstring recv_msg;
	server_msg.packet >> recv_msg;

	wstring msg;
	msg.assign(server_msg.client_info.name);
	msg.append(L" : ");
	msg.append(recv_msg);

	for(auto i = clients.begin(); i != clients.end(); ++i)
	{
		if(i->first.id == server_msg.client_info.id) continue;

		Packet send_packet;
		send_packet << msg;

		UnsafeSend(i->first.id, send_packet);
	}
}

int wmain()
{
	BeginServer(L"test");

	system("cls");

	map<int, HANDLER> handlers;
	handlers[SVMSG_NEW_CLIENT] = HandleNewClient;
	handlers[SVMSG_CLIENT_GONE] = HandleGoneClient;
	handlers[SVMSG_CLIENT_DELAY_TIME_UPDATED] = UpdateDelayTime;
	handlers[SVMSG_PACKET_RECV] = HandleReceivedPacket;

	while(1)
	{
		if(GetServerMsg(&server_msg))
		{
			switch(server_msg.type)
			{
				case SVMSG_NEW_CLIENT:
				case SVMSG_PACKET_RECV:
				case SVMSG_CLIENT_GONE:
				case SVMSG_CLIENT_DELAY_TIME_UPDATED:
					handlers[server_msg.type]();
					break;

				case SVMSG_BIND_FAILED:
					wcout << L"바인드 실패" << endl;
					break;

				case SVMSG_UNEXPECTED_ERROR:
					wcout << L"예상치 못한 오류 발생" << endl;
					break;

				default:
					gotoxy(0, 15);
					wcout << L"알 수 없는 메시지" << endl;
					break;
			}
		}

		Sleep(50);
	}

	EndServer();

	return 0;
}