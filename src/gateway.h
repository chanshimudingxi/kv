#ifndef GATEWAY_H_
#define GATEWAY_H_

#include "net/server.h"
#include "net/connection.h"
#include <cstring>
#include <iostream>
#include <time.h>
#include "net/util.h"
#include "proto/csproto_packet.h"
#include "net/log.h"
#include "ss_client.h"

class Gateway:public Server{
public:
    Gateway(int maxFdCount);
    ~Gateway();
    virtual bool HandleAccept(Connection* c);
    virtual int  RecvPkgHeadLen();
    virtual bool HandleRecvPkgHead(Connection* c, const void* pkg, int size, int* setPkgLen);
    virtual int  RecvPkgLenMaxLimit();
    virtual bool HandleRecvPkg(Connection* c, const void* pkg, int pkgLen);
    bool InitClient();
private:
    SSClient* m_client;
};

#endif