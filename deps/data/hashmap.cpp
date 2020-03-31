/*
* author:chanshimudingxi
*/
#include "hashmap.h"
#include <iomanip>
#include <iostream>

HashSlot::HashSlot(HashFunction hashFunc,unsigned int capacity)
{
	m_hashFunc = hashFunc;
	m_keyCapacity = capacity;
	m_usedIndex = 0;
	m_keyCnt = 0;
	m_nodes[m_usedIndex] = new HashNode*[m_keyCapacity];
	for(int i=0; i<m_keyCapacity; i++)
	{
		m_nodes[m_usedIndex][i] = NULL;
	}
	m_nodes[1-m_usedIndex] = NULL;
}

HashSlot::~HashSlot()
{
	for(int i=0; i<m_keyCapacity;i++)
	{
		HashNode* pCur = m_nodes[m_usedIndex][i];
		while(pCur)
		{
			HashNode* pDelete = pCur;
			pCur = pCur->next;
			delete pDelete;
			pDelete = NULL;
		}
	}
	delete [] m_nodes[m_usedIndex];
	m_nodes[m_usedIndex] = NULL;
	m_keyCnt = 0;
	m_keyCapacity = 0;
	m_usedIndex = 0;
	m_hashFunc = NULL;
}

HashNode* HashSlot::Get(const std::string& key)
{
	HashNode* pCur = m_nodes[m_usedIndex][m_hashFunc(key)%m_keyCapacity];
	while(pCur)
	{
		if(pCur->key == key)
		{
			break;
		}
		pCur = pCur->next;
	}
	return pCur;
}

int HashSlot::Set(const std::string& key, const std::string& value)
{	
	if(m_keyCnt*3 > 2*m_keyCapacity)
	{
		//rehash
		unsigned int newKeyCapacity = m_keyCapacity*2;
		unsigned int newUsedIndex = 1 - m_usedIndex;
		m_nodes[newUsedIndex] = new HashNode*[newKeyCapacity];
		for(int i=0;i<newKeyCapacity;i++)
		{
			m_nodes[newUsedIndex][i] = NULL;
		}
		
		for(int i=0;i<m_keyCapacity;i++)
		{
			HashNode* pCur = m_nodes[m_usedIndex][i];
			while(pCur)
			{
				HashNode** ppFront = &m_nodes[newUsedIndex][(m_hashFunc(pCur->key))%newKeyCapacity];
				if((*ppFront) == NULL)
				{
					(*ppFront) = pCur;
					pCur = pCur->next;
					(*ppFront)->next = NULL;
				}
				else
				{
					HashNode* pInsert = *ppFront;
					while(pInsert->next)
					{
						pInsert = pInsert->next;
					}
					pInsert->next = pCur;					
					pCur = pCur->next;
					pInsert->next->next = NULL;
				}
			}
		}
		delete [] m_nodes[m_usedIndex];
		m_nodes[m_usedIndex] = NULL;
		
		m_keyCapacity = newKeyCapacity;
		m_usedIndex = newUsedIndex;
	}
	
	//note!!! must use HashNode**,otherwise pCur is nil
	HashNode** ppFront = &m_nodes[m_usedIndex][m_hashFunc(key)%m_keyCapacity];
	HashNode* pCur = *ppFront;
	if(NULL == (*ppFront))
	{
		(*ppFront) = new HashNode;
		(*ppFront)->key = key;
		(*ppFront)->value = value;
		(*ppFront)->next = NULL;
		++m_keyCnt;
	    return 0;
    }
	while(pCur->next)
	{
		if(pCur->key == key)
		{
			pCur->value = value;
		    return 0;
        }
		pCur = pCur->next;
	}
	if(pCur->key == key)
	{
		pCur->value = value;
	    return 0;
    }
	else
	{
		HashNode* pNewNode = new HashNode;
		pNewNode->key = key;
		pNewNode->value = value;
		pNewNode->next = NULL;
		pCur->next = pNewNode;
		++m_keyCnt;
	}
	return 0;
}

HashMap::HashMap(int slotCnt,HashFunction slotFunc,HashFunction nodeFunc)
{
	m_slotHashFunc = slotFunc;
	m_nodeHashFunc = nodeFunc;
	m_slotCnt = slotCnt;
	m_slots = new HashSlot*[m_slotCnt];
	for(int i = 0; i < m_slotCnt; i++)
	{
		m_slots[i] = new HashSlot(slotFunc);
	}
}

HashMap::~HashMap()
{
	m_nodeHashFunc = NULL;
	m_slotHashFunc = NULL;
	for(int i = 0; i < m_slotCnt; i++)
	{
		delete m_slots[i];
		m_slots[i] = NULL;
	}
	delete [] m_slots;
	m_slots = NULL;
	m_slotCnt = 0;
}

HashNode* HashMap::Get(const std::string& key)
{
	HashSlot *slot = GetSlot(key);
	return slot->Get(key);
}

int HashMap::Set(const std::string& key,const std::string& value)
{
	HashSlot *slot = GetSlot(key);
	return slot->Set(key,value);
}

HashSlot* HashMap::GetSlot(const std::string& key)
{
	return m_slots[m_slotHashFunc(key)%m_slotCnt];
}
