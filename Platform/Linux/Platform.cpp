
#include <string>
#include <filesystem>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sys/types.h>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <cstdlib>

#include "../Platform.h"

#include "../../KSUtils/KSUtil.h"

#include <cstring>

std::string KSExtractFilePath(const std::string& filePath) {
    std::filesystem::path p(filePath);
    return p.parent_path().string();
};


const char* psep() {
    return "/";
}


std::string CLpathDelimiter( psep() );


int sockStart() {
    return 0;
}

int KSGetThreadId() {
    return gettid();
}


std::string execShellCmd( const std::string& cmd, int timeout ) {
    system( cmd.c_str() );

    std::string result = "";

    return result;
}

int CLMakeSocket( CLSocketType stype ) {

    int sock = socket(AF_INET, SOCK_DGRAM, 0 );
    return sock;
}


int CLCreatePath( const std::string& dirPath ) {

    try {
        if ( std::filesystem::exists( dirPath ) )
            return 0;

        std::filesystem::create_directories( dirPath );
        return 1;
    } catch (const std::filesystem::filesystem_error& e) {
    }
    return -1;
}

void CLCloseSocket( int socket ) {
    close(socket);
}

int CLFdWait( int fd, int timeout ) {

    fd_set readset;
    FD_ZERO(&readset);
    FD_SET(fd, &readset);
    timeval time_wait;

    time_wait.tv_sec = timeout/1000;
    time_wait.tv_usec = (timeout%1000)*1000;

    int res = select( fd+1, &readset, NULL, NULL, &time_wait);

    return res;
}


std::string sockaddr_to_string(const struct sockaddr* sa, bool port ) {

    char buffer[64];
    const size_t buflen = sizeof( buffer );

    if (sa->sa_family == AF_INET) {
        struct sockaddr_in *ipv4 = (struct sockaddr_in *)sa;
        char ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(ipv4->sin_addr), ip_str, INET_ADDRSTRLEN);
        if ( port )
            snprintf(buffer, buflen, "%s:%d", ip_str, ntohs(ipv4->sin_port));
        else
            snprintf(buffer, buflen, "%s", ip_str );
    } else if (sa->sa_family == AF_INET6) {
        struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)sa;
        char ip_str[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET6, &(ipv6->sin6_addr), ip_str, INET6_ADDRSTRLEN);
        if ( port )
            snprintf(buffer, buflen, "[%s]:%d", ip_str, ntohs(ipv6->sin6_port));
        else
            snprintf(buffer, buflen, "[%s]", ip_str );
    } else {
        snprintf(buffer, buflen, "Unknown address family (%d)", sa->sa_family);
    }
    std::string res( buffer );
    return res;
}



unsigned long long lastTotalUser, lastTotalUserLow, lastTotalSys, lastTotalIdle;

CLProcStat::CLProcStat() {
    init();
}

void CLProcStat::init() {
    FILE* file = fopen( "/proc/stat", "r" );
    int res = fscanf(file, "cpu %llu %llu %llu %llu", &lastTotalUser, &lastTotalUserLow,
        &lastTotalSys, &lastTotalIdle);
    fclose(file);
    if ( res ) {
        res = 0;
    }
}

double CLProcStat::getCurrentCPULoad() {
    double percent;
    FILE* file;
    unsigned long long totalUser, totalUserLow, totalSys, totalIdle, total;

    file = fopen("/proc/stat", "r");
    int res = fscanf(file, "cpu %llu %llu %llu %llu", &totalUser, &totalUserLow,
        &totalSys, &totalIdle);
    fclose(file);
    if ( res ) {
        res = 0;
    }

    if (totalUser < lastTotalUser || totalUserLow < lastTotalUserLow ||
        totalSys < lastTotalSys || totalIdle < lastTotalIdle){
        //Overflow detection. Just skip this value.
        percent = -1.0;
    }
    else{
        total = (totalUser - lastTotalUser) + (totalUserLow - lastTotalUserLow) +
            (totalSys - lastTotalSys);
        percent = total;
        total += (totalIdle - lastTotalIdle);
        percent /= total;
        percent *= 100;
    }

    lastTotalUser = totalUser;
    lastTotalUserLow = totalUserLow;
    lastTotalSys = totalSys;
    lastTotalIdle = totalIdle;

    return percent;
}

CLProcStat& procStat() {
    static CLProcStat inst;
    return inst;
}

std::string getBinPath() {
    std::filesystem::path exe_path = "/proc/self/exe";
    std::string res = std::filesystem::read_symlink(exe_path);

    return res;
}

std::string xclipGet() {
    FILE* pipe = popen( "xsel -o -b", "r" );

    std::string res;

    if (!pipe) {
        return res;
    }

    char buf[32768];

    size_t fres = fread( buf, 1, sizeof( buf ), pipe );
    pclose( pipe );

    res.resize( fres );
    memcpy( res.data(), buf, fres );
    return res;
}

void xclipCopy( const std::string& str ) {
    FILE* pipe = popen( "xsel -b", "w" );

    if (!pipe) {
        //std::cerr << "popen failed!" << std::endl;
        return;
    }

    fwrite( str.c_str(), 1, str.length(), pipe );
    pclose( pipe );

}

