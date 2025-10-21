#ifndef LIBRTPDB64_H
#define LIBRTPDB64_H

#include <cstdint>

#ifndef dword
typedef uint32_t dword;
#endif

#ifndef byte
typedef unsigned char byte;
#endif


extern "C" int SELECTDBPARAM( int appId, int rndVal, bool ReadDllsAndChannelsInfo, byte ABlendValue );
extern "C" int GETDBPARAMINFO( int rndVal, int appId, void* pInfo );

//extern "C" int GETPROP(int appId, int rndVal, bool readDllsInfo, dword propId, byte ABlendValue );

#endif
