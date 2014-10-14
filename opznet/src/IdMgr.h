#pragma once

#include <vector>
#include <list>
#include <map>

#include "opznet/shared_structs.h"

namespace opznet
{

class opzMiniIdMgr
{
public:
	opzMiniIdMgr();
	~opzMiniIdMgr();

	ID GetId();
	void SetId(ID id);
	void DeleteId(ID id);

private:
	ID _level;
	std::vector<ID> _binTree;

	ID GetTreeIndex(ID id);
	void DoubleGrow();
};

class opzIdMgr
{
public:
	static void S_Create();
	static void S_Delete();

	static ID S_GetId(ID type) {return _instance->GetId(type); }
	static void S_SetId(ID type, ID id) {_instance->SetId(type, id); }
	static void S_DeleteId(ID type, ID id) { _instance->DeleteId(type, id); }

private:
	static opzIdMgr *_instance;

	std::map<ID, opzMiniIdMgr *> _miniIdMgrMap;

	opzIdMgr(void);
	~opzIdMgr(void);

	opzMiniIdMgr *GetMiniIdMgr(ID objectId);
	ID GetId(ID type);
	void SetId(ID type, ID id);
	void DeleteId(ID type, ID id);
};

inline ID GetId(ID type)
{
	return opzIdMgr::S_GetId(type);
}

inline void ReturnId(ID type, ID id)
{
	opzIdMgr::S_DeleteId(type, id);
}

}