#include "KSRequestSamplesLoop.h"

#include <thread>

#include <sys/types.h>
#include <fcntl.h>

#include "../../CommonCpp/jsonBox/JsonBox.h"
#include "../../CommonCpp/KSEventLog/KSEventLog.h"
#include "../../CommonCpp/KSUtils/KSUtil.h"

#include "KSRtpSamplesBuf.h"

#include "../../CommonCpp/KSConfParser/KSConf.h"

#include "../../CommonCpp/Platform/Platform.h"

#include "../../CommonCpp/KSRtpRequest/KSRtpRequest.h"

#include <chrono>
#include <random>


extern void redrawSurface();



KSTInterval KSRequestSamplesLoop::getTimeInterval() {
    std::lock_guard<std::mutex> lg(m_mutex);
    KSTInterval res = ti_requested;
    return res;
}


void KSRequestSamplesLoop::change( const KSTInterval& tinterval ) {
    std::unique_lock<std::mutex> lg(m_mutex);
    ti_requested = tinterval;
    Log().D1( strfmt( "KSRequestSamplesLoop::change tstart: %s tend: %s", standart_datetime( tinterval.tstart ).c_str(), standart_datetime( tinterval.tend() ).c_str() ) );
    lg.unlock();
    wake();
}

void KSRequestSamplesLoop::runTrend( uint32_t dbParamId ) {

    // do random sleep to reduce network racing condition probability
    int randomSleep = 5 * (dbParamId % 10);
    Log().D1( strfmt( "random sleep: %d", randomSleep ) );
    KSsleep( randomSleep );

    KSRtpRequest rtpReq;
    rtpReq.init( nullptr );

    rtpReq.m_reqcb.dbParamId = dbParamId;
    rtpReq.m_reqcb.comm = m_reqComm;

    m_mutex.lock();
    KSTInterval requested = ti_requested;
    m_mutex.unlock();

    KSRtpSamplesBuf& samplesBuf = trendsBuf.get( dbParamId );

    if ( samplesBuf.ti_filled.tstart == 0 ) {
        samplesBuf.ti_filled = requested;
        samplesBuf.ti_filled.tduration = 0;
    }

    if ( requested.tstart < samplesBuf.ti_filled.tstart ) {
        rtpReq.m_reqcb.timeStart = requested.tstart;
        rtpReq.m_reqcb.timeDuration = samplesBuf.ti_filled.tstart - requested.tstart;
        samplesBuf.ti_filled.tstart = requested.tstart;
        rtpReq.run( m_cancel );
    }

    if ( requested.tend() > samplesBuf.ti_filled.tend() ) {
        rtpReq.m_reqcb.timeStart = samplesBuf.ti_filled.tend();
        rtpReq.m_reqcb.timeDuration = requested.tend() - rtpReq.m_reqcb.timeStart;
        //samplesBuf.ti_filled.tduration += requested.tend() - samplesBuf.ti_filled.tend();
        rtpReq.m_reqcb.timeDuration += 60000;
        rtpReq.run( m_cancel );
    }
    --m_trendsThreadCounter;
}

void KSRequestSamplesLoop::reloadConfig() {
    std::string confPath = strfmt( "%s%scfg.conf", m_binPath.c_str(), psep() );
    g_conf.init(0, nullptr, confPath.c_str() );

    Log().setLevel( std::stoi( g_conf.get("logLevel", "5" ) ) );

    m_reqComm.ipaddr = g_conf.get( "ipaddr", "127.0.0.1" );
    m_reqComm.dbGuid = g_conf.get( "dbGuid", "" );
    m_reqComm.seconsPerSample = (g_conf.get( "useDensity", "1" ) == "1")? ti_requested.tduration / 1000 / 50000 : 0;
    m_reqComm.use64bit = g_conf.get( "use64bit", "0" ) == "1";

    Log().D1( strfmt("rtpReq.m_reqcb.seconsPerSample %d", m_reqComm.seconsPerSample ) );

    Log().I( strfmt( "ipaddr: %s dbGuid: %s", m_reqComm.ipaddr.c_str(), m_reqComm.dbGuid.c_str() ) );
}

void KSRequestSamplesLoop::run() {

    KSsleep(200);
    ti_requested.tduration = 1*60*60*1000;
    ti_requested.tstart = KSTimeNow() - ti_requested.tduration;

    while ( !m_terminated ) {

        reloadConfig();

        m_cancel = false;
        m_changed = false;
        trendsBuf.instanceOwn();
        for ( uint32_t paramId : m_paramIds ) {
            KSRtpSamplesBuf& samplesBuf = trendsBuf.get( paramId );
            if ( !samplesBuf.m_enabled )
                continue;
            ++m_trendsThreadCounter;
            std::thread thread_obj( &KSRequestSamplesLoop::runTrend, this, paramId );
            thread_obj.detach();
            //KSRequestSamplesLoop::runTrend( paramId );
        }

        while ( !m_terminated && m_trendsThreadCounter > 0 ) {
            KSsleep(2);
        }
        redrawSurface();
        trendsBuf.instanceFree();

        Log().D1( "Request finished" );
        while ( !m_changed ) {
            KSsleep(2);
        }
    }
}


void KSRequestSamplesLoop::addParamId(uint32_t dbParamId) {
    m_paramIds.push_back( dbParamId );
}


void KSRequestSamplesLoop::wake() {
    resetIds();
    int cnt = trendsBuf.getTrendsCount();

    for ( int i=0; i < cnt; ++i ) {
        addParamId( trendsBuf.getParIdByIndex(i) );
    }

    m_changed = true;
    //m_requestStartTime = KSGetTickCount() + 300;
    //cv.notify_all();
}

void KSRequestSamplesLoop::start() {
    std::thread thread_obj(&KSRequestSamplesLoop::run, this );
    thread_obj.detach();
}

void KSRequestSamplesLoop::stop() {
    m_terminated = true;
    cv.notify_all();
}

