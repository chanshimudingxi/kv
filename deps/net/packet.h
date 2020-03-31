/*
 * 包应该是通过方法来进行描述，是一种接口的集合。
 * 表示能够进行编码和解码。
*/

#ifndef PACKET_H_
#define PACKET_H_

#include <string>

class Packet{
public:
    virtual bool EncodeToString(std::string* str)=0;
    virtual int  PacketHeadLen()=0;
    virtual bool CheckHeadFromArray(const char* buf, int size, int* pkgLen)=0;
    virtual bool DecodeFromArray(const char* buf, int size)=0;
    virtual int  PacketLenMaxLimit()=0;
};

#endif
