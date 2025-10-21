//
// CommonCPP utils
// Author: Dmitry Melnik
//

#include "KSUtil.h"

#include <stdarg.h>
#include <fstream>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <iostream>

#include <fcntl.h>
//#include <poll.h>

#include <charconv>
#include <filesystem>
#include <sys/stat.h>

#include <signal.h>

#include <locale>
#include <codecvt>
#include <cstring>
#include <thread>

#include <stdlib.h>

#include "../crc32/crc32.hpp"
#include <cmath>


#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

#include "../Platform/Platform.h"


using namespace std::chrono;

int KSGettid() {
    pid_t tid = KSGetThreadId();
    return tid;
}

uint64_t KSGetTickCount()
{
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

bool KSAllowSleep = true;

void KSSleepFinish() {
    KSAllowSleep = false;
}

void KSsleep(int sleepForMs) {
    if ( KSAllowSleep )
        std::this_thread::sleep_for( std::chrono::milliseconds( sleepForMs ) );
}

void KSsleep( int sleepForMs, bool& cancel ) {
    KSTimeDiff tdiff;
    while ( !cancel ) {
        if ( tdiff.check( sleepForMs ) )
            break;
        KSsleep(500);
    }
}

uint32_t getValueBE(void* pmem) {
    byte* pmemin = (byte*)pmem;
    byte buf[sizeof(uint32_t)];
    for ( size_t i=0; i < sizeof(buf); ++i ) {
        buf[sizeof(buf)-1-i] = pmemin[i];
    }
    uint32_t res = *(uint32_t*)buf;
    return res;
}

bool GetFileInfo(const std::string& filename, KSFileInfo& finfo )
{
    struct stat fstat;
    memset( &fstat, 0, sizeof( fstat ) );
    int rc = stat(filename.c_str(), &fstat);
    if ( rc != 0 )
        return false;
    finfo.fsize = fstat.st_size;
#ifdef _WIN32
    finfo.accessTime = fstat.st_atime;
    finfo.modifTime = fstat.st_mtime;
    finfo.createTime = fstat.st_ctime;
#else
    finfo.accessTime = fstat.st_atim.tv_sec;
    finfo.modifTime = fstat.st_mtim.tv_sec;
    finfo.createTime = fstat.st_ctim.tv_sec;
#endif // WINDOWS
    return true;
}


int64_t GetFileSize(const std::string& filename)
{
#ifdef _WIN32
    struct _stat64 stat_buf;
    int rc = stat64(filename.c_str(), &stat_buf);
#else
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
#endif
    return rc == 0 ? stat_buf.st_size : -1;
}

int fastIntToStr( char* pbuf, int val ) {
    pbuf[0] = '0';
    if ( val == 0 )
        return 1;
    char buf[32];
    int i = 0;
    for ( ; val != 0; ++i ) {
        buf[i] = '0' + val % 10;
        val /= 10;
    }
    for ( int j=0; j < i; ++j ) {
        pbuf[j] = buf[i-j-1];
    }
    return i;
}

// trim from end of string (right)
std::string& rtrim(std::string& s, const char* t)
{
    s.erase(s.find_last_not_of(t) + 1);
    return s;
}

// trim from beginning of string (left)
std::string& ltrim(std::string& s, const char* t)
{
    s.erase(0, s.find_first_not_of(t));
    return s;
}

// trim from both ends of string (right then left)
std::string& trim(std::string& s, const char* t)
{
    return ltrim(rtrim(s, t), t);
}


int KSStrToInt( const std::string& str ) {
    int res = 0;
    if ( str.starts_with("0x") ) {
        res = std::stoi( str.substr(2), nullptr, 16 );
    }
    else {
        res = str.length()? std::stoi( str ) : 0;
    }
    return res;
}

int KSStrToInt( const std::string_view& str, int defVal ) {
    int res = 0;
    std::from_chars_result result;
    if ( str.starts_with("0x") ) {
        result = std::from_chars( str.substr(2).data(), str.data() + str.size(), res, 16 );
    }
    else {
        result = std::from_chars( str.data(), str.data() + str.size(), res );
    }
    return ( result.ec == std::errc::invalid_argument )? defVal : res;
}


int KSStrToInt( const std::string& str, int defVal ) {
    int res = KSStrToInt(str);
    return (res == 0 )? defVal : res;
}

std::string utf8(const std::wstring& wstr) {
    // Create a converter object
    // std::codecvt_utf8_utf16<wchar_t> handles conversion between UTF-16 (often used by wstring) and UTF-8
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    // Perform the conversion
    return converter.to_bytes(wstr);
}

std::wstring utf8( const std::string& s ) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> u8conv;
    //std::wstring_convert<std::codecvt_utf8<wchar_t>> u8conv;
    std::wstring ws = u8conv.from_bytes(s.c_str());
    return ws;
}

