#include "namesvr.h"

Namesvr::Namesvr(int maxFdCount):Server(maxFdCount){}

Namesvr::~Namesvr(){}

bool Namesvr::HandleAccept(Connection* c){
    if(!EnableTcpKeepAlive(c->GetFd(),60,30,2)){
        LOG_ERROR("tcp_keep_alive set failed");
        return false;
    }

    if(!EnableTcpNoDelay(c->GetFd())){
        LOG_ERROR("tcp_no_delay set failed");
        return false;
    }
    return true;
}

int Namesvr::RecvPkgHeadLen(){
    SSProtoPacket sspkt;
    return sspkt.PacketHeadLen();
}

bool Namesvr::HandleRecvPkgHead(Connection* c, const void* pkg, int size, int* setPkgLen){
    //协议前两个字节默认为包的长度
    SSProtoPacket sspkt;
    bool res = sspkt.CheckHeadFromArray((char*)pkg,size,setPkgLen);
    LOG_DEBUG("fd:%d, set pkg_len:%d",c->GetFd(),*setPkgLen);
    return res;
}

int Namesvr::RecvPkgLenMaxLimit(){
    SSProtoPacket sspkt;
    return sspkt.PacketLenMaxLimit();
}

bool Namesvr::HandleRecvPkg(Connection* c, const void* pkg, int pkgLen){
    LOG_DEBUG("fd:%d, handle pkg_len:%d",c->GetFd(),pkgLen);
    SSProtoPacket sspkt;
    bool res = sspkt.DecodeFromArray((char*)pkg,pkgLen);
    LOG_DEBUG("%s",DumpHex((char*)pkg,pkgLen).c_str());
    return res;
}

int main(){
    Namesvr s(10);
    s.SetListenPort(9999);
    s.Init();

    while(true)
    {
        s.NeedKeepRunning();
    }
    return 0;
}