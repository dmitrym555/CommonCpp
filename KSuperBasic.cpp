//
// C++ common units
// Author: Dmitry Melnik
//

#include "KSuperBasic.h"

#include <stdio.h>

#include <cstring>

void KSGUID::fromStr( const char* sguid ) {
	sscanf( sguid,
	       "{%8x-%4hx-%4hx-%2hhx%2hhx-%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx}",
	       &Data1, &Data2, &Data3,
	       &Data4[0], &Data4[1], &Data4[2], &Data4[3],
	       &Data4[4], &Data4[5], &Data4[6], &Data4[7] );
}

void KSGUID::toChars( char* guid ) {
    sprintf( guid, "{%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}",
        (long unsigned int)Data1, Data2, Data3,
        Data4[0], Data4[1], Data4[2], Data4[3],
        Data4[4], Data4[5], Data4[6], Data4[7] );
}

std::string KSGUID::toStr() {
    std::string res;
    res.resize( 38 );
    toChars( res.data() );
    return res;
}


void CLpwdstring::decrypt() {
    for ( size_t iii=0; iii < length(); ++iii ) {
        data()[iii] = data()[iii] ^ 177;
    }
}

void CLpwdstring::destroy() {
    memset( this->data(), 0, this->length() );
}

CLpwdstring::~CLpwdstring() {
    destroy();
}
