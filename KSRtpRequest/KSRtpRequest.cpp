#include "KSRtpRequest.h"

#include "../SDDAPIStructs.h"

#include "../../CommonCpp/KSEventLog/KSEventLog.h"

#include "../../CommonCpp/strfmt.h"

#include "../../CommonCpp/KSUtils/KSUtil.h"

#include "../../CommonCpp/jsonBox/JsonBox.h"

#include "KSRtpSamplesBuf.h"

KSRtpRequest::KSRtpRequest() {
    m_reqcb.cmd = (int)KSApiRequestCmd::start;
}

void KSRtpRequest::processIncoming(const unsigned char* buf, int len) {
}


int KSRtpRequest::startRequest(int dbParamId) {

    m_reqcb.eob = 0;
    if ( !m_tcpconn->isConnected() ) {
        m_tcpconn->init( m_reqcb.comm.ipaddr, m_reqcb.comm.ipport );
        if ( 0 > m_tcpconn->connect( KSUDPTCP::udp ) ) {
            Log().E("socket connect error");
            return -1;
        }
    }

    KSDASRequestHeaderWithCmd reqH;
    KSApiStringHeader reqsH;

    std::string json;

    if ( m_reqcb.cmd == (int)KSApiRequestCmd::start ) {
        json = strfmt( R"(
            {   "request":"rtpSamples"
              , "dbGuid":"%s"
              , "cmd":%d
              , "requestId":%d
              , "sequenceNumber":%d
              , "dbParamId":%d
              , "timeStart":"%lld"
              , "timeDuration":"%lld"
              , "samplesLimit":%d
              , "secondsPerSample":%d
              , "value64":%d
            }
        )",
        m_reqcb.comm.dbGuid.c_str(), m_reqcb.cmd, m_reqcb.requestId, m_reqcb.sequenceNumber, dbParamId,
        m_reqcb.timeStart, m_reqcb.timeDuration, m_reqcb.samplesLimit, m_reqcb.comm.seconsPerSample, m_reqcb.comm.use64bit? 1:0 );
    } else {
        json = strfmt( R"({ "request":"rtpSamples", "cmd":%d, "requestId":%d, "sequenceNumber":%d, "samplesLimit":%d })",
        m_reqcb.cmd, m_reqcb.requestId, m_reqcb.sequenceNumber, m_reqcb.samplesLimit );
    }

    Log().D( 7, strfmt( "json request: %s", json.c_str() ) );
    reqsH.strLen = json.length();
    reqsH.strCrc = DWCalcCrc( json.c_str(), json.length() );

    byte request[1024];
    int p = 0;
    reqH.query = QUERY_JSON;
    ksMemCpy( request, &reqH, sizeof(reqH), p );
    ksMemCpy( request, &reqsH, sizeof(reqsH), p );
    ksMemCpy( request, json.c_str(), json.length() + 1, p );

    //tcpconn->setIncomingsProcessor( this );
    if ( p == m_tcpconn->send( request, p, true ) ) {
        //Log().D1( std::format( "{} bytes sent", p ) );
    }
    else {
        Log().W( "socket send error" );
    }

    return 0;
}

int KSRtpRequest::init( KLTCPConnection* tcpconn ) {
    if (tcpconn) {
        m_tcpconn = tcpconn;
    }
    else {
        m_tcpconn = KLTCPConnection::create(0);
    }
    return 0;
}

