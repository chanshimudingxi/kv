/*
 * author: chanshimudingxi
 * 
 */
 
#ifndef _SKIP_LIST_H_
#define _SKIP_LIST_H_

#include "time.h"
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <string>

#define SKIP_LIST_MAX_LEVEL 32

struct SkipListNode
{
  SkipListNode()
  {
    memset(forward, 0, sizeof(SkipListNode*)*SKIP_LIST_MAX_LEVEL);
  } 
  ~SkipListNode()
  {
    memset(forward, 0, sizeof(SkipListNode*)*SKIP_LIST_MAX_LEVEL);
  }
  std::string key;
  std::string value;
	SkipListNode* forward[SKIP_LIST_MAX_LEVEL];
};

class SkipList
{
public:
	SkipList();
	~SkipList();
	SkipListNode* Find(const std::string& key);
	bool Insert(const std::string& key, const std::string& value);
	bool Delete(const std::string& key);
	SkipListNode* Next(SkipListNode* node);
  SkipListNode* Head();
private:
	SkipListNode* m_pHead;
	int m_level;
	int m_len;
};

#endif
