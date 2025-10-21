#include "aes.h"

#include <cstring>
#include <stdio.h>
#include <stdlib.h>

// Циклический сдвиг байта влево на 1
#define ROTL(x)     (((x)>>7)|((x)<<1))
#define ROTL8(x)    (((x)<<8)|((x)>>24))
#define ROTL16(x)   (((x)<<16)|((x)>>16))
#define ROTL24(x)   (((x)<<24)|((x)>>8))

#define Nb 4            // длина блока в DWORD  (128 bit)
#define Nk 4            // длина ключа в DWORD  (128 bit)
#define Nr 10           // число раундов


int InitCrypt;
static int sizeblock = 4*Nb;
char* DefKey = (char*)"@*7B5efls#o%;:v&";
static BYTE PowTab[256];        // Таблица степеней примитивного элемента поля
static BYTE LogTab[256];        // Обратная таблица (по значению можно найти степень)
static BYTE SubBytesTab[256];   // Таблица афинного преобразования SubBytes
static BYTE InvSubBytesTab[256];// Таблица обратного преобразования SubBytes
DWORD* pExpKey = nullptr;

//  Операция умножения элемента поля GF(2^8) a на x.
static BYTE xtime(BYTE a) {
    return ((a<<1) ^ (((a>>7) & 1) * 0x1b));
}

// Перемножение элементов поля GF(2^8)
static BYTE bmul(BYTE x, BYTE y) {
    if (x && y)
        return PowTab[(LogTab[x] + LogTab[y])%255];
    else
        return 0;
}

// Генерирует таблицы степеней примитивного элемента поля и обратную таблицу
static void GenPowerTab(void) {
    int i;

    LogTab[0] = 0;
    PowTab[0] = 1;    // в нулевой степени 1
    PowTab[1] = 3;    // сам примитивный элемент
    LogTab[1] = 0;    // сразу же генерим обратную таблицу
    LogTab[3] = 1;
    for (i = 2; i < 256; i++) {
        PowTab[i] = PowTab[i-1]^xtime(PowTab[i-1]);
        LogTab[PowTab[i]] = i;
    }
}

static BYTE SubBytes(BYTE x) {
    BYTE y = PowTab[255 - LogTab[x]]; // Нахождение обратного элемента

    x = y;  x = ROTL(x);
    y ^= x; x = ROTL(x);
    y ^= x; x = ROTL(x);
    y ^= x; x = ROTL(x);
    y ^= x; y ^= 0x63;
    return y;
}

// Генерирует прямую и обратную таблицы преобразования Subbytes
static void GenSubBytesTab(void) {
    int i; BYTE y;

    SubBytesTab[0] = 0x63; // инициализирующий элемент
    InvSubBytesTab[0x63] = 0;
    for (i = 1; i < 256; i++) {
        y = SubBytes((BYTE) i);
        SubBytesTab[i] = y;
        InvSubBytesTab[y] = i;
    }
}

static DWORD MixCol(BYTE b[4]) {
  BYTE s[4]; DWORD *d = (DWORD*)s;

  s[0] = bmul(0x2, b[0]) ^ bmul(0x3, b[1]) ^ b[2] ^ b[3];
  s[1] = b[0] ^ bmul(0x2, b[1]) ^ bmul(0x3, b[2]) ^ b[3];
  s[2] = b[0] ^ b[1] ^ bmul(0x2, b[2]) ^ bmul(0x3, b[3]);
  s[3] = bmul(0x3, b[0]) ^ b[1] ^ b[2] ^ bmul(0x2, b[3]);
  return *d;
 }

static DWORD InvMixCol(BYTE b[4]) {
  BYTE s[4]; DWORD *d = (DWORD*)s;

  s[0] = bmul(0xe, b[0]) ^ bmul(0xb, b[1]) ^ bmul(0xd, b[2]) ^ bmul(0x9, b[3]);
  s[1] = bmul(0x9, b[0]) ^ bmul(0xe, b[1]) ^ bmul(0xb, b[2]) ^ bmul(0xd, b[3]);
  s[2] = bmul(0xd, b[0]) ^ bmul(0x9, b[1]) ^ bmul(0xe, b[2]) ^ bmul(0xb, b[3]);
  s[3] = bmul(0xb, b[0]) ^ bmul(0xd, b[1]) ^ bmul(0x9, b[2]) ^ bmul(0xe, b[3]);
  return *d;
}

static void MixColumn(DWORD in[Nb]) {
  int i;
  for (i = 0; i < Nb; i++) in[i] = MixCol((BYTE*)&in[i]);
}

static void InvMixColumn(DWORD in[Nb]) {
  int i;
  for (i = 0; i < Nb; i++) in[i] = InvMixCol((BYTE*)&in[i]);
}

static DWORD SubWord(DWORD w) {
  int i;
  BYTE* b = (BYTE*)&w;

  for (i = 0; i < 4; i++) b[i] = SubBytesTab[b[i]];
  return w;
}

// Выполняет расширение ключа
static void KeyExpansion(DWORD key[Nk], DWORD ExKey[Nb*(Nr+1)]) {
  int i;
  BYTE xi = 0x01;
  DWORD temp, w;

  for (i = 0; i < Nk; i++) ExKey[i] = key[i];

  xi = 0x01;
  for (i = Nk; i < Nb*(Nr+1); i++) {
    temp = ExKey[i-1];
    if (i % Nk == 0) {
      w = xi;
      w = ROTL24(xi);
      temp = SubWord(ROTL8(temp)) ^ w;
      xi = xtime(xi);
      }
    ExKey[i] = ExKey[i-Nk] ^ temp;
    }
}

