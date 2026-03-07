#ifndef CLTARGETCONNECTION_H
#define CLTARGETCONNECTION_H

#include <cstdint>
#include <string>

#include "KSuperBasic.h"

class CLTargetConnection
{
protected:
    void*       m_proto;
    uint64_t    m_whenWasConnected;

public:
    std::string m_targetId;

    virtual int connect(int timeout = 5000)=0;
    virtual int connectWithSocket(int socket)=0;
    virtual void disconnect()=0;
    virtual int send(unsigned char* buf, int reqlen, bool dontcallback=false)=0;
    virtual int waitForIncomingData(int timeout)=0;
    virtual int processIncoming(int startParam)=0;
    virtual int receiveSync(unsigned char* buf, int sz, int timeoutperbyte ) { return 0; }
    virtual void resetRecieve() {};
    virtual int checkInpData() { return 0; }

    virtual int readData( byte* recbuf, size_t recBufSize )=0;

    virtual bool isConnected(int* sock=nullptr)=0;

    virtual void setIncomingsProcessor( void* proto ) { m_proto = proto; };

    virtual ~CLTargetConnection() {}
};

#endif // CLTARGETCONNECTION_H

