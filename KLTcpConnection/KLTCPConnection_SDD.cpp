//
// Data access server project
// Author: Dmitry Melnik
//

#include "KLTCPConnection_SDD.h"

#include "KLTargetConnection.h"
#include "KSTargetProto.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <poll.h>
#include <time.h>
#include <unistd.h>

#include "../../CommonCpp/KSEventLog/KSEventLog.h"
#include <format>

#include "../../CommonCpp/KSDateTime/KSDateTime.h"
#include "../../CommonCpp/KSUtils/KSUtil.h"

#include "../../CommonCpp/Platform/Platform.h"

#define LOGSRC 0


class KLTCPConnectionLinuxImpl : public KLTCPConnection
{
    std::string m_ipaddr;
    short m_ipport;
    int m_sock;
    bool m_connected;
    bool dataWasReceived;

    const int recvBufSize = 65536;
    byte* m_recvBuf = nullptr;
    int   m_recvBufPointer = 0;
    int   m_copiedBufPointer = 0;

public:
    void init(const std::string& ipaddr, short port) override;
    virtual int connect(int timeout, KSUDPTCP type) override;
    virtual int connectWithSocket(int socket) override;
    virtual void disconnect() override;
    virtual int send(byte* buf, int reqlen, bool dontcallback=false) override;
    virtual int processIncoming(int paramStart) override;

    virtual bool isConnected(int* psock=nullptr) override {
        if ( psock )
            *psock = m_sock;
        return m_connected;
    };

    virtual int waitForIncomingData(int timeout=3000) override;
    virtual void resetRecieve() override;
    virtual int receiveSync(byte* buf, int sz, int timeoutperbyte ) override;
    virtual int checkInpData() override;

    KLTCPConnectionLinuxImpl();
    virtual ~KLTCPConnectionLinuxImpl() override;
};

KLTCPConnectionLinuxImpl::KLTCPConnectionLinuxImpl() {
    m_recvBuf = new byte[recvBufSize];
    m_connected = false;
    m_whenWasConnected = 0;
    dataWasReceived = false;
}

KLTCPConnectionLinuxImpl::~KLTCPConnectionLinuxImpl() {
    disconnect();
    delete [] m_recvBuf;
}

KLTCPConnection* KLTCPConnection::create(int impl) {
    KLTCPConnectionLinuxImpl* res = new KLTCPConnectionLinuxImpl();
    return res;
}

void KLTCPConnectionLinuxImpl::init(const std::string& ipaddr, short port) {
    m_ipaddr = ipaddr;
    m_ipport = port;
}

int KLTCPConnectionLinuxImpl::waitForIncomingData(int timeout) {
//    fd_set readset;
//    FD_ZERO(&readset);
//    FD_SET(m_sock, &readset);
//    timeval time_wait;
//    time_wait.tv_sec = timeout/1000;
//    time_wait.tv_usec = (timeout%1000)*1000;
//    int res = select(m_sock+1, &readset, NULL, NULL, &time_wait);

    struct pollfd pfd;
    pfd.fd = m_sock;
    pfd.events = POLLIN;
    int res = poll( &pfd, 1, timeout );

    if ( res == 1 ) { // check the data is there
        byte recvBuf[1];
        res = recv(m_sock, recvBuf, sizeof(recvBuf), MSG_DONTWAIT|MSG_PEEK|MSG_NOSIGNAL);
        if ( res <= 0 ) {
            res = -1;
        }
    }
    return res;
}

void KLTCPConnectionLinuxImpl::resetRecieve() {
    m_copiedBufPointer = m_recvBufPointer = 0;
}


int KLTCPConnectionLinuxImpl::receiveSync(byte* buf, int sz, int timeoutperbyte ) {

    if ( sz == 0 )
        return 0;

    fd_set readset;
    FD_ZERO(&readset);
    FD_SET(m_sock, &readset);
    timeval time_wait;
    int timeout = sz * timeoutperbyte/1000;
    timeout = std::min( timeout, 500 );
    time_wait.tv_sec = timeout/1000;
    time_wait.tv_usec = (timeout%1000)*1000;

    while ( m_recvBufPointer - m_copiedBufPointer < sz ) {
        int selres = select(m_sock+1, &readset, NULL, NULL, &time_wait);
        if ( selres <= 0 )
            return -1;
        int recvres = recv( m_sock, m_recvBuf + m_recvBufPointer, recvBufSize, MSG_DONTWAIT );
        if ( recvres > 0 )
            m_recvBufPointer += recvres;
        if ( m_recvBufPointer - m_copiedBufPointer >= sz )
            break;
        if ( selres == 1 && recvres == 0 ) {
            disconnect();
            return -1;
        }
    };

    memcpy( buf, m_recvBuf + m_copiedBufPointer, sz );
    m_copiedBufPointer += sz;

    return sz;
}

