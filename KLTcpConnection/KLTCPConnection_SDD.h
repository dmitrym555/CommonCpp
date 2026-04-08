#ifndef KLTCPCONNECTIONSDD_H
#define KLTCPCONNECTIONSDD_H

#include "KLTargetConnection.h"

#include "../../CommonCpp/KLBasicStructs.h"

#include <string>

class KLTCPConnection: public KLTargetConnection
{

public:
    static KLTCPConnection* create(int impl);

    virtual void init(const std::string& ipaddr, short port)=0;

    virtual int connect(int timeout, KSUDPTCP type )=0;
    virtual int connectWithSocket(int socket)=0;
    virtual void disconnect()=0;
    virtual int send(byte* buf, int reqlen, bool dontcallback=false)=0;
    virtual int processIncoming(int startParam)=0;

    virtual int waitForIncomingData(int timeout)=0;

};

#endif // KLTCPCONNECTION_H