std::wstring utf8( const char* s ) {
    return utf8( (std::string)s );
}

std::wstring fromBytes( const std::string& s ) {
    std::wstring res;
    try {
        res = utf8( s );
    }
    catch ( const std::runtime_error& err ) {
        res = utf8( cp1251_to_utf8( s ) );
    }

    return res;
}

size_t utf8charSizeAtPos( const char* str ) {
    unsigned char c = *str;
    size_t char_length = 0;
    if ((c & 0x80) == 0) {
        char_length += 1;
    } else if ((c & 0xE0) == 0xC0) {
        char_length += 2;
    } else if ((c & 0xF0) == 0xE0) {
        char_length += 3;
    } else if ((c & 0xF8) == 0xF0) {
        char_length += 4;
    } else {
        char_length += 1;
    }
    return char_length;
}

size_t utf8charSizeAtPos( const std::string& str, size_t pos ) {
    unsigned char c = str[pos];
    size_t char_length = 0;
    if ((c & 0x80) == 0) {
        char_length += 1;
    } else if ((c & 0xE0) == 0xC0) {
        char_length += 2;
    } else if ((c & 0xF0) == 0xE0) {
        char_length += 3;
    } else if ((c & 0xF8) == 0xF0) {
        char_length += 4;
    } else {
        char_length += 1;
    }
    return char_length;
}

std::string strExpandTabs( const std::string& str, int tabSize ) {
    std::string res;
    res.reserve( str.size() + 6 * tabSize );
    for ( size_t iii=0; iii < str.length(); ++iii ) {
        if ( str[iii] == '\t' )
            res += std::string( tabSize, ' ' );
        else if ( str[iii] != '\r' )
            res += str[iii];
    }
    return res;
}

size_t strVisibleSize( const std::string& str, int tabSize ) {

    size_t strPos = 0;
    size_t colPos = 0;
    for ( ; strPos < str.length(); ) {
        char chr = str[ strPos ];
        strPos += utf8charSizeAtPos( str, strPos );
        colPos += ( chr == 9 )? tabSize : 1;
        if ( chr == '\r' )
            colPos -= 1;
    }
    return colPos;
}

size_t subStrSize( const std::string& str, size_t visibleChars, size_t tabSize ) {
    size_t strPos = 0;
    for ( ; (strPos < str.length()) && ( visibleChars > 0 ); ) {
        char chr = str[strPos];
        strPos += utf8charSizeAtPos( str, strPos );
        int add = 1;
        switch( chr ) {
            case 9:
                add = tabSize;
            break;
            case '\r':
                add = 0;
            break;
            default:
            break;
        }
        visibleChars -= add;

    }
    return strPos;
}

size_t utf8_position(const std::string& utf8_string, size_t visible_chars) {
    size_t char_count = 0;
    size_t byte_pos = 0;
    while (char_count < visible_chars && byte_pos < utf8_string.length()) {
        // Decode one UTF-8 character
        size_t char_length = utf8charSizeAtPos( utf8_string, byte_pos );
        // Check if the full character is within the string
        if (byte_pos + char_length > utf8_string.length()) {
            return std::string::npos;
        }
        // Increment the character count and byte position
        char_count++;
        byte_pos += char_length;
    }
    if (char_count == visible_chars) {
        return byte_pos;
    } else {
        return std::string::npos;
    }
}


