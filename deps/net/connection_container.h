#ifndef CONNECTION_CONTAINER_H_
#define CONNECTION_CONTAINER_H_

#include "connection.h"

//管理描述符的容器
//key:描述符
//value:连接

//数组
//1-进程能够打开描述符的最大个数<=容器的容量
//2-依赖内核分配文件描述符的策略是依次递增使用未被使用的描述符，然后回绕。
class ConnectionContainer {
private:
	class Value {
    public:
        Value():use(false),conn(nullptr){}
        ~Value(){}
    public:
		bool use;
		Connection *conn;
	};
public:
	ConnectionContainer(int cap);
	~ConnectionContainer();
	Connection* Get(int key);
	bool Set(int key, Connection* c);
	bool Remove(int key);
    int Size();
private:
	int capacity;//容量
	int size;
	Value* valueArray;
private:
	bool isLegal(int key);
};

#endif 
