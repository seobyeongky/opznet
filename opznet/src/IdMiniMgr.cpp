#include "IdMgr.h"
#include "mathHelper.h"

#include <assert.h>

namespace opznet
{

opzMiniIdMgr::opzMiniIdMgr(void)
{
	_binTree.resize(2, 0u);
	_level = 1u;
}

opzMiniIdMgr::~opzMiniIdMgr(void)
{
}

ID opzMiniIdMgr::GetId()
{
	ID i, count;
	if(_binTree[1] >= Pow2(_level - 1))
	{
		DoubleGrow();
	}
	
	//본격적으로 빈 id를 찾습니다.
	for(i = 1u, count = 0u; count < _level - 1; count++)
	{
		_binTree[i]++;
		if(_binTree[2u * i] < Pow2(_level - count - 2U))
		{	//왼쪽
			i *= 2u;
		}
		else
		{	//오른쪽
			i = 2u * i + 1u;
		}
	}
	_binTree[i] = 1u;
	return i - Pow2(_level - 1u) + 1u;
}

void opzMiniIdMgr::SetId(ID id)
{
	while(Pow2(_level - 1) < id)
	{
		//level이 충분하지 못하다.
		DoubleGrow();
	}

	ID index = GetTreeIndex(id);
	if(index == 0u) return;
	if(_binTree[index] >= 1u)
	{
//		assert(L"이미 할당 된 ID를 또 Set했습니다.");
		return;
	}
	else
	{
		while(index > 0u)
		{
			++_binTree[index];
			index /= 2u;
		}
	}
}

void opzMiniIdMgr::DeleteId(ID id)
{
	ID index = GetTreeIndex(id);
	if(index == 0u) return;
//	assert(_binTree[id] > 0u && L"아직 할당되지 않은 ID를 삭제하려고 했습니다.");
	if(_binTree[id] <= 0u) return;
	
	while(index > 0u)
	{
		--_binTree[index];
		index /= 2u;
	}
}

ID opzMiniIdMgr::GetTreeIndex(ID id)
{
	id += Pow2(_level - 1u) - 1u;
	assert(id <= _binTree.size());	//없는 아이디의 경우1
	assert(id > 0u);					//없는 아이디의 경우2
	if(id > _binTree.size() || id < 1u)return 0u;
	else return id;
}

void opzMiniIdMgr::DoubleGrow()
{
	//동적으로 _binTree의 크기를 2배로 늘려줍니다..
	//먼저 이전 _binTree의 정보를 잇게 해줍니다.
	std::vector<ID> newBinTree(2 * _binTree.size(), 0u);
	ID count = 1u;
	for(ID i = 0u; i < _level; ++i, count *= 2u)
	{
		for(ID j = 0u; j < count; ++j)
		{
			newBinTree[2 * count + j] = _binTree[count + j];
		}
	}
	//복사를 했으니 예전 vector는 버립니다.
	_binTree = newBinTree;
	_binTree[1] = _binTree[2];
	_level++;
}
}