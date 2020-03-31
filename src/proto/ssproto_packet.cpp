#include "ssproto_packet.h"
#include "net/util.h"
#include <netinet/in.h>
#include <string.h>

bool SSProtoPacket::EncodeToString(std::string* str){
    int size = PacketHeadLen();
    char* buf = new char[size];
    char* cur = buf;
    uint16_t *pVersion = (uint16_t*)cur;
    *pVersion = htons(m_ssprotoHead.version);//版本1
    cur+=2;
    uint32_t *pLen = (uint32_t*)cur;
    *pLen = htonl(size);
    cur+=4;
    uint64_t *seq = (uint64_t*)cur;
    *seq = htonll(m_ssprotoHead.seq);
    cur+=8;
    str->clear();
    str->append(buf,size);
    delete [] buf;
    return true;
}

int  SSProtoPacket::PacketHeadLen(){
    return 2+4+8;
}

bool SSProtoPacket::CheckHeadFromArray(const char* buf, int size, int* pkgLen){
    if(size < PacketHeadLen()){
        return false;
    }
   
    const char* cur = buf;
    cur +=2;

    uint32_t* pLen = (uint32_t*)cur; 
    *pkgLen = ntohl(*pLen);
    
    return true;
}

bool SSProtoPacket::DecodeFromArray(const char* buf, int size){
    if(size < PacketHeadLen()){
        return false;
    }
   
    const char* cur = buf;
    uint16_t *pVersion = (uint16_t*)cur;
    m_ssprotoHead.version = ntohs(*pVersion);
    cur +=2;
   
    uint32_t *pLen = (uint32_t*)cur;
    m_ssprotoHead.len = ntohl(*pLen);
    cur += 4;
    if(m_ssprotoHead.len < PacketHeadLen() || m_ssprotoHead.len > PacketLenMaxLimit())
    {
        return false;
    }

    uint64_t *seq = (uint64_t*)cur;
    m_ssprotoHead.seq = ntohll(*seq);
   
    int curSize = size - PacketHeadLen();
    return true;
}

int SSProtoPacket::PacketLenMaxLimit(){
    return SS_PROTO_PACKET_SIZE;
}

SSProtoHead& SSProtoPacket::Head(){
    return m_ssprotoHead;
}