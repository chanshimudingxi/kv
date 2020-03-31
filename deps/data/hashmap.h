/*
* author:chanshimudingxi
*/
#include "generalhashfunctions.h"
#include <string>

struct HashNode
{
	std::string value;
	std::string key;
	HashNode* next;
};

class HashSlot
{
#define DEFAULT_CAPACITY 3
public:
	HashSlot(HashFunction hashFunc = DJBHash,unsigned int capacity=DEFAULT_CAPACITY);
	~HashSlot();
	HashNode* Get(const std::string& key);
	int Set(const std::string& key, const std::string& value);
private:	
	HashNode** m_nodes[2];
	unsigned int m_usedIndex;
	HashFunction m_hashFunc;
	unsigned int m_keyCnt;
	unsigned int m_keyCapacity;
};

class HashMap
{
#define DEFAULT_SLOT_CNT 10
public:
	HashMap(int slotCnt = DEFAULT_SLOT_CNT,HashFunction slotHashFunc = DJBHash,HashFunction nodeHashFunc = DJBHash);
	~HashMap();	
	HashNode* Get(const std::string& key);
	int Set(const std::string& key,const std::string& value);
private:
	HashSlot* GetSlot(const std::string& key);
private:
	HashFunction m_slotHashFunc;
	HashFunction m_nodeHashFunc;
	HashSlot** m_slots;
	unsigned int m_slotCnt;
};
