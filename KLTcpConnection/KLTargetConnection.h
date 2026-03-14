#ifndef KLTARGETCONNECTION_H
#define KLTARGETCONNECTION_H


#include <cstdint>
#include <string>

enum class KSUDPTCP { udp, tcp };


class KLTargetConnection
{
protected:
    void*       m_proto;
    uint64_t    m_whenWasConnected;

public:
    std::string m_targetId;

    virtual int connect(int timeout = 5000, KSUDPTCP type = KSUDPTCP::tcp )=0;
    virtual int connectWithSocket(int socket)=0;
    virtual void disconnect()=0;
    virtual int send(uint8_t* buf, int reqlen, bool dontcallback=false)=0;
    virtual int waitForIncomingData(int timeout)=0;
    virtual int processIncoming(int startParam)=0;
    virtual int receiveSync(uint8_t* buf, int sz, int timeoutperbyte ) { return 0; }
    virtual void resetRecieve() {};
    virtual int checkInpData() { return 0; }

    virtual bool isConnected(int* sock=nullptr)=0;

    virtual void setIncomingsProcessor( void* proto ) { m_proto = proto; };

    virtual ~KLTargetConnection() {}
};

#endif // KLTARGETCONNECTION_H