static void AddRoundKey(DWORD in[Nb], DWORD key[Nb]) {
  int i;
  for (i = 0; i < Nb; i++) in[i] ^= key[i];
}

static void Cipher(DWORD block[Nb], DWORD key[Nb*(Nr+1)]) {
  int round, i, j; BYTE* temp;

  for (round = 1; round < Nr; round++) {
    for (i = 0; i < Nb; i++) {
      temp = (BYTE*)&block[i];
      for (j = 0; j < 4; j++) temp[j] = SubBytesTab[temp[j]];
      }
    MixColumn(block);
    AddRoundKey(block, &key[4*round]);
    }

  for (i = 0; i < Nb; i++) {
    temp = (BYTE*)&block[i];
    for (j = 0; j < 4; j++) temp[j] = SubBytesTab[temp[j]];
    }

  AddRoundKey(block, &key[4*Nr]);
}

static void InvCipher(DWORD block[Nb], DWORD key[Nb*(Nr+1)]) {
  int round, i, j; BYTE *temp;

  for(round = Nr-1; round > 0; round--) {
    for (i = 0; i < Nb; i++) {
      temp = (BYTE*)&block[i];
      for (j = 0; j < 4; j++) temp[j] = InvSubBytesTab[temp[j]];
      }
    AddRoundKey(block, &key[4*round]);
    InvMixColumn(block);
    }

  for (i = 0; i < Nb; i++) {
    temp = (BYTE*)&block[i];
    for (j = 0; j < 4; j++) temp[j] = InvSubBytesTab[temp[j]];
    }
  AddRoundKey(block, &key[0]);
}

char* ConvXorKeyToUserKey(char *XorStr, char *UsKey) {
    int i, j, x; char *s, *ss;

    if (!XorStr || !UsKey) return nullptr;
    j = strlen(XorStr)/2; if (!j) return nullptr;
    for (i = 0, s = XorStr, ss = UsKey; i < j; ++i, s += 2, ++ss) {
        sscanf(s,"%2X",&x); *ss = x;
    }
    for (i = 0, s = UsKey, ss = DefKey; i < j; ++i, ++s, ++ss) {
        *s ^= *ss; *s = 0;
    }
    return UsKey;
}

void INITCRYPT(char *Key) {
  int i, j;

  GenPowerTab();
  GenSubBytesTab();
  if (!Key) {InitCrypt = 1; return;}
  // далее исключительно для исполнительной
  if (!pExpKey) pExpKey = (DWORD*)calloc(Nb*(Nr+1), sizeof(DWORD));
  if (!pExpKey) {InitCrypt = 0; return;}
  i = strlen(Key);
  if (i < 16) {for (j = i; j < 16; ++j) *(Key+j) = j; *(Key+j) = 0;}
  KeyExpansion((DWORD*)Key, pExpKey);
  InitCrypt = 1;
}

int ENCRYPTBUF(char *Buf, int Len, DWORD *pExp) {
  int i, j, ChSend; char *in = Buf; DWORD *ExpKey;

  if (!InitCrypt) return Len;
  if (!pExp) ExpKey = pExpKey; else ExpKey = pExp;
  if (!ExpKey) return Len;
  for (i = 0, ChSend = 0; i < Len/sizeblock; ++i) {
    AddRoundKey((DWORD*)in, ExpKey);
    Cipher((DWORD*)in, ExpKey);
    in += sizeblock;
    ChSend += sizeblock;
    }
  i = Len % sizeblock;
  if (i) {
    for (j = i; j < sizeblock; ++j) in[j] = 0;
    AddRoundKey((DWORD*)in, ExpKey);
    Cipher((DWORD*)in, ExpKey);
    ChSend += sizeblock;
    in += sizeblock;
    }
  *in = i; ChSend++;
  return ChSend;
}

int DECRYPTBUF(char *Buf, int Len, DWORD *pExp) {
  int i, ChRecv; char *in = Buf, *EndBuf = Buf+Len; DWORD *ExpKey;

  if (!InitCrypt) return Len;
  if (!pExp) ExpKey = pExpKey; else ExpKey = pExp;
  if (!ExpKey) return Len;
  for (i = 0, ChRecv = 0; i < Len/sizeblock; ++i) {
    AddRoundKey((DWORD*)in, ExpKey+4*Nr);
    InvCipher((DWORD*)in, ExpKey);
    in += sizeblock;
    ChRecv += sizeblock;
    if ((in+sizeblock) > EndBuf) {
      if (*in) ChRecv -= sizeblock-*in;
      break;
      }
    }
  return ChRecv;
}

DWORD* GETEXPKEY(char *Key) {
  int i, j; DWORD *pKey; char LocKey[16];

  if (Key) pKey = (DWORD*)calloc(Nb*(Nr+1), sizeof(DWORD)); else return nullptr;
  if (!pKey) return nullptr;
  i = strlen(Key);
  i = (i > 16) ? 16 : i;
  memcpy(LocKey, Key, i);
  if (i < 16) for (j = i; j < 16; ++j) *(LocKey+j) = j;
  KeyExpansion((DWORD*)LocKey, pKey);
  return pKey;
  }

void FREEEXPKEY(DWORD *pExp) {
  if (pExp) {free(pExp); pExp = nullptr;}
}

