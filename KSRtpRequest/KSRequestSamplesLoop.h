#ifndef KSREQUESTSAMPLESLOOP_H
#define KSREQUESTSAMPLESLOOP_H

#include "../../CommonCpp/SDDAPIStructs.h"

#include "../../CommonCpp/KLTcpConnection/KLTCPConnection.h"

#include "../../CommonCpp/KSConnectionCB.h"

#include "../../CommonCpp/KSRtpRequest/KSRtpRequest.h"

#include <condition_variable>

#include "../../CommonCpp/KSDateTime/KSDateTime.h"

#include <map>
#include <atomic>



class KSRequestSamplesLoop {
    bool m_terminated = false;
    uint64_t m_requestStartTime = 0;
    std::mutex m_mutex;
    bool m_cancel = false;
    bool m_changed = false;

    std::condition_variable cv;
    std::mutex m_cvmutex;

    std::vector<std::string>  m_paramIds;
    std::atomic<int> m_trendsThreadCounter = 0;

    KSRtpRequestCBComm m_reqComm;

    KSTInterval ti_requested;

    void run();

public:
    std::string m_binPath;

    void runTrend( const std::string& dbParamId );

    void reloadConfig();
    void setConfig(const std::string& dbAlias);

    void start();
    void stop();
    void change( const KSTInterval& tinterval );
    void wake();
    KSTInterval getTimeInterval(bool local = true);

    KSRtpRequestCBComm& reqCB() { return m_reqComm; }

    void resetIds() { m_paramIds.clear(); }

    void addParamId( const std::string& dbParamId);

    void resetFilled();
};



#endif // KSREQUESTSAMPLESLOOP_H
