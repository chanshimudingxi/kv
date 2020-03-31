#ifndef SERVER_H_
#define SERVER_H_

#include "connection.h"
#include "connection_container.h"
#include <string>
#include <sys/epoll.h>

#define DEFAULT_LISTEN_PORT 8888
#define DEFAULT_LISTEN_BACKLOG 10
#define DEFAULT_LOOP_WAIT_TIME 1000

class Server {
public:
	Server(int maxFdCount);
	~Server();
    void SetListenPort(int port);
    void SetListenBacklog(int backlog);
    void SetTimeout(int timeout);
    bool Init();
    bool NeedKeepRunning();
    bool Stop();
protected:
    Connection* GetConnection(int fd);
    bool SendPkg(int fd, const void* pkg, int pkgLen);
    bool EnableTcpKeepAlive(int fd, int aliveTime, int interval, int count);
    bool EnableTcpNoDelay(int fd);
    bool Destroy(int fd);
private:
    virtual bool HandleAccept(Connection* c) =0;
    virtual int  RecvPkgHeadLen()=0;//包头长度
    virtual bool HandleRecvPkgHead(Connection* c, const void* pkg, int size, int* pkgLen) = 0;//包头解析
    virtual int  RecvPkgLenMaxLimit()=0;//包的最大长度
    virtual bool HandleRecvPkg(Connection* c, const void* pkg, int pkgLen) = 0;//包解析
    bool Listen();
	bool Accept(int fd);
	bool Read(int fd);
	bool Write(int fd);
protected:
	int m_listenBacklog;//监听队列大小
	int m_listenPort;//监听端口
	int m_maxFdCount;//进程能够打开的描述符最大个数
    int m_timeout;//连接超时时间（单位是秒）

    //管理描述符对应连接的容器
    ConnectionContainer* m_connContainer;
    
    //管理描述符对应事件的容器
    int m_epfd;
    struct epoll_event *m_events;
    int m_maxLoopWaitTime;//等待事件发生的最长时间(单位是毫秒)
};

#endif
