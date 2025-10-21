#ifndef PLATFORMH
#define PLATFORMH

#include <string>

#include <cstdint>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#endif

#ifndef byte
typedef unsigned char byte;
#endif

#ifndef word
typedef unsigned short word;
#endif

#ifndef dword
typedef uint32_t dword;
#endif

extern std::string CLpathDelimiter;

const char* psep();

int sockStart();

int KSGetThreadId();

enum class CLSocketType { udp, tcp };

int CLMakeSocket( CLSocketType stype );
void CLCloseSocket( int socket );

int CLFdWait( int fd, int timeout );

std::string execShellCmd( const std::string& cmd, int timeout );

std::string sockaddr_to_string(const struct sockaddr* sa, bool port = false );


class CLProcStat {
public:
    void init();
    double getCurrentCPULoad();

    CLProcStat();
};

CLProcStat& procStat();

std::string getBinPath();

void xclipCopy( const std::string& str );
std::string xclipGet();

int CLCreatePath( const std::string& dirPath );

#endif