int cp1251_to_utf8(char *out, const char *in, int buflen) {
    static const int table[128] = {
        0x82D0,0x83D0,0x9A80E2,0x93D1,0x9E80E2,0xA680E2,0xA080E2,0xA180E2,
        0xAC82E2,0xB080E2,0x89D0,0xB980E2,0x8AD0,0x8CD0,0x8BD0,0x8FD0,
        0x92D1,0x9880E2,0x9980E2,0x9C80E2,0x9D80E2,0xA280E2,0x9380E2,0x9480E2,
        0,0xA284E2,0x99D1,0xBA80E2,0x9AD1,0x9CD1,0x9BD1,0x9FD1,
        0xA0C2,0x8ED0,0x9ED1,0x88D0,0xA4C2,0x90D2,0xA6C2,0xA7C2,
        0x81D0,0xA9C2,0x84D0,0xABC2,0xACC2,0xADC2,0xAEC2,0x87D0,
        0xB0C2,0xB1C2,0x86D0,0x96D1,0x91D2,0xB5C2,0xB6C2,0xB7C2,
        0x91D1,0x9684E2,0x94D1,0xBBC2,0x98D1,0x85D0,0x95D1,0x97D1,
        0x90D0,0x91D0,0x92D0,0x93D0,0x94D0,0x95D0,0x96D0,0x97D0,
        0x98D0,0x99D0,0x9AD0,0x9BD0,0x9CD0,0x9DD0,0x9ED0,0x9FD0,
        0xA0D0,0xA1D0,0xA2D0,0xA3D0,0xA4D0,0xA5D0,0xA6D0,0xA7D0,
        0xA8D0,0xA9D0,0xAAD0,0xABD0,0xACD0,0xADD0,0xAED0,0xAFD0,
        0xB0D0,0xB1D0,0xB2D0,0xB3D0,0xB4D0,0xB5D0,0xB6D0,0xB7D0,
        0xB8D0,0xB9D0,0xBAD0,0xBBD0,0xBCD0,0xBDD0,0xBED0,0xBFD0,
        0x80D1,0x81D1,0x82D1,0x83D1,0x84D1,0x85D1,0x86D1,0x87D1,
        0x88D1,0x89D1,0x8AD1,0x8BD1,0x8CD1,0x8DD1,0x8ED1,0x8FD1
    };

    char* pout = out;
    for ( ; *in && (( out - pout ) < buflen-1); ) {
        if (*in & 0x80) {
            int v = table[(int)(0x7f & *in++)];
            if (!v)
                continue;
            *out++ = (char)v;
            *out++ = (char)(v >> 8);
            if (v >>= 16)
                *out++ = (char)v;
        }
        else {
            *out++ = *in++;
        }
    }
    *out = 0;
    return (out - pout);
}

void cp1251_to_utf8( const std::string& s, std::string& out ) {

    out.resize( s.length() * 2 );

    int sz = cp1251_to_utf8( out.data(), s.c_str(), out.length() );

    out.resize( sz );
}

std::string cp1251_to_utf8( const std::string& s ) {
    std::string out;
    cp1251_to_utf8( s, out );
    return out;
}

bool KSFileExists(const std::string& filePath) {
    struct stat buffer;
    return (stat (filePath.c_str(), &buffer) == 0);
};


int KSSplit( const std::string_view& sss, const std::string& delimiter, KSSplitItem* pdata, int maxDataSize ) {
    size_t pos_start = 0, pos_end = 0, delim_len = delimiter.length();

    if ( sss.length()==0 )
        return 0;

    int iii = 0;
    for (; (pos_end < sss.length()) && (iii < (int)(maxDataSize/sizeof(KSSplitItem))); ++iii ) {
        pos_end = sss.find(delimiter, pos_start);
        if ( pos_end == std::string::npos ) {
            pos_end = sss.length();
        }
        pdata[iii].pos = pos_start;
        pdata[iii].len = pos_end - pos_start;
        pos_start = pos_end + delim_len;
    }
    return iii;
}


std::string keyval( const std::string& str, const std::string& delimiter, std::string& val ) {
    size_t pos = str.rfind( delimiter );
    if ( pos != std::string::npos ) {
        val = str.substr(  pos + delimiter.length() );
        return str.substr( 0, pos );
    }
    return str;
}


