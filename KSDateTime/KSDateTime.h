//
// C++ Date and time handlers
// Author: Dmitry Melnik
//

#ifndef KSDATETIME_H
#define KSDATETIME_H

#include <cinttypes>
#include <time.h>

#include <chrono>

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

#ifndef WORD
typedef unsigned short WORD;
#endif

#pragma pack(push, 1)
// CP56Time2a timestamp

typedef struct _CP56TIME2A //
{
  WORD msec;     // 16
  BYTE min	:6;  // 22
  BYTE res1	:1;  // 23
  BYTE iv	:1;  // 24
  BYTE hour	:5;  // 29
  BYTE res2	:2;  // 31
  BYTE su	:1;  // 32
  BYTE mday	:5;  // 37
  BYTE wday	:3;  // 40
  BYTE month:4;  // 44
  BYTE res3	:4;  // 48
  BYTE year	:7;  // 55
  BYTE res4	:1;  // 56

  void fromKSTime( uint64_t kstime );
  uint64_t toKSTime();
} CP56TIME2A;


struct KLDATETIME32 {
    unsigned int Sec  : 6;  //  6  0...59
    unsigned int Min  : 6;  // 12  0...59
    unsigned int Hour : 5;  // 17  0...23
    unsigned int Day  : 5;  // 22  1...31
    unsigned int Mon  : 4;  // 26  1...12
    unsigned int Year : 5;  // 31  0...31 + 2000
    unsigned int PrLZ : 1;  // 32  0 - лето, 1 - зима
};

struct KLDATETIME64 {
    WORD   year;
    BYTE   mon;
    BYTE   day;
    BYTE   hour;
    BYTE   min;
    BYTE   sec;
    BYTE   reserv; // для бита летнее/зимнее, и пр.

    uint64_t toKsTime() const;
};


struct RDATETIME
{
    unsigned int Sec  : 6;  //  6  0...59
    unsigned int Min  : 6;  // 12  0...59
    unsigned int Hour : 5;  // 17  0...23
    unsigned int Day  : 5;  // 22  1...31
    unsigned int Mon  : 4;  // 26  1...12
    unsigned int Year : 5;  // 31  0...31 + 2000
    unsigned int PrLZ : 1;  // 32  0 - лето, 1 - зима
    WORD         msec;

    void fromKSTime( uint64_t kstime );
    uint64_t toKSTime() const;
    void fromKLDATETIME64( const KLDATETIME64& src );
};


struct KLTIME
{
    short  Year;
    short  Mon;
    short  Day;
    short  Hour;
    short  Min;
    short  Sec;
    short  Msec;
    short  PrLZ;

    void fromKSTime( uint64_t kstime );
};

#pragma pack(pop)

bool IsDayLight();
time_t time_t2000();

class PKSDateTime {

public:
    virtual word year()=0;
    virtual byte month()=0;
    virtual byte day()=0;
    virtual byte hour()=0;
    virtual byte minute()=0;
    virtual byte second()=0;
    virtual word millis()=0;

    virtual void setDateTime(word y, byte m, byte d, byte h=0, byte mm=0, byte s=0, word ms=0 )=0;

    virtual ~PKSDateTime() {}
};

class KSDateTime {
public:
    PKSDateTime* dt;
    KSDateTime();
    ~KSDateTime();

    void putTimeStamp(RDATETIME& ts);
};

void KSTimeToFILETIME(uint64_t kstime, uint64_t& ft);
void RdateTimeToFILETIME(const RDATETIME& ts, uint64_t& ft);
uint64_t RdateTimeTotm2000(const RDATETIME& ts);

void DateTimeToCP56Time2a(const KSDateTime& DT, bool su, CP56TIME2A& cp56time2a);

void cp56dtToFILETIME(const CP56TIME2A& ts, uint64_t& ft);

void cp56dtToRDATETIME(const CP56TIME2A& ts, RDATETIME& rdt);

std::chrono::system_clock::time_point tp2000();

template<class T>
uint64_t KSTime( std::chrono::time_point<T> tp ) {
    using namespace std::chrono;
    uint64_t res = duration_cast<std::chrono::milliseconds>( tp.time_since_epoch() - tp2000().time_since_epoch() ).count();
    return res;
}

time_t makeTime_t( const std::string& stime );
uint64_t KSTime( const std::string& stime );
uint64_t KSTimeFromDate( const std::string& sdate, int delimiterSize=1 );

uint64_t KSTimeNow();
uint64_t KSTimeNowLocal();

std::string KSDayTime( uint64_t kstime );

std::string isodatetime(std::chrono::system_clock::time_point* tp = nullptr);
std::string standart_datetime(std::chrono::system_clock::time_point* tp = nullptr);
std::string shortdatetime(std::chrono::system_clock::time_point* tp = nullptr );

std::string isodatetime( uint64_t kstime );
std::string standart_datetime( uint64_t kstime, bool milliseconds = false );

std::string standart_date( int ksdate );


struct KSTInterval {
    uint64_t  tstart = 0;
    int64_t   tduration = 0;
    uint64_t  tend() const { return tstart + tduration; }
    void reset() { tstart = tduration = 0; }
};


#endif // KSDATETIME_H
