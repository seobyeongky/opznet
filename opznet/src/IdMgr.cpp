#include "IdMgr.h"
#include "common.h"

#include <assert.h>

namespace opznet

{
/**********************
		public
**********************/
void opzIdMgr::S_Create()
{
	CREATE_SINGLETON(new opzIdMgr())
}

void opzIdMgr::S_Delete()
{
	DELETE_SINGLETON
}

/**********************
		private
**********************/
opzIdMgr *opzIdMgr::_instance = NULL;

opzIdMgr::opzIdMgr(void)
{
	_instance = this;
}

opzIdMgr::~opzIdMgr(void)
{
	for(auto i = _miniIdMgrMap.begin();
		i != _miniIdMgrMap.end();
		++i)
	{
		delete i->second;
	}
}

opzMiniIdMgr *opzIdMgr::GetMiniIdMgr(ID type)
{
	auto foundOne = _instance->_miniIdMgrMap[type];
	if(foundOne == nullptr)
	{
		auto newOne = new opzMiniIdMgr();
		_instance->_miniIdMgrMap[type] = newOne;
		foundOne = newOne;
	}
	return foundOne;
}

ID opzIdMgr::GetId(ID type)
{
	auto foundOne = GetMiniIdMgr(type);
	return foundOne->GetId();
}

void opzIdMgr::SetId(ID type, ID id)
{
	auto foundOne = GetMiniIdMgr(type);
	foundOne->SetId(id);
}

void opzIdMgr::DeleteId(ID type, ID id)
{
	auto foundOne = _instance->_miniIdMgrMap[type];
	if(foundOne == nullptr) return;
	foundOne->DeleteId(id);
}

}