bool strMatchAny( const std::string& str, const std::vector<std::string>& someStrings ) {
    bool res = someStrings.end() != std::find( someStrings.begin(), someStrings.end(), str );
    return res;
}

void removeLastPathComponent( std::string& str, const std::string& delimiter ) {
    size_t pos = str.rfind( delimiter );
    if ( pos == std::string::npos ) {
        str = "";
        return;
    }
    str = str.substr( 0, pos - 1 );
}

std::string removeOuterCharacters(const std::string& s, int count ) {
    std::string res;
    if ( (int)s.length() <= count * 2 )
        return res;
    res = s.substr(count, s.length() - 2*count );
    return res;
}

std::string getDBasePath(const std::string& KLoggerCfgPath ) {
    std::string res;

    std::string projPath = KSExtractFilePath( KSExtractFilePath(KSExtractFilePath(KLoggerCfgPath)) );

    res = strfmt( "%s/Base", projPath.c_str() );
    return res;
}


uint32_t DWCalcCrc(const void* in, uint32_t length ) {

    byte* buffer = (byte*)in;

    //uint32_t res = crc32<IEEE8023_CRC32_POLYNOMIAL>(0xFFFFFFFF, buffer, buffer + length );
    uint32_t res = 0;
    for ( uint32_t i=0; i < length; ++i ) {
        res ^= buffer[i];
        res <<= 4;
    }
    return res;
}

uint64_t QWCalcCrc(const void* in, uint32_t length ) {

    byte* buffer = (byte*)in;
    uint64_t res = 0;
    for ( uint32_t i=0; i < length; ++i ) {
        res ^= buffer[i];
        res <<= 4;
    }
    return res;
}


void ksMemCpy(void* dst, const void* src, dword len, int& pointer) {
    byte* pbuf = (byte*)dst;
    memcpy(pbuf + pointer, src, len);
    pointer += len;
}

std::string ksDoubleToStr( double d ) {
    //std::string sd = std::format( "{:.5g}", d );
    std::string res = std::isfinite( d )? std::format( "{:.5g}", d ) : "0";
    return res;
}

std::string dbParamIdPretty( TKSRtpParamId parId ) {
    int rttId = parId / 65536;
    int IecId = parId % 65536;
    int stationId = rttId >> 8;
    rttId %= 256;
    std::string res = strfmt( "%03d.%03d.%05d", stationId, rttId, IecId );
    return res;
}

std::string urlDecode(const std::string& src) {
    std::string ret;
    char ch;
    int i, ii;
    for (i=0; i < (int)src.length(); i++) {
        if (src[i]=='%') {
            sscanf(src.substr(i+1,2).c_str(), "%x", &ii);
            ch=static_cast<char>(ii);
            ret+=ch;
            i=i+2;
        } else {
            ret+=src[i];
        }
    }
    return (ret);
}

void KSStringReplaceChar( std::string& str, int chr, int newchr ) {
    for ( size_t iii=0; iii < str.length(); ++iii ) {
        if ( str[iii] == chr )
            str[iii] = newchr;
    }
}

std::string strGetLeft( const std::string& str, const char* delimiter ) {
    size_t pos = str.find( delimiter );
    if ( pos == std::string::npos )
        pos = str.length();
    std::string res = str.substr(0, pos );
    return res;
}

std::string strGetRight( const std::string& str, const std::string& delimiter, bool returnSource ) {
    size_t pos = str.rfind( delimiter );
    if ( pos != std::string::npos )
        return str.substr( pos + delimiter.length() );
    return returnSource? str : "";
}



std::string getProcName( int pid, std::string& procName ) {
    std::string cmdPath = std::format( "/proc/{}/cmdline", pid );
    std::string cmdLine, path;
    FILE* fff = fopen( cmdPath.c_str(), "rb" );
    if ( fff == nullptr )
        return cmdLine;
    char buf[1024];
    size_t sze = fread( buf, 1, sizeof(buf), fff );
    fclose( fff );
    if ( sze <= 0 )
        return cmdLine;
    if ( buf[sze-1] == 0 )
        sze--;
    //std::ifstream cmdFile( cmdPath.c_str() );

    cmdLine.resize( sze );
    memcpy( cmdLine.data(), buf, sze );

    KSStringReplaceChar( cmdLine, 0, ' ' );

    //getline( cmdFile, cmdLine );
    if (!cmdLine.empty())
    {
        // Keep first cmdline item which contains the program path
        std::string path = strGetLeft( cmdLine, " " );
        procName = strGetRight(path, "/", true );
    }
    return cmdLine;
}

