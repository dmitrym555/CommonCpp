#ifndef KBASICSTRUCTS_H
#define KBASICSTRUCTS_H

#include <cstdint>
#include <string>
#include <list>
#include <vector>

#include <sys/socket.h>

#include <mutex>

#include "KSuperBasic.h"


#ifndef TKSRtpParamId
typedef uint32_t TKSRtpParamId;
#endif


#define TKSMsecPerDay 86400000


#pragma pack(push, 1)


struct KSSharedMemoryStruct {
    uint32_t    stationId;
    char        projectPath[256];
    uint32_t    dasApiPort;
    uint32_t    alarmApiPort;
};

struct KSApiRequestHistorySingleStartHeader {
    word      cmd;
    word      version;      //
    word      stationId;    //
    word      rttId;        //
    dword     paramId;      //
    uint64_t  timeStart;    //
    uint64_t  timeEnd;      //
    dword     samplesLimit; //
    byte      reserved[64-36];
    dword     crc;          // 64
};

struct KSRtpBitFlags {
    uint16_t      type:4;
    uint16_t      qual:9;
    uint16_t      isServerTimeStamp:1;
    uint16_t      flag:1;
    uint16_t      discretVal:1;
};

struct KSRtpParamValueBase {
    union {
        KSRtpBitFlags bitflags;
        uint16_t meta;
    };
};

struct KSRtpParamValueValue32;

struct KSRtpParamValueValue64: public KSRtpParamValueBase {
    union {
        double    dval;
        int64_t   ival;
        uint64_t  uintval;
        bool      bval;
    } Value;                // 10
    double getDVal() const;
    uint64_t getIVal() const;
    void setValue(double dval);
    std::string getStringVal() const;
    void setValue(const KSRtpParamValueValue32& val32);
    void setValue(const KSRtpParamValueValue64& val);
};

struct KSRtpParamValueValue32: public KSRtpParamValueBase {
    union {
        float     fval;
        int32_t   ival;
        uint32_t  uintval;
        bool      bval;
    } Value;                //  6
    void setValue(const KSRtpParamValueValue64& val64);
};

struct KSRtpParamValue32 : public KSRtpParamValueValue32 {
    uint32_t    timeMsec;   // 10
};

struct KSRtpParamValue : public KSRtpParamValueValue64 {
    uint32_t    timeMsec;   // 14
};

struct KSRtpPValueFullTime : public KSRtpParamValueValue64 {
    uint64_t      t2000Msec;   // 18
};

struct KSRtpParamDiscretValue: public KSRtpParamValueBase {
    uint32_t    timeMsec;          //  7
};

struct KSRtpParamDiscretValueFullTime: public KSRtpParamValueBase {
    uint64_t      t2000Msec;       //  7
};

struct KSRtpParamBase {
    word               rttId;        //  4
    word               IecId;        //  6
    uint8_t            fromArchive:1;

    KSRtpParamBase() {
        fromArchive = 0;
    }
};

struct KSRtpParam : public KSRtpParamBase {
    KSRtpParamValue    value;        // 20
    uint32_t paramDbId() const { return (rttId << 16) + IecId; }
};

struct KSRtpParamFullTime : public KSRtpParamBase {
    KSRtpPValueFullTime    value;        // 20
    uint8_t                fromArchive:1;
    uint32_t paramDbId() const { return (rttId << 16) + IecId; }

    KSRtpParamFullTime() {
        fromArchive = 0;
    }
};


// 16-byte block header
struct KSRtpBlobHeader {
    uint16_t      version = 1;      //  2
    uint16_t      samplesCount;     //  4
    uint32_t      baseDay;          //  8
    uint8_t       type;             //  9
    uint8_t       compressed = 0;   // 10
    uint8_t       reserved[16-10];  // 16
};

struct KSDasapiId {
    int dasapiid;
    struct sockaddr cl_addr;
};


struct KSRtpReadSamplesReqStruct {
    int             cmd = 1;
    dword           requestId = 0;
    dword           sequenceNumber = 0;
    std::string     dbGuid;
    TKSRtpParamId   dbParamId = 1;
    uint64_t        timeStart = 0;
    uint64_t        timeDuration = 60*1000;
    dword           samplesLimit = 3000;
    dword           secondsPerSample = 0;
    byte            value64 = 1;
    KSDasapiId      dasapi;

    byte sampleSize() const {
        byte res = value64? sizeof(KSRtpParamValue) : sizeof(KSRtpParamValue32);
        return res;
    }
    uint64_t timeEnd() const { return timeStart + timeDuration; }
};



#pragma pack(pop)



#endif
