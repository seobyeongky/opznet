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
	
	//���������� �� id�� ã���ϴ�.
	for(i = 1u, count = 0u; count < _level - 1; count++)
	{
		_binTree[i]++;
		if(_binTree[2u * i] < Pow2(_level - count - 2U))
		{	//����
			i *= 2u;
		}
		else
		{	//������
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
		//level�� ������� ���ϴ�.
		DoubleGrow();
	}

	ID index = GetTreeIndex(id);
	if(index == 0u) return;
	if(_binTree[index] >= 1u)
	{
//		assert(L"�̹� �Ҵ� �� ID�� �� Set�߽��ϴ�.");
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
//	assert(_binTree[id] > 0u && L"���� �Ҵ���� ���� ID�� �����Ϸ��� �߽��ϴ�.");
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
	assert(id <= _binTree.size());	//���� ���̵��� ���1
	assert(id > 0u);					//���� ���̵��� ���2
	if(id > _binTree.size() || id < 1u)return 0u;
	else return id;
}

void opzMiniIdMgr::DoubleGrow()
{
	//�������� _binTree�� ũ�⸦ 2��� �÷��ݴϴ�..
	//���� ���� _binTree�� ������ �հ� ���ݴϴ�.
	std::vector<ID> newBinTree(2 * _binTree.size(), 0u);
	ID count = 1u;
	for(ID i = 0u; i < _level; ++i, count *= 2u)
	{
		for(ID j = 0u; j < count; ++j)
		{
			newBinTree[2 * count + j] = _binTree[count + j];
		}
	}
	//���縦 ������ ���� vector�� �����ϴ�.
	_binTree = newBinTree;
	_binTree[1] = _binTree[2];
	_level++;
}
}