void KSRtpRequest::run(const bool& cancel ) {
    byte recvBuff[1024*64];

    m_reqcb.cmd = (int)KSApiRequestCmd::start;
    m_reqcb.error = "";
    m_reqcb.sequenceNumber = 0;
    Log().D1("KSRtpRequest::run");

    int i=0;
    int retryCount = 10;
    int recvBlockTimeoutPerByte = 680; // 15 ms per sample
//    dword successBlockTime = 0;
    const dword minBlockSize = 512;
    dword maxBlockSize = 64000;
    uint64_t updateDurationStart = KSGetTickCount();
    m_reqcb.samplesLimit = 30;
    int totalBytes = 0;
    uint64_t requestStart = KSGetTickCount();

    for ( ; i < retryCount && !cancel; ++i ) {
        m_tcpconn->resetRecieve();
        if ( i > 0 && (m_reqcb.cmd == (int)KSApiRequestCmd::next || m_reqcb.cmd == (int)KSApiRequestCmd::resend ) ) {
            m_reqcb.cmd = (int)KSApiRequestCmd::resend;
            Log().W( strfmt( "Resend request repeat: %d", i ) );
        }

        uint64_t blockStart = KSGetTickCount();
        startRequest( m_reqcb.dbParamId );
        int processPosition = 0;

        int expSize = sizeof(KSDASRequestHeaderWithCmd);
        if ( 0 >= m_tcpconn->receiveSync( recvBuff + processPosition, expSize, 7*recvBlockTimeoutPerByte ) ) {
            Log().E( "SDD header not received" );
            continue;
        }

        KSDASRequestHeaderWithCmd& reqH = *(KSDASRequestHeaderWithCmd*)(recvBuff + processPosition);

        if ( 0 != memcmp( &reqH.signature, KSDASAPISignature, sizeof(reqH.signature) ) ) {
            Log().E( "Expected signature SDDv" );
            continue;
        }
        if ( reqH.query != QUERY_JSON ) {
            Log().E( "Expected json query" );
            continue;
        }
        processPosition += expSize;

        expSize = sizeof(KSApiStringHeader);
        if ( 0 >= m_tcpconn->receiveSync( recvBuff + processPosition, expSize, recvBlockTimeoutPerByte ) ) {
            Log().E( "KSApiStringHeader" );
            continue;
        }

        KSApiStringHeader& reqsH = *(KSApiStringHeader*)(recvBuff + processPosition);

        if ( 0 != memcmp( &reqsH.signature, KSApiJsonRequestSignature, sizeof(reqsH.signature) ) ) {
            Log().E( "Expected signature KSJQ" );
            continue;
        }
        processPosition += expSize;

        expSize = reqsH.strLen + 1;
        if ( 0 >= m_tcpconn->receiveSync( recvBuff + processPosition, expSize, recvBlockTimeoutPerByte ) ) {
            Log().E( "reqsH.strLen" );
            continue;
        }

        if ( reqsH.strCrc != DWCalcCrc( recvBuff + processPosition, reqsH.strLen ) ) {
            Log().E( "Json request crc mismatch" );
            continue;
        }
        recvBuff[ processPosition + reqsH.strLen ] = 0;
        m_reqcb.json = (std::string)( (const char*)(recvBuff + processPosition) );
        m_reqcb.status = 3;
        processPosition += expSize;

        Log().D(5, strfmt("recv: %s",  m_reqcb.json.c_str() ) );

        JsonBox::Value jv;
        jv.loadFromString( m_reqcb.json );

        m_reqcb.eof = jv["eof"].getInteger();
        m_reqcb.error = jv["error"].getString();
        m_reqcb.requestId = jv["requestId"].getInteger();
        m_reqcb.totalSamples = jv["totalSamples"].getInteger();
        m_reqcb.samplesInBlock = jv["samplesInBlock"].getInteger();
        int sequenceNumber = jv["sequenceNumber"].getInteger();
        int blockBaseDay = jv["blockBaseDay"].getInteger();

        if ( sequenceNumber != (int)m_reqcb.sequenceNumber ) {
            Log().W( strfmt("Assertion failed: send and recieve sequenceNumber do not match %d %d", m_reqcb.sequenceNumber, sequenceNumber ));
        }

        std::string spayloadcrc = jv["payloadCrc"].getString();

        uint64_t payloadcrc = std::stoull( spayloadcrc );

        if ( m_reqcb.error.length() ) {
            Log().E( strfmt( "server error: %s ", m_reqcb.error.c_str() ) );
            break;
        }

        expSize = m_reqcb.samplesInBlock * m_reqcb.getSampleValueSize();
        if ( 0 > m_tcpconn->receiveSync( recvBuff + processPosition, expSize, recvBlockTimeoutPerByte ) ) {
            Log().E( "payload" );
            continue;
        }

        if ( expSize ) {
            uint64_t plCrc = QWCalcCrc( recvBuff + processPosition, expSize );
            if ( plCrc != payloadcrc ) {
                Log().E( "payload crc mismatch" );
                continue;
            }
        }

        int blockTime = KSGetTickCount() - blockStart;
        int blockSize = processPosition + expSize;
        totalBytes += blockSize;
        dword requestTime = KSGetTickCount() - requestStart;
        Log().D(5, strfmt( "totalBytes %d requestTime %d blockTime %d",  totalBytes, requestTime, blockTime ));
        dword speedAvg = totalBytes / (1+requestTime);
        dword speedBlock = blockSize / ( 1 + blockTime );

        //m_reqcb.samplesLimit = 40;
        m_reqcb.samplesLimit = std::min( maxBlockSize/m_reqcb.getSampleValueSize(), (2*speedAvg + speedBlock)*2 + minBlockSize / m_reqcb.getSampleValueSize() );
        Log().D( 6, strfmt( "speedAvg %d speedBlock %d samplesInBlock %d newSamplesLimit %d ", speedAvg, speedBlock, m_reqcb.samplesInBlock, m_reqcb.samplesLimit ) );

        KSRtpSamplesBuf& samplesBuf = trendsBuf.get( m_reqcb.dbParamId );

        for ( dword i=0; i < m_reqcb.samplesInBlock; ++i ) {
            KSRtpParamValue32& sample32 = *(KSRtpParamValue32*)( recvBuff + processPosition + i*m_reqcb.getSampleValueSize());
            KSRtpParamValue& sample64 = *(KSRtpParamValue*)( recvBuff + processPosition + i*m_reqcb.getSampleValueSize());
            KSRtpPValueFullTime dbPar;
            dbPar.meta = sample32.meta;
            if ( m_reqcb.comm.use64bit ) {
                dbPar.setValue( sample64 );
                dbPar.t2000Msec = (uint64_t)blockBaseDay * TKSMsecPerDay + sample64.timeMsec;
            }
            else {
                dbPar.setValue( sample32 );
                dbPar.t2000Msec = (uint64_t)blockBaseDay * TKSMsecPerDay + sample32.timeMsec;
            }
            KSTimeToLocal( dbPar.t2000Msec );
            samplesBuf.addSample(dbPar);
        }

        int updateDuration = KSGetTickCount() - updateDurationStart;
        if ( updateDuration > 500 ) {
            updateDurationStart = KSGetTickCount();
        }

        if ( m_reqcb.eof ) {
            Log().D1( strfmt( "recv json: %s", m_reqcb.json.c_str() ) );
            break;
        }

        if ( m_reqcb.samplesInBlock == 0 ) {
            KSsleep(500);
        }

        m_reqcb.cmd = (int)KSApiRequestCmd::next;
        i = -1;
        m_reqcb.sequenceNumber++;
        if ( m_tcpconn->checkInpData() > 0 ) {
            // Something went wrong
            Log().E("Assertion failed: there are still some data in the input buffer");
            m_tcpconn->disconnect();
        }
    }
    if ( i == retryCount ) {
        Log().E( "Server is not available" );
    }
    m_reqcb.cmd = (int)KSApiRequestCmd::close;
    startRequest( m_reqcb.dbParamId );
    Log().I("========================================================");
    Log().I("");
    if ( !cancel && m_tcpconn->checkInpData() > 0 ) {
        // Something went wrong
        Log().E("Assertion failed: there are still some data in the input buffer");
        m_tcpconn->disconnect();
    }
}