int KSGetpid() {
    return getpid();
}

int getProcPPid( int pid ) {
    std::string stat = std::format( "/proc/{}/stat", pid );
    char statbuf[1024];
    FILE* fstat = fopen( stat.c_str(), "r" );
    if ( !fstat )
        return -1;
    int res = fread( statbuf, 1, 64, fstat );
    fclose( fstat );
    if ( res == 64 ) {
        std::vector<std::string> v = split<char>( statbuf, " " );
        if ( v.size() > 2 ) {
            std::string& sppid = v[3];
            int ppid = std::stoi(sppid);
            return ppid;
        }
    }
    return -1;
}

void getProcIdsByName( const std::string& aprocName, std::vector<KSProcEntry>& out )
{
    // Open the /proc directory
    DIR *dp = opendir("/proc");
    if (dp != NULL)
    {
        // Enumerate all entries in directory until process found
        struct dirent* dirp;
        while ( (dirp = readdir(dp)) )
        {
            // Skip non-numeric entries
            int aid = atoi( dirp->d_name );
            if ( aid > 0 ) {
                // Read contents of virtual /proc/{pid}/cmdline file
                // Compare against requested process name
                std::string procName, pprocName;
                std::string cmdLine = getProcName( aid, procName );
                if ( cmdLine.length() ) {
                    if ( procName.starts_with( aprocName ) ) {
                        int ppid = getProcPPid( aid );
                        if ( ppid != -1 ) {
                            if ( getProcName(ppid, pprocName).length() ) {
                                if ( pprocName != procName ) {
                                    //std::string binLink = std::format( "/proc/{}/exe", aid );
                                    //char buf[1024];
                                    //int res = readlink( binLink.c_str(), buf, sizeof(buf) );
                                    //if ( res > 0 && res < (int)sizeof( buf ) )
                                    //    buf[res] = 0;

                                    KSProcEntry& entry = out.emplace_back();
                                    entry.pid = aid;
                                    //entry.path = ( res > 0 )? buf : cmdLine;
                                    entry.path = cmdLine;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    closedir(dp);
}



bool KSMakeDirs(const std::string& path ) {
    return std::filesystem::create_directories( path );
}

void KSMakeDirsThrow(const std::string& path ) {
    bool res = KSMakeDirs( path );
    if ( !res )
        throw std::runtime_error( std::format("{} Could not create dir {}", __func__, path ) );
}



int KSMakeDir(const std::string& path ) {
#ifdef _WIN32
    int res = mkdir( path.c_str() ); // , S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
#else
    int res = mkdir( path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
#endif
    return res;
}

void strToLower( std::string& str ) {
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {
        return std::tolower(c);
    });
}

std::string getFileExt( const std::string& fpath ) {
    size_t dotpos = fpath.rfind(".");
    if ( dotpos == std::string::npos )
        return "";
    std::string res = fpath.substr(dotpos + 1);

    strToLower( res );

    return res;
}

int KSRandom( int limit ) {

    std::srand( std::time( {} ) );
    int rres = std::rand() % limit;

    return rres;
}

std::string join( const std::vector<std::string>& lines, const std::string& delimiter ) {
    std::string res;
    for ( size_t i=0; i < lines.size(); ++i ) {
        if ( i > 0 )
            res += delimiter;
        res += lines[i];
    }
    return res;
}

std::string replace_all(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        // Advance start_pos by the length of the 'to' string to avoid
        // infinite loops if 'to' contains 'from' (e.g., replacing 'x' with 'yx')
        start_pos += to.length();
    }
    return str;
}

bool KSFileAppend( const std::string& fpath, const std::string& str ) {
    FILE* fff = fopen( fpath.c_str(), "a" );
    if ( !fff )
        return false;

    size_t wres = fwrite( str.data(), 1, str.length(), fff );
    fclose( fff );

    bool bres = ( wres == str.length() );
    return bres;
}
