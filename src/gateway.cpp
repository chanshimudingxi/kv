#include "gateway.h"
#include "proto/ssproto_packet.h"

Gateway::Gateway(int maxFdCount):Server(maxFdCount){}

Gateway::~Gateway(){}

bool Gateway::HandleAccept(Connection* c){
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

int Gateway::RecvPkgHeadLen(){
    CSProtoPacket cspkt;
    return cspkt.PacketHeadLen();
}

bool Gateway::HandleRecvPkgHead(Connection* c, const void* pkg, int size, int* setPkgLen){
    //协议前两个字节默认为包的长度
    CSProtoPacket cspkt;
    bool res = cspkt.CheckHeadFromArray((char*)pkg,size,setPkgLen);
    LOG_DEBUG("fd:%d, set pkg_len:%d",c->GetFd(),*setPkgLen);
    return res;
}

int Gateway::RecvPkgLenMaxLimit(){
    CSProtoPacket cspkt;
    return cspkt.PacketLenMaxLimit();
}

bool Gateway::HandleRecvPkg(Connection* c, const void* pkg, int pkgLen){
    LOG_DEBUG("fd:%d, handle pkg_len:%d",c->GetFd(),pkgLen);
    CSProtoPacket cspkt;
    bool res = cspkt.DecodeFromArray((char*)pkg,pkgLen);
    //LOG_DEBUG("%s",DumpHex((char*)pkg,pkgLen).c_str());
    SSProtoPacket sspkt;
    sspkt.Head().version = cspkt.Head().version;
    sspkt.Head().seq = cspkt.Head().uid;
    std::string val;
    sspkt.EncodeToString(&val);
    m_client->SendPkg(val.data(),val.size());
    return res;
}

bool Gateway::InitClient(){
    m_client = new SSClient();
    m_client->SetTimeout(200);
    m_client->SetAddress("127.0.0.1",9999);
}

int main(){
    Gateway s(10);
    s.SetListenPort(6666);
    s.Init();
    s.InitClient();

    while(true)
    {
        s.NeedKeepRunning();
    }
    return 0;
}