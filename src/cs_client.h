#ifndef CS_CLIENT_H_
#define CS_CLIENT_H_

#include "net/client.h"
#include "net/util.h"
#include "proto/csproto_packet.h"
#include "net/log.h"

class CSClient : public Client{
public:
    CSClient();
    ~CSClient();
public:
    virtual int  RecvPkgHeadLen();
    virtual bool HandleRecvPkgHead(const void* pkg, int size, int* pkgLen);
    virtual int  RecvPkgLenMaxLimit();
    virtual bool HandleRecvPkg(const void* pkg, int pkgLen);
};

#endif