#ifndef KLTCPCONNECTION_H
#define KLTCPCONNECTION_H

#include "KLTargetConnection.h"

#include <string>


class KLTCPConnection: public KLTargetConnection
{

public:
    static KLTCPConnection* create(int impl);

    virtual void init(const std::string& ipaddr, unsigned short port)=0;

    virtual int connect(KSUDPTCP type, int timeout = 5000)=0;
    virtual void disconnect()=0;
    virtual int send(byte* buf, int reqlen, bool dontcallback=false)=0;
    virtual int processIncoming()=0;

    virtual int receiveSync(byte* buf, int sz, int timeout=400 )=0;
    virtual void resetRecieve()=0;
    virtual int checkInpData()=0;

};

#endif // KLTCPCONNECTION_H
