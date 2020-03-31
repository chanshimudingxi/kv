#include "data/hashmap.h"
#include "gtest/gtest.h"
#include <stdio.h>
#include <string>

class SimpleGetSetTest : public ::testing::Test
{
public: 
	virtual void SetUp() {}  
	virtual void TearDown() {}  
protected:
	HashMap test;
};

TEST_F(SimpleGetSetTest,SetTest1)
{
	for(int i=0;i<1000000;i++)
	{
		char key[15];
		char value[15];
		snprintf(key,sizeof(key),"key=%d",i);
		snprintf(value,sizeof(value),"value1=%d",i);
		std::string strKey(key);
		std::string strValue(value);
		test.Set(strKey,strValue);
	}
	SUCCEED();
}

TEST_F(SimpleGetSetTest,GetTest1)
{
	for(int i=0;i<1000000;i++)
	{
		char key[15];
		char value[15];
		snprintf(key,sizeof(key),"key=%d",i);
		snprintf(value,sizeof(value),"value1=%d",i);
		std::string strKey(key);
		std::string strValue(value);
		test.Set(strKey,strValue);
	}
	SUCCEED();

	for(int i=0;i<1000000;i++)
	{
		char key[15];
		char value[15];
		snprintf(key,sizeof(key),"key=%d",i);
		snprintf(value,sizeof(value),"value1=%d",i);
		std::string strKey(key);
		HashNode* vNode = test.Get(strKey);
		ASSERT_TRUE(vNode!=NULL);
		ASSERT_STREQ(value,vNode->value.c_str());
	}
}

TEST_F(SimpleGetSetTest,SetTest2)
{
	for(int i=0;i<1000000;i++)
	{
		char key[15];
		char value[15];
		snprintf(key,sizeof(key),"key=%d",i);
		snprintf(value,sizeof(value),"value2=%d",i);
		std::string strKey(key);
		std::string strValue(value);
		test.Set(strKey,strValue);
	}
	SUCCEED();
}

TEST_F(SimpleGetSetTest,GetTest2)
{
	for(int i=0;i<1000000;i++)
	{
		char key[15];
		char value[15];
		snprintf(key,sizeof(key),"key=%d",i);
		snprintf(value,sizeof(value),"value2=%d",i);
		std::string strKey(key);
		std::string strValue(value);
		test.Set(strKey,strValue);
	}
	SUCCEED();

	for(int i=0;i<1000000;i++)
	{
		char key[15];
		char value[15];
		snprintf(key,sizeof(key),"key=%d",i);
		snprintf(value,sizeof(value),"value2=%d",i);
		std::string strKey(key);
		HashNode* vNode = test.Get(strKey);
		ASSERT_STREQ(value,vNode->value.c_str());
	}
}

int main(int argc, char **argv) 
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
