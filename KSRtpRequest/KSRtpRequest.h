#ifndef KSRTPREQUEST_H
#define KSRTPREQUEST_H

#include <string>
#include "../KSuperBasic.h"

#include "../KBasicStructs.h"
#include "../KSConnectionCB.h"

#include "../../CommonCpp/KLTcpConnection/KLTCPConnection.h"

#include <format>


struct KSRtpRequestCBComm {
    std::string ipaddr = "10.0.0.134";
    std::string dbGuid;
    word ipport = 0x6543;
    int     secondsPerSample = 0;
    bool    use64bit = false;
    bool    useUtc = true;
};

struct KSRtpRequestCB : public KSRtpReadSamplesReqStruct {

    KSRtpRequestCBComm comm;

    dword   totalSamples;
    int     eof = 0;
    int     eob = 0;
    int     status = 0;
    dword   samplesInBlock;
    int     rttId = 1;
    int     dbParamId;
    std::string error;
    std::string json;

    std::string getSignalId() {
        std::string res = std::format( "{}#{}", comm.dbGuid, dbParamId );
        return res;
    }

    byte  getSampleValueSize() {  return comm.use64bit? sizeof(KSRtpParamValue) : sizeof(KSRtpParamValue32); }
};

//
//

class KSRtpRequest: public KSConnectionCB
{
    KLTCPConnection* m_tcpconn = nullptr;
public:
    KSRtpRequestCB  m_reqcb;

    virtual void processIncoming(const unsigned char* buf, int len) override;

    int startRequest(int dbParamId);
    int init( KLTCPConnection* tcpconn = nullptr );
    void run( const bool& cancel );
    KSRtpRequest();
};




#endif


