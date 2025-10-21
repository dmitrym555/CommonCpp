#ifndef AES_H
#define AES_H

#include <cstdint>

#ifndef DWORD
typedef uint32_t DWORD;
#endif

#ifndef BYTE
typedef unsigned char BYTE;
#endif


DWORD* GETEXPKEY(char *Key);
void FREEEXPKEY(DWORD *pExp);
void INITCRYPT(char *Key);
int ENCRYPTBUF(char *Buf, int Len, DWORD *pExp);
int DECRYPTBUF(char *Buf, int Len, DWORD *pExp);
char* ConvXorKeyToUserKey(char *XorStr, char *UsKey);

extern int InitCrypt;
extern char* DefKey;

#endif
