#include "../../opznet/src/Thread.h"

#include <opznet/client.h>
#include <Windows.h>
#include <iostream>
#include <deque>

#ifdef _DEBUG
	#pragma comment(lib, "opznet-client-d.lib")
#else
	#pragma comment(lib, "opznet-client.lib")
#endif

using namespace opznet;
using namespace std;

class InputThread : public opzThread
{
public:
	InputThread() : opzThread()
	{
	}

	bool GetLine(wstring &input_string)
	{
		if(queue.empty())
			return false;

		input_string = queue.back();
		queue.pop_back();

		return true;
	}

protected:
	void Run()
	{
		wcout << L"입력 쓰레드 실행" << endl;
		while(1)
		{
			wstring input_string;
			getline(wcin, input_string);
			queue.push_front(input_string);
		}
	}

private:
	deque<wstring> queue;
};

client_msg_t	client_msg;
bool			loop_over = false;
InputThread		input;
wstring			input_string;
wstring			my_name;
bool			connected = false;

void HandleInput()
{
	if(input.GetLine(input_string))
	{ 
		if(input_string == L"help")
		{
			wcout << L"my name is ..." << endl <<
				L"connect ..." << endl <<
				L"find local server" << endl <<
				L"disconnect" << endl;
		}
		else if(input_string.substr(0, 11) == L"my name is ")
		{
			my_name = input_string.substr(11, input_string.length());
		}
		else if(input_string.substr(0, 8) == L"connect ")
		{
			if(my_name.empty())
				wcout << L"이름을 먼저 지정해주십시오." << endl;
			else
				RequestConnectToServer(input_string.substr(8, input_string.length()).c_str(), my_name.c_str());
		}
		else if(input_string == L"find local server")
		{
			if(!RequestSearchLocalNetServers())
			{
				wcout << L"이미 찾는 중입니다." << endl;
			}
		}
		else if(input_string == L"disconnect")
		{
			if(connected)
			{
				DisconnectToServer();
				connected = false;
			}
			else
				wcout << L"연결되지 않았습니다." << endl;
		}
		else
		{
			if(connected)
			{
				Packet send_packet;
				send_packet << input_string;
				SafeSend(send_packet);
			}
			else
			{
				wcout << L"연결이 아직 안돼서 못보냅니다." << endl;
			}
		}
	}
}

int main()
{
	_wsetlocale(LC_ALL, L"korean");
	BeginClient();
	
	input.Begin();
	
	while(!loop_over)
	{
		HandleInput();

		if(GetClientMsg(&client_msg))
		{
			switch(client_msg.type)
			{
			case CLMSG_CONNECTED:
				wcout << L"연결 완료!" << endl;
				connected = true;
				break;

			case CLMSG_CONNECT_TRY_TIMEOUT:
				wcout << L"연결 실패!" << endl;
				break;

			case CLMSG_UNEXPECTED_ERROR:
				wcout << L"심각한 오류 발생!" << endl;
				connected = false;
				break;
				
			case CLMSG_CLIENT_INTRODUCTION:
				wcout << client_msg.client_info.name << L"님이 서버에 입장하셨습니다." << endl;
				break;

			case CLMSG_CLIENT_GONE:
				wcout << client_msg.client_info.name << L"님이 서버를 떠났습니다." << endl;
				break;

			case CLMSG_DISCONNECTED:
				wcout << L"서버와 연결이 해제됐습니다." << endl;
				connected = false;
				break;

			case CLMSG_PACKET_RECV:
				{
					wstring msg;
					client_msg.packet >> msg;
					wcout << msg << endl;
					break;
				}

			case CLMSG_LOCAL_SERVER_FOUND:
				wcout << client_msg.server_info.name << L"\t[" << client_msg.server_info.num_of_client << L"명]\t" <<
					client_msg.server_info.address << endl;
				break;

			case CLMSG_FINDING_LOCAL_SERVER_OVER:
				wcout << L"지역 서버 검색 종료" << endl;
				break;

			default:
				wcout << L"알 수 없는 메시지" << endl;
			}
		}
		else
		{
			Sleep(10);
		}
	}

	EndClient();

	return 0;
}