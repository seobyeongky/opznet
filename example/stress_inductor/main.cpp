#include <opznet/client.h>
#include <Windows.h>

#ifdef _DEBUG
#pragma comment(lib, "opznet-client-d.lib")
#else
#pragma comment(lib, "opznet-client.lib")
#endif

using namespace opznet;
using namespace std;

int main()
{
	BeginClient();

	bool connected = false;
	client_msg_t msg;

	while(1)
	{
		if(GetClientMsg(&msg))
		{
			if(msg.type == CLMSG_CONNECTED)
				connected = true;
			else if(msg.type == CLMSG_DISCONNECTED)
				connected = false;
		}

		if(connected)
		{
			const wchar_t *haha = L"you are a idiot. hahah1ahahahahahahah";
			Packet send_packet;
			send_packet << haha;
			SafeSend(send_packet); 
		}
		else
		{
			//try to connect
			RequestConnectToServer(L"localhost", L"stress_inductor");
		}

		Sleep(500);
	}

	EndClient();
}