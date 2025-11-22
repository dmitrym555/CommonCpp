//
// C++ Date and time handlers
// Author: Dmitry Melnik
//

#include "KSDateTime.h"

#include <cstring>

#include <iomanip>

#include "../strfmt.h"

bool IsDayLight()
{
//  TTimeZoneInformation TimeZoneInformation;
//  return (GetTimeZoneInformation(&TimeZoneInformation) != TIME_ZONE_ID_STANDARD);
    return true;
}
//---------------------------------------------------------------------------



// -----------------------------------------------------------
// DateTimeToCP56Time2a
// -----------------------------------------------------------
void DateTimeToCP56Time2a(const KSDateTime& DT, bool su, CP56TIME2A& cp56time2a)
{
    //word Year,Month,Day,Hour,Minute,Second,MilliSecond;
    ///DecodeDateTime(DT,Year,Month,Day,Hour,Minute,Second,MilliSecond);
    cp56time2a.msec = DT.dt->second()*1000 + DT.dt->millis(); // Second*1000 + MilliSecond;
    cp56time2a.min = DT.dt->minute();
    cp56time2a.res1 = 0;
    cp56time2a.iv = 0;
    cp56time2a.hour = DT.dt->hour();
    cp56time2a.res2 = 0;
    cp56time2a.su = su;
    cp56time2a.mday = DT.dt->day();
    cp56time2a.wday = 0;
    cp56time2a.month = DT.dt->month();
    cp56time2a.res3 = 0;
    cp56time2a.year = DT.dt->year() % 100;
    cp56time2a.res4 = 0;
}
//---------------------------------------------------------------------------


// -----------------------------------------------------------
// CP56Time2aToDateTime
// -----------------------------------------------------------
bool CP56Time2aToDateTime(const CP56TIME2A cp56time2a, KSDateTime& DT)
{
  try
  {
    WORD Year,Month,Day,Hour,Minute,Second,MilliSecond;
    MilliSecond = cp56time2a.msec % 1000;
    Second = cp56time2a.msec / 1000;
    Minute = cp56time2a.min;
    Hour = cp56time2a.hour;
    Day = cp56time2a.mday;
    Month = cp56time2a.month;
    KSDateTime kdt;
    Year = kdt.dt->year();
    Year = Year - (Year % 100) + cp56time2a.year;
    DT.dt->setDateTime( Year,Month,Day,Hour,Minute,Second,MilliSecond);
    return true;
  }
  catch(...)
  {
    return false;
  }
}
//---------------------------------------------------------------------------

// -----------------------------------------------------------
// KLTIMEtoCP56Time2a
// -----------------------------------------------------------
void KLTIMEtoCP56Time2a(const KLTIME kltime, CP56TIME2A& cp56time2a)
{
    cp56time2a.msec = kltime.Sec * 1000 + kltime.Msec;
    cp56time2a.min = kltime.Min;
    cp56time2a.res1 = 0;
    cp56time2a.iv = 0;
    cp56time2a.hour = kltime.Hour;
    cp56time2a.res2 = 0;
    cp56time2a.su = !kltime.PrLZ;
    cp56time2a.mday = kltime.Day;
    cp56time2a.wday = 0;
    cp56time2a.month = kltime.Mon;
    cp56time2a.res3 = 0;
    if (kltime.Year < 2000) {
        cp56time2a.year = 0;
    }
    else {
        if (kltime.Year > 2099)
            cp56time2a.year = 99;
        else
            cp56time2a.year = kltime.Year % 100;
    }
    cp56time2a.res4 = 0;
}
//---------------------------------------------------------------------------


using namespace std::chrono;
//
// KSDateTime implementation
//
class PKSDateTimeImpl : public PKSDateTime {
    std::chrono::system_clock::time_point tpoint;
public:
    PKSDateTimeImpl();
    virtual word year() override;
    virtual byte month() override;
    virtual byte day() override;
    virtual byte hour() override;
    virtual byte minute() override;
    virtual byte second() override;
    virtual word millis() override;

    virtual void setDateTime( word y, byte m, byte d, byte h=0, byte mm=0, byte s=0, word ms=0 );
};

PKSDateTimeImpl::PKSDateTimeImpl() {
    tpoint = std::chrono::system_clock::now();
}

void PKSDateTimeImpl::setDateTime( word y, byte m, byte d, byte h, byte mm, byte s, word ms ) {
    tm tm;
    tm.tm_year = y;
    tm.tm_mon = m;
    tm.tm_mday = d;
    tm.tm_hour = h;
    tm.tm_min = mm;
    tm.tm_sec = s;
    std::time_t t = std::mktime(&tm);
    tpoint = std::chrono::system_clock::from_time_t( t );
}

