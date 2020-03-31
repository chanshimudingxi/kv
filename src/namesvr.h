#ifndef NAMESVR_H_
#define NAMESVR_H_

#include "net/server.h"
#include "net/connection.h"
#include <cstring>
#include <iostream>
#include <time.h>
#include "net/util.h"
#include "proto/ssproto_packet.h"
#include "net/log.h"

class Namesvr:public Server{
public:
    Namesvr(int maxFdCount);
    ~Namesvr();
    virtual bool HandleAccept(Connection* c);
    virtual int  RecvPkgHeadLen();
    virtual bool HandleRecvPkgHead(Connection* c, const void* pkg, int size, int* setPkgLen);
    virtual int  RecvPkgLenMaxLimit();
    virtual bool HandleRecvPkg(Connection* c, const void* pkg, int pkgLen);
};

#endif