int KLTCPConnectionLinuxImpl::connectWithSocket(int socket) {
    disconnect();
    m_sock = socket;
    m_connected = true;
    Log().L( DL1, LOGSRC, std::format( "{}::{} finished. Socket {}", KSMETHOD, socket ) );
    return 1;
}


int KLTCPConnectionLinuxImpl::connect(int timeout, KSUDPTCP type) {
    if ( m_connected )
        return 0;
    struct sockaddr_in addr;

    m_sock = socket( AF_INET, SOCK_STREAM, 0 );
    if ( m_sock < 0 ) {
        return -1;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_ipport);
    addr.sin_addr.s_addr = inet_addr( m_ipaddr.c_str() );

    std::string dest = std::format( "{}:{}", m_ipaddr, m_ipport );
    Log().L( DL1, LOGSRC, std::format( "{}::{} Direct connection to {} {}", KSMETHOD, dest, m_targetId ) );

    //int reuse = 1;
    //if (0 > setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) ) {
    //    Log().L( DLW, LOGSRC, std::format( "setsockopt SO_REUSEADDR error on {}", dest ) );
    //}


    int res = connect_with_timeout(m_sock, (struct sockaddr*)&addr, sizeof(addr), timeout );
    //int res = ::connect(m_sock, (struct sockaddr *)&addr, sizeof(addr));
    if ( res < 0 ) {
        close(m_sock);
        Log().L( DLW, LOGSRC, std::format( "{}::{} socket connect error: {} on {} {}", KSMETHOD, res, dest, m_targetId ) );
        return -2;
    }
    else {
        m_whenWasConnected = KSGetTickCount();
        Log().L( DLI, LOGSRC, std::format( "{}::{} Direct connect SUCCESS on {}. socket {} {}", KSMETHOD, dest, m_sock, m_targetId ) );
    }

    // Non-blocking mode
    int sockfd_flags_before;
    if((sockfd_flags_before=fcntl(m_sock,F_GETFL,0)<0))
        return -1;
    if(fcntl(m_sock,F_SETFL,sockfd_flags_before | O_NONBLOCK)<0)
        return -1;

    m_connected = true;
    return 1;
}

void KLTCPConnectionLinuxImpl::disconnect() {
    if (!m_connected)
        return;
    m_connected = false;
    Log().L( DL1, LOGSRC, std::format("{}::{} socket disconnect {}:{}, socket {} {}", KSMETHOD, m_ipaddr, m_ipport, m_sock, m_targetId ) );
    //shutdown(m_sock, SHUT_RDWR);
    close( m_sock );
    dataWasReceived = false;
    m_whenWasConnected = 0;
    //((KSTargetProto*)m_proto)->disconnect();
}

int KLTCPConnectionLinuxImpl::checkInpData() {
    byte buf[1];
    int res = recv(m_sock, buf, sizeof(buf), MSG_PEEK | MSG_DONTWAIT );
    return res;
}


int KLTCPConnectionLinuxImpl::processIncoming( int startParam ) {
    const int recBufSize = 4096*16;
    byte recbuf[recBufSize];

    struct pollfd pfd;
    pfd.fd = m_sock;
    pfd.events = POLLIN;
    int waitres = poll( &pfd, 1, 1000 );
    int recvres = 0;

    if ( waitres < 0 ) {
        disconnect();
        return -1;
    }

    if ( waitres == 0 )
        return 0;

    recvres = recv(m_sock, recbuf, recBufSize, MSG_DONTWAIT | MSG_NOSIGNAL);

    if ( recvres <= 0 ) {
        int error_code;
        int error_code_size = sizeof( error_code );
        getsockopt(m_sock, SOL_SOCKET, SO_ERROR, &error_code, (socklen_t*)&error_code_size);

        Log().L( DLW, LOGSRC, std::format("{}::{} socket reading error {} on {}:{} socket {} {}", KSMETHOD, error_code, m_ipaddr, m_ipport, m_sock, m_targetId ) );
        disconnect();
        return -1;
    }

    if ( m_connected ) {
        if ( m_proto )
            ((KSTargetProto*)m_proto)->processIncoming( recbuf, recvres, startParam );
    }
    return recvres;
}

int KLTCPConnectionLinuxImpl::send( byte* buf, int reqlen, bool dontcallback ) {

    int res = ::send( m_sock, buf, reqlen, MSG_NOSIGNAL );

    if ( res != reqlen ) {
        int error_code;
        int error_code_size = sizeof( error_code );
        getsockopt(m_sock, SOL_SOCKET, SO_ERROR, &error_code, (socklen_t*)&error_code_size);
        Log().L( DLW, LOGSRC, std::format("{}::{} socket error {} on {}:{} socket {} {}", KSMETHOD, error_code, m_ipaddr, m_ipport, m_sock, m_targetId ) );
        disconnect();
        return -1;
    }
    if ( !dontcallback )
        res = processIncoming(0);
    return res;
}