word PKSDateTimeImpl::year() {
    time_t tt = system_clock::to_time_t(tpoint);
    tm _tm = *localtime(&tt);
    return _tm.tm_year + 1900;
    //const std::chrono::year_month_day ymd{std::chrono::floor<std::chrono::days>(tpoint)};
    //return (word)(int)ymd.year();
}

byte PKSDateTimeImpl::month() {
    time_t tt = system_clock::to_time_t(tpoint);
    tm _tm = *localtime(&tt);
    return _tm.tm_mon + 1;
//    const std::chrono::year_month_day ymd{std::chrono::floor<std::chrono::days>(tpoint)};
//    return (word)(unsigned)ymd.month();
}

byte PKSDateTimeImpl::day() {
    time_t tt = system_clock::to_time_t(tpoint);
    tm _tm = *localtime(&tt);
    return _tm.tm_mday;
//    const std::chrono::year_month_day ymd{std::chrono::floor<std::chrono::days>(tpoint)};
//    return (word)(unsigned)ymd.day();
}

byte PKSDateTimeImpl::hour() {
    time_t tt = system_clock::to_time_t(tpoint);
    tm _tm = *localtime(&tt);
    return _tm.tm_hour;
//    auto tp = zoned_time{current_zone(), tpoint}.get_local_time();
//    auto dp = floor<days>(tp);
//    year_month_day ymd{dp};
//    hh_mm_ss time{floor<milliseconds>(tp-dp)};
//    auto h = time.hours();
//    return (byte)h.count();
}

byte PKSDateTimeImpl::minute() {
    time_t tt = system_clock::to_time_t(tpoint);
    tm _tm = *localtime(&tt);
    return _tm.tm_min;
}

byte PKSDateTimeImpl::second() {
    time_t tt = system_clock::to_time_t(tpoint);
    tm _tm = *localtime(&tt);
    return _tm.tm_sec;
}

word PKSDateTimeImpl::millis() {
    auto duration = tpoint.time_since_epoch();
    word millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() % 1000;
    return millis;
}


//
// KSDateTime
//
KSDateTime::KSDateTime() {
    dt = new PKSDateTimeImpl();
}

KSDateTime::~KSDateTime() {
    delete dt;
}

void KSDateTime::putTimeStamp(RDATETIME& ts) {
    ts.Year = dt->year() - 2000;
    ts.Mon = dt->month();
    ts.Day = dt->day();
    ts.Hour = dt->hour();
    ts.Min = dt->minute();
    ts.msec = dt->millis();
}


void KSTimeToFILETIME(uint64_t kstime, uint64_t& ft) {
    //RDATETIME rdt;
    //rdt.fromKSTime( kstime );
    std::time_t t1970 = 0;
    uint64_t seconds1 = duration_cast<std::chrono::seconds>( tp2000().time_since_epoch() - std::chrono::system_clock::from_time_t( t1970 ).time_since_epoch() ).count();
    ft = (11644473600LL + seconds1 ) * 10000000LL + (kstime * 10000);
    //RdateTimeToFILETIME( rdt, ft );
}


void RdateTimeToFILETIME(const RDATETIME& ts, uint64_t& ft) {
    tm tm;
    memset( &tm, 0, sizeof(tm) );
    tm.tm_year = ts.Year + 2000 - 1900;
    tm.tm_mon = ts.Mon - 1;
    tm.tm_mday = ts.Day;
    tm.tm_hour = ts.Hour;
    tm.tm_min = ts.Min;
    tm.tm_sec = ts.msec/1000;
    tm.tm_isdst = 0;
//    std::time_t t = timegm(&tm);
    std::time_t t = std::mktime(&tm);
    std::chrono::system_clock::time_point tpoint = std::chrono::system_clock::from_time_t( t );

    ft = (11644473600LL + tpoint.time_since_epoch() / std::chrono::seconds(1)) * 10000000LL + (ts.msec%1000)*10000;
}

uint64_t RdateTimeTotm2000(const RDATETIME& ts) {
    tm tm;
    memset( &tm, 0, sizeof(tm) );
    tm.tm_year = ts.Year + 2000 - 1900;
    tm.tm_mon = ts.Mon - 1;
    tm.tm_mday = ts.Day;
    tm.tm_hour = ts.Hour;
    tm.tm_min = ts.Min;
    tm.tm_sec = ts.msec/1000;
    tm.tm_isdst = 0;
//    std::time_t t = timegm(&tm);
    std::time_t t = std::mktime(&tm);

    uint64_t tm2000 = KSTime( std::chrono::system_clock::from_time_t( t ) ) + ts.msec%1000;
    return tm2000;
}


