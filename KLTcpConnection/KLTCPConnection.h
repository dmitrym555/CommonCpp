#ifndef KLTCPCONNECTION_H
#define KLTCPCONNECTION_H

#include "KLTargetConnection.h"

#include <string>


class KLTCPConnection: public KLTargetConnection
{

public:
    static KLTCPConnection* create(int impl);

    virtual void init(const std::string& ipaddr, unsigned short port)=0;

    virtual int connect(int timeout, KSUDPTCP type )=0;
    virtual void disconnect()=0;
    virtual int send(uint8_t* buf, int reqlen, bool dontcallback=false)=0;

    virtual int receiveSync(uint8_t* buf, int sz, int timeout=400 )=0;
    virtual void resetRecieve()=0;
    virtual int checkInpData()=0;

};

#endif // KLTCPCONNECTION_H
