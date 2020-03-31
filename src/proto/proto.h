/*
 * 协议是一种数据传输规则，这里是cs的传输协议。协议至少也需要包含CSProtoHead，
 * 以下注释说明各个字段的含义。
*/

#ifndef PROTO_H_
#define PROTO_H_
#include <stdint.h>

#pragma pack(1)
struct CSProtoHead{
    uint16_t version;//协议版本
    uint32_t len;//包长度
    uint64_t uid;//用户id
};
#pragma pack()

#pragma pack(1)
struct SSProtoHead{
    uint16_t version;//协议版本
    uint32_t len;//包长度
    uint64_t seq;//消息id
};
#pragma pack()

const uint32_t CS_PROTO_PACKET_SIZE = 1024*1024*4;
const uint32_t SS_PROTO_PACKET_SIZE = 1024*1024*16;

#endif
