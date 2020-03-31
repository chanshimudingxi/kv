#ifndef UTIL_H_
#define UITL_H_
#include <string>
#include <stdint.h>

extern std::string DumpHex(char* src,int size);
extern uint64_t ntohll(uint64_t arg64);
extern uint64_t htonll(uint64_t arg64);

#endif
