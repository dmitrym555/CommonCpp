//
// CommonCPP utils
// Author: Dmitry Melnik
//

#ifndef KSUTIL_H
#define KSUTIL_H

#include <cstdlib>
#include <cinttypes>

#include <string_view>

#include <string>
#include <chrono>
#include <vector>

#include "../strfmt.h"

#include "../KSuperBasic.h"

#include <atomic>

#ifndef TKSRtpParamId
typedef uint32_t TKSRtpParamId;
#endif

#define BYTEBUF( sz ) \
		byte bytebuf[4096];\
		byte* bufptr = bytebuf;\
		std::unique_ptr<byte[]> bytebufptr;\
		if ( sz > sizeof(bytebuf) ) {\
            bytebufptr.reset( new byte[sz] );\
            bufptr = bytebufptr.get();\
		}


int KSGettid();

int KSGetpid();

int getProcPPid( int pid );

uint64_t KSGetTickCount();
void KSsleep(int sleepForMs);
void KSSleepFinish();

uint32_t getValueBE(void* pmem);

class KSTimeDiff {
    uint64_t   m_ticks = KSGetTickCount();
public:
    bool check(uint64_t tdiff) {
        uint64_t ticks = KSGetTickCount();
        if ( ticks - m_ticks > tdiff ) {
            m_ticks = ticks;
            return true;
        }
        return false;
    }
    void reset() {
        m_ticks = KSGetTickCount();
    }
};

void KSsleep( int sleepForMs, bool& cancel );


struct KSFileInfo {
    uint64_t  fsize = 0;
    int       accessTime = 0;
    int       createTime = 0;
    int       modifTime = 0;
};

bool GetFileInfo(const std::string& filename, KSFileInfo& finfo );


int64_t GetFileSize(const std::string& filename);

int fastIntToStr(char* pbuf, int val);

int KSStrToInt( const std::string& str );
int KSStrToInt( const std::string& str, int defVal );
int KSStrToInt( const std::string_view& str, int defVal );

std::string& trim(std::string& s, const char* t = " \t\n\r\f\v\0");

std::string strExpandTabs( const std::string& str, int tabSize=8 );
size_t strVisibleSize( const std::string& str, int tabSize=8 );
size_t subStrSize( const std::string& str, size_t visibleChars, size_t tabSize=8 );


std::wstring fromBytes( const std::string& s );
std::string utf8(const std::wstring& wstr);
std::wstring utf8( const std::string& s );
std::wstring utf8( const char* s );
int cp1251_to_utf8(char *out, const char *in, int buflen);
void cp1251_to_utf8( const std::string& s, std::string& out );
std::string cp1251_to_utf8( const std::string& s );
size_t utf8charSizeAtPos( const std::string& str, size_t pos );
size_t utf8_position(const std::string& utf8_string, size_t visible_chars);
size_t utf8charSizeAtPos( const char* str );

std::string KSExtractFilePath(const std::string& filePath);
bool KSFileExists(const std::string& filePath);

//std::vector<std::string> split(const std::string& s, const std::string& delimiter);

template<class T>
std::vector<std::basic_string<T>> split(const std::basic_string<T>& s, const std::basic_string<T>& delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::basic_string<T> token;
    std::vector<std::basic_string<T>> res;
    if (s.length()==0)
        return res;

    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
        token = s.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back (token);
    }

    res.push_back(s.substr (pos_start) );
    return res;
}




struct KSSplitItem {
    size_t pos;
    size_t len;
};

int KSSplit( const std::string_view& s, const std::string& delimiter, KSSplitItem* pdata, int maxDataSize );

bool strMatchAny( const std::string& str, const std::vector<std::string>& strings );

std::string removeOuterCharacters(const std::string& s, int count=1 );

std::string getDBasePath(const std::string& KLoggerCfgPath );

void removeLastPathComponent( std::string& str, const std::string& delimiter = "/" );

template<class T1> std::string getXmlChildElValue( const T1* xmlel, const char* name, const char* defaultValue )
{
    xmlel = xmlel->FirstChildElement(name);
    if ( !xmlel )
        return defaultValue;

    const char* ps = xmlel->GetText();
    if ( !ps )
        return defaultValue;
    std::string res = ps;
    return res;
}

template<class T1> const T1* getXmlDeepElValue( const T1* xmlel, std::string path ) {

    std::vector<std::string> spl = split<char>( path, "/" );
    for ( const std::string& name : spl ) {
        xmlel = xmlel->FirstChildElement( name.c_str() );
        if ( xmlel == nullptr )
            return nullptr;
    }
    return xmlel;
}


class KSAtomicFlagGuard {
    std::atomic_flag* m_flag;
public:
    KSAtomicFlagGuard(std::atomic_flag* pflag) { m_flag = pflag; }
    ~KSAtomicFlagGuard() { std::atomic_flag_clear_explicit(m_flag, std::memory_order_release); }
};

uint32_t DWCalcCrc(const void* in, uint32_t length );
uint64_t QWCalcCrc(const void* in, uint32_t length );

void ksMemCpy(void* dst, const void* src, dword len, int& pointer);

std::string ksDoubleToStr( double d );

std::string dbParamIdPretty( TKSRtpParamId parId );

std::string urlDecode(const std::string& src);


struct KSProcEntry {
    int          pid;
    std::string  path;
};

void getProcIdsByName( const std::string& procName, std::vector<KSProcEntry>& out );

std::string execShellCmd( const std::string& cmd, int timeout );


int KSMakeDir( const std::string& path );

bool KSMakeDirs( const std::string& path );

void KSMakeDirsThrow(const std::string& path );

class KSFileAuto {
    FILE* m_file = nullptr;
public:
    KSFileAuto( FILE* fff ) { m_file = fff; }
    ~KSFileAuto() { if (m_file) fclose( m_file ); }
};

class KSPipeAuto {
    FILE* m_file = nullptr;
public:
    KSPipeAuto( FILE* fff ) { m_file = fff; }
    ~KSPipeAuto() { if (m_file) pclose( m_file ); }
};


std::string getFileExt( const std::string& fpath );


int KSRandom( int limit );


std::string strGetLeft( const std::string& str, const char* delimiter );
std::string strGetRight( const std::string& str, const std::string& delimiter, bool returnSource = false );

std::string keyval( const std::string& src, const std::string& delimiter, std::string& val );

std::string join( const std::vector<std::string>& lines, const std::string& delimiter );
std::string replace_all(std::string str, const std::string& from, const std::string& to);
std::string KSReplaceAll( std::string str, std::vector<std::string>& fromList, std::vector<std::string>& toList );

bool KSFileAppend( const std::string& fpath, const std::string& str );

#endif // KSUTIL_H