void cp56dtToFILETIME(const CP56TIME2A& ts, uint64_t& ft) {
    tm tm;
    memset( &tm, 0, sizeof(tm) );
    tm.tm_year = ts.year + 2000 - 1900;
    tm.tm_mon = ts.month - 1;
    tm.tm_mday = ts.mday;
    tm.tm_hour = ts.hour;
    tm.tm_min = ts.min;
    tm.tm_sec = ts.msec / 1000;
    tm.tm_isdst = 0;
//    std::time_t t = timegm(&tm);
    std::time_t t = std::mktime(&tm);
    std::chrono::system_clock::time_point tpoint = std::chrono::system_clock::from_time_t( t );

    ft = (11644473600LL + tpoint.time_since_epoch() / std::chrono::seconds(1)) * 10000000LL;
}

void cp56dtToRDATETIME(const CP56TIME2A& ts, RDATETIME& rdt) {
    rdt.Year = ts.year;
    rdt.Mon = ts.month;
    rdt.Day = ts.mday;
    rdt.Hour = ts.hour;
    rdt.Min = ts.min;
    rdt.msec = ts.msec;
    rdt.PrLZ = ts.su;
}


time_t time_t2000() {
    //std::tm tm;// = {0,0,0,1,0,2000 - 1900};
    //memset( &tm, 0, sizeof(tm) );
    //tm = {0,0,0,1,0,2000 - 1900};
    //tm.tm_isdst = 0;
    //tm.tm_gmtoff = 0;
    static time_t res = 946684800;//timegm( &tm );
    return res;
}

std::chrono::system_clock::time_point tp2000() {
    static std::chrono::system_clock::time_point res = std::chrono::system_clock::from_time_t( time_t2000() );
    return res;
}

uint64_t KSTimeNow() {
    return KSTime( std::chrono::system_clock::now() );
}


uint64_t KSTimeNowLocal() {

    auto tp = std::chrono::system_clock::now();

    const time_zone* tz = std::chrono::current_zone();
    sys_info si = tz->get_info( tp );
    int tzoffset = si.offset.count();
    //->to_local( std::chrono::system_clock::now() );
    uint64_t res = KSTime( tp );
    res += tzoffset * 1000;
    return res;
}




time_t makeTime_t( const std::string& stime ) {
    struct std::tm tm;
    std::istringstream ss(stime);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    std::time_t res = mktime(&tm);
    return res;
}

// milliseconds since 2000.01.01 00:00:00 utc0
uint64_t KSTime( const std::string& stime ) {
    return KSTime( std::chrono::system_clock::from_time_t( makeTime_t( stime ) ) );
}

uint64_t KSTimeFromDate( const std::string& sdate, int delimiterSize ) {
    std::tm tm;
    memset( &tm, 0, sizeof(tm) );
    tm = {0,0,0,1,0,2000 - 1900};
    int pos = 0;
    tm.tm_year = std::stoi( sdate.substr( pos, 4 ) );
    //std::from_chars( sdate.c_str() + pos, sdate.c_str() + pos + 4, tm.tm_year );
    pos += 4+delimiterSize;
    tm.tm_mon = std::stoi( sdate.substr( pos, 2 ) );
    //std::from_chars( sdate.c_str() + pos, sdate.c_str() + pos + 2, tm.tm_mon );
    pos += 2+delimiterSize;
    //std::from_chars( sdate.c_str() + pos, sdate.c_str() + pos + 2, tm.tm_mday );
    tm.tm_mday = std::stoi( sdate.substr( pos, 2 ) );
    pos += 2;
    tm.tm_year -= 1900;
    tm.tm_mon -= 1;

    return KSTime( std::chrono::system_clock::from_time_t(mktime(&tm)) );
}

std::string KSDayTime( uint64_t kstime )
{
    int daySeconds = (kstime%(24*3600000))/1000;
    std::string res = strfmt( "tStart: %02d:%02d:%02d", daySeconds/3600, (daySeconds%3600)/60, daySeconds%60 );
    return res;
}

std::string isodatetime(std::chrono::system_clock::time_point* tp ) {
    auto const now = (tp)? *tp : std::chrono::system_clock::now();
    std::time_t newt = std::chrono::system_clock::to_time_t(now);
    struct tm& t = *localtime(&newt);

    std::string res = strfmt( "%d%02d%02d%02d%02d%02d", t.tm_year +1900, t.tm_mon+1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec );
    return res;
}

std::string isodatetime( uint64_t kstime ) {
    auto tp2000 = std::chrono::system_clock::from_time_t( time_t2000() );
    auto tp1 = tp2000 + std::chrono::milliseconds( kstime );
    return isodatetime( &tp1 );
}


