#ifndef SS_CLIENT_H_
#define SS_CLIENT_H_

#include "net/client.h"
#include "net/util.h"
#include "proto/ssproto_packet.h"
#include "net/log.h"

class SSClient : public Client{
public:
    SSClient();
    ~SSClient();
public:
    virtual int  RecvPkgHeadLen();
    virtual bool HandleRecvPkgHead(const void* pkg, int size, int* pkgLen);
    virtual int  RecvPkgLenMaxLimit();
    virtual bool HandleRecvPkg(const void* pkg, int pkgLen);
};

#endif