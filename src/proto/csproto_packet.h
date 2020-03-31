/*
 * 协议只说明了数据构成的形式，并不提供具体方法；
 * 包只说明了构成数据的抽象接口，并不提供具体细节。
 * 两者结合才能构成能被实实在在使用的数据。
*/
#ifndef CSPROTO_PACKET_H_
#define CSPROTO_PACKET_H_

#include "proto.h"
#include "net/packet.h"

class CSProtoPacket : public Packet{
public:
    virtual bool EncodeToString(std::string* str);
    virtual int  PacketHeadLen();
    virtual bool CheckHeadFromArray(const char* buf, int size, int* pkgLen);
    virtual bool DecodeFromArray(const char* buf, int size);
    virtual int  PacketLenMaxLimit();
    CSProtoHead& Head();
private:
    CSProtoHead m_csprotoHead;
    char m_csprotoBody[CS_PROTO_PACKET_SIZE/2];
}; 

#endif