std::string shortdatetime(std::chrono::system_clock::time_point* tp ) {
    auto const now = (tp)? *tp : std::chrono::system_clock::now();
    std::time_t newt = std::chrono::system_clock::to_time_t(now);
    struct tm& t = *localtime(&newt);

    std::string res = strfmt( "%02d %02d:%02d:%02d", t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec );
    return res;
}


std::string standart_datetime(std::chrono::system_clock::time_point* tp ) {
    auto const now = (tp)? *tp : std::chrono::system_clock::now();
    std::time_t newt = std::chrono::system_clock::to_time_t(now);
    struct tm& t = *localtime(&newt);


    std::string res = strfmt( "%d-%02d-%02d %02d:%02d:%02d", t.tm_year +1900, t.tm_mon+1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec );
    return res;
}

std::string standart_datetime( uint64_t kstime, bool milliseconds ) {
    auto tp2000 = std::chrono::system_clock::from_time_t( time_t2000() );
    auto tp1 = tp2000 + std::chrono::milliseconds( kstime );
    std::string res = standart_datetime( &tp1 );
    if ( milliseconds ) {
        res += std::format( ".{}", kstime % 1000 );
    }
    return res;
}

std::string standart_date( int ksdate ) {
    std::string res = standart_datetime( (uint64_t)ksdate * 24*3600000 );
    res = res.substr(0, 10);
    return res;
}

///*
void RDATETIME::fromKSTime( uint64_t kstime ) {
    auto tp2000 = std::chrono::system_clock::from_time_t( time_t2000() );
    auto tp1 = tp2000 + std::chrono::milliseconds( kstime );
    std::time_t newt = std::chrono::system_clock::to_time_t(tp1);
    struct tm& t = *localtime(&newt);

    Year = t.tm_year + 1900 - 2000;
    Mon = t.tm_mon + 1;
    Day = t.tm_mday;
    Hour = t.tm_hour;
    Min = t.tm_min;
    Sec = t.tm_sec;
    msec = t.tm_sec*1000 + kstime % 1000;
}
//*/

uint64_t RDATETIME::toKSTime() const {
    return RdateTimeTotm2000( *this );
}


uint64_t KLDATETIME64::toKsTime() const {
    tm tm;
    memset( &tm, 0, sizeof(tm) );
    tm.tm_year = year + 2000 - 1900;
    tm.tm_mon = mon - 1;
    tm.tm_mday = day;
    tm.tm_hour = hour;
    tm.tm_min = min;
    tm.tm_sec = sec;
    tm.tm_isdst = 0;
    std::time_t t = std::mktime(&tm);

    uint64_t tm2000 = KSTime( std::chrono::system_clock::from_time_t( t ) );
    return tm2000;

}

void RDATETIME::fromKLDATETIME64( const KLDATETIME64& src ) {
    Year = src.year - 2000;
    Mon  = src.mon;
    Day  = src.day;
    Hour = src.hour;
    Min  = src.min;
    Sec  = src.sec;
    msec = src.sec*1000;
}

void KLTIME::fromKSTime( uint64_t kstime ) {
    auto tp2000 = std::chrono::system_clock::from_time_t( time_t2000() );
    auto tp1 = tp2000 + std::chrono::milliseconds( kstime );
    std::time_t newt = std::chrono::system_clock::to_time_t(tp1);
    struct tm& t = *localtime(&newt);

    Year = t.tm_year + 1900;
    Mon = t.tm_mon + 1;
    Day = t.tm_mday;
    Hour = t.tm_hour;
    Min = t.tm_min;
    Sec = t.tm_sec;
    Msec = kstime % 1000;
}

void CP56TIME2A::fromKSTime( uint64_t kstime ) {
    auto tp2000 = std::chrono::system_clock::from_time_t( time_t2000() );
    auto tp1 = tp2000 + std::chrono::milliseconds( kstime );
    std::time_t newt = std::chrono::system_clock::to_time_t(tp1);
    struct tm& t = *localtime(&newt);

    year = (t.tm_year + 1900) % 100;
    month = t.tm_mon + 1;
    mday = t.tm_mday;
    hour = t.tm_hour;
    min = t.tm_min;
    msec = kstime % (1000*60);
}


uint64_t CP56TIME2A::toKSTime() {
    tm tm;
    memset( &tm, 0, sizeof(tm) );
    tm.tm_year = year + 2000 - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = mday;
    tm.tm_hour = hour;
    tm.tm_min = min;
    tm.tm_sec = msec / 1000;
    tm.tm_isdst = 0;

#ifdef _WIN32
    std::time_t t = std::mktime(&tm);
#else
    std::time_t t = timegm(&tm);
#endif

    uint64_t tm2000 = KSTime( std::chrono::system_clock::from_time_t( t ) ) + msec%1000;
    return tm2000;
}
