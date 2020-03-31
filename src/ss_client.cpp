#include "ss_client.h"

SSClient::SSClient(){}

SSClient::~SSClient(){}

int SSClient::RecvPkgHeadLen(){
    SSProtoPacket sspkt;
    return sspkt.PacketHeadLen();
}

bool SSClient::HandleRecvPkgHead(const void* pkg, int size, int* pkgLen){
    SSProtoPacket sspkt;
    bool res = sspkt.CheckHeadFromArray((char*)pkg,size,pkgLen);
    LOG_DEBUG("fd:%d, size:%d set pkg_len:%d",m_pconn->GetFd(),size,*pkgLen);
    LOG_DEBUG("%s",DumpHex((char*)pkg,size).c_str());
    return res;
}

int SSClient::RecvPkgLenMaxLimit(){
    SSProtoPacket sspkt;
    return sspkt.PacketLenMaxLimit(); 
}

bool SSClient::HandleRecvPkg(const void* pkg, int pkgLen){
    SSProtoPacket sspkt;
    bool res = sspkt.DecodeFromArray((char*)pkg,pkgLen);
    LOG_DEBUG("%s",DumpHex((char*)pkg,pkgLen).c_str());
    LOG_DEBUG("len:%u uin:%llu",sspkt.Head().len,sspkt.Head().seq);
    return res;
}