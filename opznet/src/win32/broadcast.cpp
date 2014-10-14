#include "common.h"

#include <Windows.h>
#include <Iprtrmib.h>
#include <WinSock2.h>
#include <algorithm>

#pragma comment(lib, "ws2_32.lib")

namespace opznet
{

typedef DWORD (CALLBACK* GetIpAddr)(LPVOID,LPVOID,BOOL);

bool GetBroadcastAddress(list<sf::IpAddress> *address_list)
{
	HINSTANCE	dllHandle = NULL;
	GetIpAddr	GetIpAddrPtr = NULL;
	dllHandle = LoadLibrary(L"iphlpapi.dll");

	if (dllHandle == NULL)
	{
#ifdef _DEBUG
		wprintf(L"[오류] iphlpapi.dll 로드 실패!\n");
#endif
		return false;
	}

	GetIpAddrPtr = (GetIpAddr)GetProcAddress(dllHandle,"GetIpAddrTable");
	
	if (GetIpAddrPtr == NULL)
	{
#ifdef _DEBUG
		wprintf(L"Get GetIpAddrTable Error\n");
#endif
		return false;
	}

	char addr[255];
	DWORD size = 255;

	DWORD ret = GetIpAddrPtr((LPVOID)&addr,&size,FALSE);

	if (ret != NO_ERROR)
	{
#ifdef _DEBUG
		wprintf(L"Error %d\n", ret);
#endif
		return false;
	}

	FreeLibrary(dllHandle);

	MIB_IPADDRTABLE* mAddr = (MIB_IPADDRTABLE *) addr;
	for (int x=0; x < (int)mAddr->dwNumEntries; x++)
	{
		MIB_IPADDRROW	*Table = (MIB_IPADDRROW *)(addr + 4 + sizeof(MIB_IPADDRROW)*x);
		DWORD			broadcast_address = Table->dwAddr | ~(Table->dwMask);
		address_list->push_back(sf::IpAddress(inet_ntoa((in_addr &)( broadcast_address ))));
	}
	
	unique(address_list->begin(), address_list->end());

	return true;
}


}