//
// Author: Dmitry Melnik
//

#include "CLTCPConnection.h"

//#include "KSTargetProto.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <netdb.h>

#include <fcntl.h>
#include <poll.h>
#include <time.h>
#include <unistd.h>

#include "../../CommonCpp/KSEventLog/KSEventLog.h"
#include <format>

#include "../../CommonCpp/KSDateTime/KSDateTime.h"
#include "../../CommonCpp/KSUtils/KSUtil.h"

#include <cstring>

#define LOGSRC 0


class CLTCPConnectionLinuxImpl : public CLTCPConnection
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
    virtual int connect(int timeout=5000) override;
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
    virtual int readData( byte* recbuf, size_t recBufSize ) override;

    CLTCPConnectionLinuxImpl();
    virtual ~CLTCPConnectionLinuxImpl() override;
};

CLTCPConnectionLinuxImpl::CLTCPConnectionLinuxImpl() {
    m_recvBuf = new byte[recvBufSize];
    m_connected = false;
    m_whenWasConnected = 0;
    dataWasReceived = false;
}

CLTCPConnectionLinuxImpl::~CLTCPConnectionLinuxImpl() {
    disconnect();
    delete [] m_recvBuf;
}

CLTCPConnection* CLTCPConnection::create(int impl) {
    CLTCPConnectionLinuxImpl* res = new CLTCPConnectionLinuxImpl();
    return res;
}

void CLTCPConnectionLinuxImpl::init(const std::string& ipaddr, short port) {
    m_ipaddr = ipaddr;
    m_ipport = port;
}


int connect_with_timeout(int sockfd, const struct sockaddr* addr, socklen_t addrlen, unsigned int timeout_ms) {
    int rc = 0;
    // Set O_NONBLOCK
    int sockfd_flags_before;
    if((sockfd_flags_before=fcntl(sockfd,F_GETFL,0)<0)) return -1;
    if(fcntl(sockfd,F_SETFL,sockfd_flags_before | O_NONBLOCK)<0) return -1;
    // Start connecting (asynchronously)
    do {
        if (::connect(sockfd, addr, addrlen)<0) {
            // Did connect return an error? If so, we'll fail.
            if ((errno != EWOULDBLOCK) && (errno != EINPROGRESS)) {
                rc = -1;
            }
            // Otherwise, we'll wait for it to complete.
            else {
                // Set a deadline timestamp 'timeout' ms from now (needed b/c poll can be interrupted)
                struct timespec now;
                if(clock_gettime(CLOCK_MONOTONIC, &now)<0) { rc=-1; break; }
                struct timespec deadline = { .tv_sec = now.tv_sec,
                                             .tv_nsec = now.tv_nsec + timeout_ms*1000000l};
                // Wait for the connection to complete.
                do {
                    // Calculate how long until the deadline
                    if(clock_gettime(CLOCK_MONOTONIC, &now)<0) { rc=-1; break; }
                    int ms_until_deadline = (int)(  (deadline.tv_sec  - now.tv_sec)*1000l
                                                  + (deadline.tv_nsec - now.tv_nsec)/1000000l);
                    if(ms_until_deadline<0) { rc=0; break; }
                    // Wait for connect to complete (or for the timeout deadline)
                    struct pollfd pfds[] = { { .fd = sockfd, .events = POLLOUT } };
                    rc = poll(pfds, 1, ms_until_deadline);
                    // If poll 'succeeded', make sure it *really* succeeded
                    if(rc>0) {
                        int error = 0; socklen_t len = sizeof(error);
                        int retval = getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len);
                        if(retval==0) errno = error;
                        if(error!=0) rc=-1;
                    }
                }
                // If poll was interrupted, try again.
                while(rc==-1 && errno==EINTR);
                // Did poll timeout? If so, fail.
                if(rc==0) {
                    errno = ETIMEDOUT;
                    rc=-1;
                }
            }
        }
    } while(0);
    // Restore original O_NONBLOCK state
    if(fcntl(sockfd,F_SETFL,sockfd_flags_before)<0) return -1;
    // Success
    return rc;
}

int CLTCPConnectionLinuxImpl::waitForIncomingData(int timeout) {

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

void CLTCPConnectionLinuxImpl::resetRecieve() {
    m_copiedBufPointer = m_recvBufPointer = 0;
}


int CLTCPConnectionLinuxImpl::receiveSync(byte* buf, int sz, int timeoutperbyte ) {

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

int CLTCPConnectionLinuxImpl::connectWithSocket(int socket) {
    disconnect();
    m_sock = socket;
    m_connected = true;
    Log().L( DL1, LOGSRC, std::format( "{}::{} finished. Socket {}", KSMETHOD, socket ) );
    return 1;
}


int CLTCPConnectionLinuxImpl::connect(int timeout) {
    if ( m_connected )
        return 0;
    struct sockaddr_in addr;

    m_sock = socket( AF_INET, SOCK_STREAM, 0 );
    if ( m_sock < 0 ) {
        return -1;
    }

    struct addrinfo hints;
    struct addrinfo* result;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       // Request IPv4 addresses
    hints.ai_socktype = SOCK_STREAM; // Request stream socket (TCP)
    hints.ai_protocol = IPPROTO_TCP;

    std::string sipport = std::format("{}", m_ipport);

    int status = getaddrinfo(m_ipaddr.c_str(), sipport.c_str(), &hints, &result);

    std::string dest = std::format( "{}:{}", m_ipaddr, m_ipport );

    if (status != 0) {
        Log().L( DLE, LOGSRC, std::format( "{}::{} Direct connection to {} {} {}", KSMETHOD, dest, m_targetId, gai_strerror(status) ) );
        return 1;
    }

    memcpy(&addr, result->ai_addr, sizeof(struct sockaddr_in));

    //addr.sin_family = AF_INET;
    //addr.sin_port = htons(m_ipport);
    //addr.sin_addr.s_addr = inet_addr( m_ipaddr.c_str() );

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

void CLTCPConnectionLinuxImpl::disconnect() {
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

int CLTCPConnectionLinuxImpl::checkInpData() {
    byte buf[1];
    int res = recv(m_sock, buf, sizeof(buf), MSG_PEEK | MSG_DONTWAIT );
    return res;
}


int CLTCPConnectionLinuxImpl::readData( byte* recbuf, size_t recBufSize ) {
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

    return recvres;
}


int CLTCPConnectionLinuxImpl::processIncoming( int startParam ) {
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
        //if ( m_proto )
        //    ((KSTargetProto*)m_proto)->processIncoming( recbuf, recvres, startParam );
    }
    return recvres;
}

int CLTCPConnectionLinuxImpl::send( byte* buf, int reqlen, bool dontcallback ) {

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







