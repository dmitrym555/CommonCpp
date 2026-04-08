#ifndef CLTCPCONNECTION_H
#define CLTCPCONNECTION_H

#include "../CLTargetConnection.h"

#include <string>

//#include "../KSuperBasic.h"


class CLTCPConnection: public CLTargetConnection
{

public:
    static CLTCPConnection* create(int impl);

    virtual void init(const std::string& ipaddr, uint16_t port)=0;

    virtual int connect(int timeout=5000)=0;
    virtual int connectWithSocket(int socket)=0;
    virtual void disconnect()=0;
    virtual int send(byte* buf, int reqlen, bool dontcallback=false)=0;
    virtual int processIncoming(int startParam)=0;
    //virtual int readData( byte* recbuf, size_t recBufSize ) override;

    virtual int waitForIncomingData(int timeout)=0;

};

#endif // CLTCPCONNECTION_H



