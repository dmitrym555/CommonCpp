#ifndef LIBRTPDB70_H
#define LIBRTPDB70_H

#include <cstdint>

#ifndef dword
typedef uint32_t dword;
#endif

#ifndef byte
typedef unsigned char byte;
#endif


extern "C" bool CHECKLICENSE(const char* );
extern "C" int GETPROP(int appId, int rndVal, bool readDllsInfo, dword propId, byte ABlendValue );

extern "C" char* GETMACHINEID();

#endif
