#include "KSRtpSamplesBuf.h"
#include <cmath>

#include "../../CommonCpp/KSEventLog/KSEventLog.h"

#include <cstdint>


int getDay( uint64_t kstime ) {
    int res = kstime/(24*3600000);
    return res;
}


int KSRtpSamplesVector::searchPositionToInsert(const KSRtpPValueFullTime& sample) {
    if ( pool.size() == 0 )
        return 0;
    if ( positionToInsert < (int)pool.size() && pool[positionToInsert].t2000Msec == sample.t2000Msec ) {
        ++positionToInsert;
        return -1;
    }
    if ( pool[positionToInsert-1].t2000Msec < sample.t2000Msec ) {
        if ( positionToInsert == (int)pool.size() )
            return positionToInsert;
        if ( pool[positionToInsert].t2000Msec > sample.t2000Msec )
            return positionToInsert;
    }
    // binary search position to insert
    int A = -1; int B = pool.size();
    int C = A;
    while ( B - A > 1 ) {
        C = (A + B)/2;
        if ( pool[C].t2000Msec == sample.t2000Msec ) {
            positionToInsert = C+1;
            return -1;
        }
        else if ( pool[C].t2000Msec > sample.t2000Msec )
            B = C;
        else
            A = C;
    };
    return A+1;
}

void KSRtpSamplesVector::addSample(const KSRtpPValueFullTime& sample) {
    int newPos = searchPositionToInsert(sample);
    //Log().D1("newPos %d", newPos );
    if ( newPos == -1 )
        return;
    if (newPos >= (int)pool.size()) {
        pool.push_back(sample);
        positionToInsert = (int)pool.size();
        return;
    }

    pool.insert( pool.begin() + newPos, sample );
    positionToInsert = newPos + 1;
}

void KSRtpSamplesBuf::resetMinMax() {
    minY = std::numeric_limits<double>::max();
    maxY = -std::numeric_limits<double>::max();
}

//
//
//
void KSRtpSamplesBuf::addSample(KSRtpPValueFullTime& sample) {

    if ( m_markedToReset ) {
        reset();
        m_markedToReset = false;
    }

    if ( std::fabs( sample.t2000Msec - m_lastInsertedSampleTime) < m_minTimeSpan ) {
        ++m_skippedCount;

        minY = std::min( minY, sample.getDVal() );
        maxY = std::max( maxY, sample.getDVal() );
        return;
    }
    m_lastInsertedSampleTime = sample.t2000Msec;

    int day = getDay( sample.t2000Msec );

    //Log().D1( "sample ts: %s %.4f", KSDayTime( sample.t2000Msec ).c_str(), sample.Value.dval );
    std::lock_guard<std::mutex> lg( m_mutex );
    if ( m_dayToAdd != day  || !m_poolToAdd ) {
        m_dayToAdd = day;
        m_poolToAdd = &m_buf[day];
    }

    KSRtpSamplesVector& sampleVector = *m_poolToAdd;
    if ( m_skippedCount > 1 ) {
        bool minormax = sampleVector.pool.size() % 2;
        sample.setValue( minormax? minY : maxY );
    }
    sampleVector.addSample( sample );
    if ( sample.t2000Msec < ti_filled.tstart )
        ti_filled.tstart = sample.t2000Msec;
    if ( sample.t2000Msec > ti_filled.tend() )
        ti_filled.tduration = sample.t2000Msec - ti_filled.tstart;
    m_skippedCount = 0;
    resetMinMax();
}

KSRtpSamplesVector& KSRtpSamplesBuf::getVector(int day) {
    std::lock_guard<std::mutex> lg( m_mutex );
    KSRtpSamplesVector& res = m_buf[day];
    return res;
}

void KSRtpSamplesBuf::reset() {
    std::lock_guard<std::mutex> lg( m_mutex );
    m_buf.clear();
    m_poolToAdd = nullptr;
    //ti_filled = tint;
    ti_filled.tduration = 0;
}

void KSRtpSamplesBuf::markToReset() {
    m_markedToReset = true;
    ti_filled.tduration = 0;
}

int KSRtpSamplesBuf::calcDensity() {
    int samples = 0;
    for ( const auto& [day, sb] : m_buf ) {
        samples += sb.pool.size();
    }
    //ti_filled.tduration
    return 0;
}

//
// KSTrendsBuf
//
std::string KSTrendsBuf::getParIdByIndex(int index)
{
    std::lock_guard<std::mutex> lg( m_mutex );
    return m_ids[index];
}

int KSTrendsBuf::getTrendsCount()
{
    std::lock_guard<std::mutex> lg( m_mutex );
    int res = m_pool.size();
    return res;
}

KSRtpSamplesBuf& KSTrendsBuf::get( const std::string& dbParamId )
{
    std::lock_guard<std::mutex> lg( m_mutex );
    return m_pool[ dbParamId ];
}

void KSTrendsBuf::markToReset() {
    std::lock_guard<std::mutex> lg( m_mutex );
    for ( auto& [dpParamId, sbuf ] : m_pool ) {
        sbuf.markToReset();
    }
}

void KSTrendsBuf::zoomInNotify() {
    if ( ++m_zoomInSteps < 15 )
        return;
    m_zoomInSteps = 0;
    markToReset();
}

void KSTrendsBuf::setMinTimeSpan(int timespan) {
    std::lock_guard<std::mutex> lg( m_mutex );
    for ( auto& [dpParamId, sbuf ] : m_pool ) {
        sbuf.m_minTimeSpan = timespan;
    }
}

void KSTrendsBuf::toggleTrend(int idx) {
    std::lock_guard<std::mutex> lg( m_mutex );
    std::string dbPramId = m_ids[idx];
    if ( dbPramId.length() ) {
        m_pool[ dbPramId ].m_enabled = !m_pool[ dbPramId ].m_enabled;
    }
}

void KSTrendsBuf::resetData() {
    std::lock_guard<std::mutex> lg( m_mutex );
    for ( auto& [dbParamId, sb] : m_pool ) {
        sb.reset();
        sb.ti_filled.reset();
    }
}

void KSTrendsBuf::setTrend( int idx, const std::string& dbParamId, uint32_t color ) {
    std::lock_guard<std::mutex> lg( m_mutex );
    if ( idx < 0 || idx > (int)m_ids.size() )
        return;

    m_ids[idx] = dbParamId;
    m_pool[dbParamId].m_enabled = true;
    m_pool[dbParamId].m_trendColor = color;
}

void KSTrendsBuf::clearTrends() {
    std::lock_guard<std::mutex> lg( m_mutex );
    m_ids.resize( 50 );
    for ( size_t i=0; i < m_ids.size(); ++i ) {
        m_ids[i] = "";
    }
    m_pool.clear();
}


KSTrendsBuf trendsBuf;


