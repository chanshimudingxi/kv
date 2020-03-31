#ifndef SSPROTO_PACKET_H_
#define SSPROTO_PACKET_H_

#include "proto.h"
#include "net/packet.h"

class SSProtoPacket : public Packet{
public:
    virtual bool EncodeToString(std::string* str);
    virtual int  PacketHeadLen();
    virtual bool CheckHeadFromArray(const char* buf, int size, int* pkgLen);
    virtual bool DecodeFromArray(const char* buf, int size);
    virtual int  PacketLenMaxLimit();
    SSProtoHead& Head();
private:
    SSProtoHead m_ssprotoHead;
}; 

#endif
