#ifndef KLTARGETCONNECTION_H
#define KLTARGETCONNECTION_H

#include "../../CommonCpp/KBasicStructs.h"


enum class KSUDPTCP { udp, tcp };

class KLTargetConnection
{
protected:
    void*     m_proto;
    uint64_t  m_whenWasConnected;
public:
    virtual int connect(KSUDPTCP type, int timeout = 5000)=0;
    virtual void disconnect()=0;
    virtual int send(byte* buf, int reqlen, bool dontcallback=false)=0;
    virtual int processIncoming()=0;
    virtual int receiveSync(byte* buf, int sz, int timeoutperbyte=680 )=0;
    virtual void resetRecieve()=0;

    virtual bool isConnected()=0;

    virtual void setIncomingsProcessor( void* proto ) { m_proto = proto; };

    virtual ~KLTargetConnection() {}
};

#endif // KLTARGETCONNECTION_H
