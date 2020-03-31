#ifndef _MOCK_SKIP_LIST_H_
#define _MOCK_SKIP_LIST_H_

#include "deps/data/skiplist.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

class  VSkipList
{
public:
	VSkipList(){}
	virtual ~VSkipList(){}
	virtual SkipListNode* Find(const std::string& key){return NULL;}
	virtual bool Insert(const std::string& key,const std::string& value){return true;}
	virtual bool Delete(const std::string& key){return true;}
};

class MockVSkipList : public VSkipList
{
public:
	MOCK_METHOD1(Find, SkipListNode*(const std::string& key));
	MOCK_METHOD2(Insert, bool(const std::string& key, const std::string& value));
	MOCK_METHOD1(Delete, bool(const std::string& key));
};

#endif
