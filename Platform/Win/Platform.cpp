
#include <windows.h>

#include <string>

#include <atomic>

#include "../Platform.h"

#pragma comment (lib,"ws2_32.lib")

std::string CLpathDelimiter = "\\";

std::string KSExtractFilePath(const std::string& filePath) {

    size_t pos = filePath.rfind( "\\" );
    std::string res;
    if ( pos == std::string::npos )
    	return res;

    res = filePath.substr(0, pos);
    return res;
};

char* psep() {
    return "\\";
}

int KSGetThreadId() {
    int tid = GetCurrentThreadId();
    return tid;
}

int sockStart() {
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    wVersionRequested = MAKEWORD(2, 2);

    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        /* Tell the user that we could not find a usable */
        /* Winsock DLL.                                  */
        printf("WSAStartup failed with error: %d\n", err);
        return -1;
    }
    return 0;
}



std::string execShellCmd( const std::string& cmd, int timeout ) {
    char buffer[4096];
    std::string result;

    return result;
}



std::atomic<bool> WsaWasStarted{ false };

int CLMakeSocket( CLSocketType stype ) {
    int wasStarted = WsaWasStarted.exchange( true );
    if ( !wasStarted ) {
        sockStart();
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0 );
    return sock;
}


void CLCloseSocket( int socket ) {
    closesocket(socket);
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



