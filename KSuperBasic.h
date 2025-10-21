//
// C++ common units
// Author: Dmitry Melnik
//

#ifndef KSUPERBASIC_H
#define KSUPERBASIC_H

#include <cstdint>
#include <string>

#ifndef word
typedef unsigned short word;
#endif

#ifndef byte
typedef unsigned char byte;
#endif

#ifndef ulong
typedef unsigned long ulong;
#endif

#ifndef uint
typedef unsigned int uint;
#endif

#ifndef ushort
typedef unsigned short ushort;
#endif

#ifndef dword
typedef uint32_t dword;
#endif


#ifndef BYTE
typedef unsigned char BYTE;
#endif



#pragma pack(push, 1)
struct KSGUID {
    dword Data1;
    word  Data2;
    word  Data3;
    byte  Data4[8];

    void fromStr( const char* guid );
    void toChars( char* guid );
    std::string toStr();
};
#pragma pack(pop)

class CLpwdstring : public std::string
{
public:
    ~CLpwdstring();

    void decrypt();
    void destroy();
};


#endif
