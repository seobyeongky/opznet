#pragma once

#define	OPZNET_CLIENT_DLL

#ifdef _DEBUG
	#define OPZ_DEBUG_MEMORY
#else
	#define OPZ_REDIRECT_NEWDELETE
#endif

#include "../common.h"
#include "opznet/client.h"

namespace opznet
{
	extern opzSafeQueue<client_msg_t *>	msg_queue;
	extern opzSafeQueue<string *>		letters;
	extern opzSafeQueue<client_msg_t *> garbages;
};