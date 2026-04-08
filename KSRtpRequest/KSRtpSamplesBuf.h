#ifndef KSRTPSAMPLESBUF_H
#define KSRTPSAMPLESBUF_H

#include <vector>
#include <map>

#include "../../CommonCpp/SDDAPIStructs.h"

#include <cstring>

#include "../../CommonCpp/KSUtils/KSUtil.h"
#include "../../CommonCpp/KSDateTime/KSDateTime.h"

int getDay( uint64_t kstime );

//
//
//
struct KSRtpSamplesVector {
    int positionToInsert = 0;
    std::mutex m_mutex;
    std::vector<KSRtpPValueFullTime>  pool;

    void addSample(const KSRtpPValueFullTime& sample);
    int searchPositionToInsert(const KSRtpPValueFullTime& sample);
};



//
//
//
class KSRtpSamplesBuf {

    int m_dayToAdd = 0;
    KSRtpSamplesVector*  m_poolToAdd = nullptr;
    uint64_t m_lastInsertedSampleTime = 0;

    int m_skippedCount = 0;
    double minY = 0;
    double maxY = 0;
    int m_markedToReset = false;

    void resetMinMax();
    int calcDensity();

    std::map<int, KSRtpSamplesVector> m_buf;
    std::mutex m_mutex;

public:
    bool       m_enabled = true;
    uint32_t   m_trendColor = 0xFF000000;
    KSTInterval ti_filled;

    int m_minTimeSpan = 0;

    void addSample( KSRtpPValueFullTime& sample );
    void reset();
    void markToReset();

    KSRtpSamplesVector& getVector(int day);


    KSRtpSamplesBuf() { resetMinMax(); }
};


class KSTrendsBuf {
    std::map< std::string, KSRtpSamplesBuf>  m_pool;
    std::vector< std::string > m_ids;
    std::mutex m_mutex;
    std::mutex m_instanceMutex;

    int m_zoomInSteps = 0;

    void markToReset();

public:
    KSRtpSamplesBuf& get( const std::string& dbParamId );
    std::string getParIdByIndex(int index);
    int getTrendsCount();
    void setMinTimeSpan( int timespan );
    void resetData();
    void setTrend( int idx, const std::string& dbParamId, uint32_t color );
    void clearTrends();
    void toggleTrend(int idx);
    void zoomInNotify();
    void instanceOwn() { m_instanceMutex.lock(); }
    void instanceFree() { m_instanceMutex.unlock(); };

    KSTrendsBuf() { clearTrends(); }
};

extern KSTrendsBuf trendsBuf;

#endif
