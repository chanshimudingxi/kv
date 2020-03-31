#include "cs_client.h"

CSClient::CSClient(){}

CSClient::~CSClient(){}

int CSClient::RecvPkgHeadLen(){
    CSProtoPacket cspkt;
    return cspkt.PacketHeadLen();
}

bool CSClient::HandleRecvPkgHead(const void* pkg, int size, int* pkgLen){
    CSProtoPacket cspkt;
    bool res = cspkt.CheckHeadFromArray((char*)pkg,size,pkgLen);
    LOG_DEBUG("fd:%d, size:%d set pkg_len:%d",m_pconn->GetFd(),size,*pkgLen);
    LOG_DEBUG("%s",DumpHex((char*)pkg,size).c_str());
    return res;
}

int CSClient::RecvPkgLenMaxLimit(){
    CSProtoPacket cspkt;
    return cspkt.PacketLenMaxLimit(); 
}

bool CSClient::HandleRecvPkg(const void* pkg, int pkgLen){
    CSProtoPacket cspkt;
    bool res = cspkt.DecodeFromArray((char*)pkg,pkgLen);
    LOG_DEBUG("%s",DumpHex((char*)pkg,pkgLen).c_str());
    LOG_DEBUG("len:%u uin:%llu",cspkt.Head().len,cspkt.Head().uid);
    return res;
}