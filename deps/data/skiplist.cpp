/*
* author:chanshimudingxi
*/

#include "skiplist.h"
#include <assert.h>

/*
 *    MAX_LEVEL
 *      ...
 *      ...
 *       3  ------------------------------------------------->  3                                                     
 *       2  ------------------------------------------------->  2 ------------>  2
 *       1  ------------------------------------>   1-------->  1 ------------>  1
 *       0  ------------->    0     ------------>   0-------->  0 -------------> 0
 *      ----                -----                  -----      -----            -----
 *      head                node1(0)              node2(1)    insert          node3(2)
*/

SkipList::SkipList()
{
	m_level = 0;
	m_len = 0;
	m_pHead = new SkipListNode;
  assert(NULL != m_pHead);
}

SkipList::~SkipList()
{
	SkipListNode* pCur = m_pHead;
	SkipListNode* pNext = NULL;
	while((pNext = pCur->forward[0]))
	{
		delete pCur;
		pCur = pNext;
	}
	delete pCur;		
	pCur = NULL;
	m_level = 0;
	m_len = 0;
}

SkipListNode* SkipList::Find(const std::string& key)
{
	SkipListNode* pCur = m_pHead;
	SkipListNode* pNext = NULL;
	for(int i = m_level; i >= 0; --i)
	{
		while((pNext = pCur->forward[i]) && pNext->key < key)
		{
			pCur = pNext;
		}
		if(pNext && pNext->key == key)
		{
			return pNext;
		}
	}
	return NULL;
}

bool SkipList::Insert(const std::string& key,const std::string& value)
{
	SkipListNode* pUpdate[SKIP_LIST_MAX_LEVEL+1];
	SkipListNode* pCur = m_pHead;
	SkipListNode* pNext = NULL;
	//插入新增节点，需要对一系列相关节点进行索引信息的更新
	for(int i = m_level; i >= 0; i--)
	{
		while((pNext = pCur->forward[i]) && pNext->key < key)
		{
			pCur = pNext;
		}
    pUpdate[i] = pCur;
	}
  
  //不允许插入相同key的节点
  if(pNext && pNext->key == key)
  {
    return false;
  }

	//获取新增节点插入的层数，层数越高，插入的概率越低
  //最终节点构成的图，如果比较理想的情况下，相当于一个二叉树
	srand(time(NULL));
	int level = 0;
	for(int i = 0; i < SKIP_LIST_MAX_LEVEL; i++)
	{
		if(rand()%2)
		{
			++level;
		}
	}
  //插入的节点如果大于当前层数，需要增加一层
  //同时在这层里，表头节点要指向该新增节点
	if(level > m_level)
	{
		level = ++m_level;
		pUpdate[level] = m_pHead;
	}
	//insert data
	SkipListNode* pNode = new SkipListNode;
  assert(NULL != pNode);
	pNode->key = key;
  pNode->value = value;
	for(int i = level; i >=0 ; i--)
	{
		pNode->forward[i] = pUpdate[i]->forward[i];//新加入节点的next节点
		pUpdate[i]->forward[i] = pNode;//新加入节点的front节点
	}
	++m_len;
	return true;
}

bool SkipList::Delete(const std::string& key)
{
	SkipListNode* pUpdate[SKIP_LIST_MAX_LEVEL+1];
	SkipListNode* pCur = m_pHead;
	SkipListNode* pNext = NULL;
	//get updates
	for(int i = m_level; i >= 0; i--)
	{
		while((pNext = pCur->forward[i]) && pNext->key < key)
		{
			pCur = pNext;
		}
		pUpdate[i] = pCur;
	}	
	//delete data
	if(pNext && pNext->key == key)
	{
		for(int i = m_level; i >= 0; i--)
		{
			if(pUpdate[i]->forward[i] == pNext)
			{
				pUpdate[i]->forward[i] = pNext->forward[i];
			}
		}
		delete pNext;
		//如果删除的是最高那层的节点，判断是否需要减少层数
		int curMaxLevel = m_level;
		while(NULL == m_pHead->forward[curMaxLevel] && curMaxLevel > 0)
		{
			--curMaxLevel;
		}
		m_level = curMaxLevel;
		--m_len;
	}
  return true;
}

SkipListNode* SkipList::Head()
{
  return m_pHead;
}

SkipListNode* SkipList::Next(SkipListNode* node)
{
  return NULL == node ? NULL : node->forward[0];
}
