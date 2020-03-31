#include "csproto_packet.h"
#include "net/util.h"
#include <netinet/in.h>
#include <string.h>

bool CSProtoPacket::EncodeToString(std::string* str){
    int head_size = PacketHeadLen();
    char* buf = new char[head_size];
    char* cur = buf;
    uint16_t *pVersion = (uint16_t*)cur;
    *pVersion = htons(m_csprotoHead.version);//版本1
    cur+=2;
    uint32_t *pLen = (uint32_t*)cur;
    *pLen = htonl(head_size + CS_PROTO_PACKET_SIZE/2);
    cur+=4;
    uint64_t *uid = (uint64_t*)cur;
    *uid = htonll(m_csprotoHead.uid);
    cur+=8;
    str->clear();
    str->append(buf,head_size);
    str->append(m_csprotoBody,CS_PROTO_PACKET_SIZE/2);
    delete [] buf;
    return true;
}

int  CSProtoPacket::PacketHeadLen(){
    return 2+4+8;
}

bool CSProtoPacket::CheckHeadFromArray(const char* buf, int size, int* pkgLen){
    if(size < PacketHeadLen()){
        return false;
    }
   
    const char* cur = buf;
    cur +=2;

    uint32_t* pLen = (uint32_t*)cur; 
    *pkgLen = ntohl(*pLen);
    
    return true;
}

bool CSProtoPacket::DecodeFromArray(const char* buf, int size){
    if(size < PacketHeadLen()){
        return false;
    }
   
    const char* cur = buf;
    uint16_t *pVersion = (uint16_t*)cur;
    m_csprotoHead.version = ntohs(*pVersion);
    cur +=2;
   
    uint32_t *pLen = (uint32_t*)cur;
    m_csprotoHead.len = ntohl(*pLen);
    cur += 4;
    if(m_csprotoHead.len < PacketHeadLen() || m_csprotoHead.len > PacketLenMaxLimit())
    {
        return false;
    }

    uint64_t *uid = (uint64_t*)cur;
    m_csprotoHead.uid = ntohll(*uid);
   
    int curSize = size - PacketHeadLen();
    if(curSize != CS_PROTO_PACKET_SIZE/2)
    {
        return false;
    }
    memcpy(m_csprotoBody, buf, curSize);
    return true;
}

int CSProtoPacket::PacketLenMaxLimit(){
    return CS_PROTO_PACKET_SIZE;
}

CSProtoHead& CSProtoPacket::Head(){
    return m_csprotoHead;
}