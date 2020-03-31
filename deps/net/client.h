#ifndef CLIENT_H_
#define CLIENT_H_

#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/resource.h>
#include <errno.h>
#include <time.h>
#include <assert.h>
#include <strings.h>
#include <arpa/inet.h>
#include <string.h>
#include "connection.h"

class Client{
#define DEFAULT_TIMEOUT 800
public:
    Client();
    ~Client();
    Client(const Client&)=delete;
    Client& operator=(const Client&)=delete;
    int GetFd();
    void SetAddress(std::string ip, int port);
    void SetTimeout(int timeout);
    void Destroy();
    bool SendPkg(const void* pkg,int pkgLen);
private:
    virtual int  RecvPkgHeadLen()=0;
    virtual bool HandleRecvPkgHead(const void* pkg, int size, int *pkgLen)=0;
    virtual int  RecvPkgLenMaxLimit()=0;
    virtual bool HandleRecvPkg(const void* pkg, int size)=0;
    bool Connect(); 
    bool Read();
    bool Write();
protected:
    std::string m_ip;
    int m_port;
    int m_timeout;//超时时间，单位为ms
    Connection* m_pconn;
};

#endif
