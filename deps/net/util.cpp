#include <sstream>
#include <iomanip>
#include <netinet/in.h>
#include "util.h"

std::string DumpHex(char* src,int size){
    if(nullptr == src || size < 1){
        return "";
    }

    std::stringstream ss;
    for(int i=1;i<=size;++i){
        ss<<"0x"<<std::setw(2)<<std::setfill('0')<<std::hex<<(src[i-1]&0xFF)<<" ";
        if(i<size && i%8==0){
            ss<<'\n';
        }
    }

    return ss.str();
}

uint64_t ntohll(uint64_t arg64)
{
    uint64_t res64 = 0;
#if __BYTE_ORDER == __LITTLE_ENDIAN
    uint32_t low = (uint32_t) (arg64 & 0x00000000FFFFFFFFLL);
    uint32_t high = (uint32_t) ((arg64 & 0xFFFFFFFF00000000LL) >> 32);
    low = ntohl(low);
    high = ntohl(high);
    res64 = (uint64_t) high + (((uint64_t) low) << 32);
#else
    res64 = arg64;
#endif    
    return res64;
}

uint64_t htonll(uint64_t arg64)
{
    uint64_t res64 = 0;
#if __BYTE_ORDER == __LITTLE_ENDIAN    
    uint32_t low = (uint32_t) (arg64 & 0x00000000FFFFFFFFLL);
    uint32_t high = (uint32_t) ((arg64 & 0xFFFFFFFF00000000LL) >> 32);

    low = htonl(low);
    high = htonl(high);

    res64 = (uint64_t) high + (((uint64_t) low) << 32);
#else
    res64 = arg64;
#endif    
    return res64;
}
