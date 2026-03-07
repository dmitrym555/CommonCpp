#include "KLTCPConnection.h"

#include "KLTargetConnection.h"
//#include "KSTargetProto.h"

#include <sys/types.h>

#ifndef _WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#else
#endif



#include "../../CommonCpp/Platform/Platform.h"

#include <fcntl.h>
#include <time.h>
#include <cstring>

#include "../../CommonCpp/KSEventLog/KSEventLog.h"

#include "../../CommonCpp/KSDateTime/KSDateTime.h"

#include "../../CommonCpp/KSConnectionCB.h"

#include "../../CommonCpp/KSUtils/KSUtil.h"

#include <unistd.h>


class KLTCPConnectionLinuxImpl : public KLTCPConnection
{
    std::string m_ipaddr;
    short m_ipport;
    int m_sock;
    bool m_connected;
    bool dataWasReceived;
    const int recvBufSize = 1024*1024;
    byte* m_recvBuf = new byte[recvBufSize];
    int   m_recvBufPointer = 0;
    int   m_copiedBufPointer = 0;


public:
    void init(const std::string& ipaddr, word port);
    virtual int connect(KSUDPTCP type, int timeout=5000) override ;
    virtual void disconnect() override;
    virtual int send(byte* buf, int reqlen, bool dontcallback=false) override;
    virtual int processIncoming() override;
    virtual int receiveSync(byte* buf, int sz, int timeout ) override;
    virtual int checkInpData() override;
    virtual void resetRecieve() override;

    virtual bool isConnected() override { return m_connected; };

    KLTCPConnectionLinuxImpl();
    ~KLTCPConnectionLinuxImpl();
};

KLTCPConnectionLinuxImpl::KLTCPConnectionLinuxImpl() {
    m_connected = false;
    m_whenWasConnected = 0;
    dataWasReceived = false;
}

KLTCPConnectionLinuxImpl::~KLTCPConnectionLinuxImpl() {
    disconnect();
    delete m_recvBuf;
}

KLTCPConnection* KLTCPConnection::create(int impl) {
    KLTCPConnectionLinuxImpl* res = new KLTCPConnectionLinuxImpl();
    return res;
}

void KLTCPConnectionLinuxImpl::init(const std::string& ipaddr, word port) {
    m_ipaddr = ipaddr;
    m_ipport = port;
}

int KLTCPConnectionLinuxImpl::connect(KSUDPTCP type, int timeout) {
    if ( m_connected )
        return 0;
    struct sockaddr_in addr;

    if ( sockStart() != 0 ) {
        return -1;
    }

    m_sock = socket(AF_INET, (type == KSUDPTCP::udp)? SOCK_DGRAM : SOCK_STREAM, 0 );
    if ( m_sock < 0 ) {
        return -1;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_ipport);
    addr.sin_addr.s_addr = inet_addr( m_ipaddr.c_str() );

    std::string dest = strfmt( "%s:%d", m_ipaddr.c_str(), m_ipport );
    Log().D1( strfmt( "Connecting to %s", dest.c_str() ) );

    //int reuse = 1;
    //if (0 > setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) ) {
    //    Log().W( std::format( "setsockopt SO_REUSEADDR error on {}", dest ) );
    //}

    int res = connect_with_timeout(m_sock, (struct sockaddr*)&addr, sizeof(addr), timeout );
    //int res = ::connect(m_sock, (struct sockaddr *)&addr, sizeof(addr));
    if ( res < 0 ) {
        Log().W( strfmt( "socket connect error: %d on %s", res, dest.c_str() ) );
        return -2;
    }
    else {
        m_whenWasConnected = KSGetTickCount();
        Log().D1( strfmt( "connect SUCCESS on %s", dest.c_str() ) );
    }

    // Non-blocking mode

#ifndef _WIN32
    int sockfd_flags_before;
    if((sockfd_flags_before=fcntl(m_sock,F_GETFL,0)<0))
        return -1;
    if(fcntl(m_sock,F_SETFL,sockfd_flags_before | O_NONBLOCK)<0)
        return -1;
#else
    ULONG ul = 1;
    ioctlsocket( m_sock, FIONBIO, &ul); // переводим сокет в неблокирующий режим
#endif

    m_connected = true;
    return 1;
}

void KLTCPConnectionLinuxImpl::disconnect() {
    if (!m_connected)
        return;
    Log().D1( strfmt("KLTCPConnection socket disconnect %s:%d", m_ipaddr.c_str(), m_ipport) );
    //shutdown(m_sock, SHUT_RDWR);
    close(m_sock);
    m_connected = false;
    dataWasReceived = false;
    m_whenWasConnected = 0;
    //((KSTargetProto*)m_proto)->disconnect();
}

void KLTCPConnectionLinuxImpl::resetRecieve() {
    m_copiedBufPointer = m_recvBufPointer = 0;
}

int KLTCPConnectionLinuxImpl::checkInpData() {
    byte buf[1];
    int res = recv(m_sock, (char*)buf, sizeof(buf), MSG_PEEK );
    return res;
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
        int res = recv(m_sock, (char*)m_recvBuf + m_recvBufPointer, recvBufSize, 0 ); //MSG_DONTWAIT );
        if ( res > 0 )
            m_recvBufPointer += res;
        if ( m_recvBufPointer - m_copiedBufPointer >= sz )
            break;
        res = select(m_sock+1, &readset, NULL, NULL, &time_wait);
        if ( res <= 0 )
            return -1;
    };

    memcpy( buf, m_recvBuf + m_copiedBufPointer, sz );
    m_copiedBufPointer += sz;

    return sz;
}

int KLTCPConnectionLinuxImpl::processIncoming() {
    byte stackbuf[65536];
//    byte* dynbuf = nullptr;
    byte* pbuf = stackbuf;
//    int bufsize = sizeof(stackbuf);
    int res = recv(m_sock, (char*)stackbuf, sizeof(stackbuf), O_TRUNC ); //|MSG_TRUNC|MSG_PEEK );
//    std::unique_ptr<byte> sp_buf;
//    if ( res > 0 ) {
//        if ( res > (int)sizeof(stackbuf) ) {
//            pbuf = dynbuf = new byte[res];
//            sp_buf.reset(dynbuf);
//            bufsize = res;
//        }
//        res = recv(m_sock, pbuf, bufsize, MSG_DONTWAIT);
//        dataWasReceived = true;
//    }

    if ( res >= 0 ) {
        ((KSConnectionCB*)m_proto)->processIncoming( pbuf, res );
    }
    else {
        int error_code;
        int error_code_size = sizeof(error_code);
        getsockopt(m_sock, SOL_SOCKET, SO_ERROR, (char*)&error_code, (socklen_t*)&error_code_size);
        if ( error_code != 0 )
            disconnect();
    }
    return res;
}

int KLTCPConnectionLinuxImpl::send(byte* buf, int reqlen, bool dontcallback ) {

    int res = ::send(m_sock, (char*)buf, reqlen, 0); // MSG_NOSIGNAL

    if ( res != reqlen ) {
        int error_code;
        int error_code_size = sizeof(error_code);
        getsockopt(m_sock, SOL_SOCKET, SO_ERROR, (char*)&error_code, (socklen_t*)&error_code_size);
        Log().W( strfmt("rtt socket error %d on %s:%d", error_code, m_ipaddr.c_str(), m_ipport ) );
        disconnect();
        return -1;
    }
    if ( !dontcallback )
        res = processIncoming();
    return res;
}






