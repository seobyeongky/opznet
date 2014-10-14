#pragma once

#define	BUILD_VERSION	1

#define OPZNET_COMMON_DLL

#pragma comment(lib, "ws2_32.lib")

#ifdef _DEBUG
	#pragma comment(lib, "sfml-network-d.lib")
	#pragma comment(lib, "sfml-system-d.lib")
#else
	#pragma comment(lib, "sfml-network.lib")
	#pragma comment(lib, "sfml-system.lib")
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <WinSock2.h>

#include <SFML/Network.hpp>
#include <SFML/System.hpp>

#include <cstdio>
#include <map>
#include <cassert>
#include <deque>
#include <list>
#include <vector>

#include <opznet/shared_structs.h>

#include "Thread.h"
#include "HeaderDef.h"
//#include "md5.h"
//#include "Heap.h"
#include "SafeQueue.h"


namespace opznet
{

using namespace std;

#define INSTANCE_EXIST_ASSERTION assert(_instance != nullptr)

#define CREATE_SINGLETON(new_statement) \
	assert(_instance == nullptr); \
	if(_instance == nullptr) \
	{ \
		new_statement; \
	}

#define DELETE_SINGLETON \
	assert(_instance != nullptr); \
	SafeDelete(_instance);

#define SafeDelete(p) if(p != nullptr){delete p; p = nullptr; }
#define SafeDeleteArray(p) if(p != nullptr){delete[] p; p = nullptr; }
#define SafeRelease(p) if(p != nullptr){p->Release(); p = nullptr; }

bool GetBroadcastAddress(list<sf::IpAddress> *address_list);

}