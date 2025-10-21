#ifndef SDDAPISTRUCTS_H
#define SDDAPISTRUCTS_H


#include "KBasicStructs.h"
#include <cstring>

#define QUERY_END			        0x0000
#define QUERY_NAME_STATION			0x1000
#define QUERY_VALUE_LIST_PASSP		0x2000
#define QUERY_WRITE_PASSP			0x3000
#define QUERY_HIST_VALUE            0x4000
#define QUERY_PASSPORT_ERROR        0x5000
#define QUERY_LIST_BASES            0x6000
#define QUERY_JSON                  0x7000

#define KLogicPasspType 0xDE // Тип паспорта, означающий параметр KLogic
#define KLogicControlPasspType 0xDF // Тип паспорта, означающий служебный параметр KLogic
#define KSControlPasspType 0xE0 // KS service type group


const char KSDASAPISignature[] = "SDDv";
const int posSignature = 94;
const int DASAPIVERSION = 1;
const char KSApiJsonRequestSignature[] = "KSJQ";


#define ValueTypeUnknown 0
#define ValueTypeAnalog  1
#define ValueTypeDiscret 2
#define ValueTypeDouble  3
#define ValueTypeInt64   4
#define ValueTypeUInt64  5
#define ValueTypeString  6
#define ValueTypeInt32   7

#define KLQ_OK                   0
#define KLQ_BAD_WRITE_DB         8
#define KLQ_OK_WRITE_DB          9
#define KLQ_CONTROLLER_NOT_FOUND 100
#define KLQ_TAG_NOT_FOUND        101
#define KLQ_READ_ERROR           102
#define KLQ_NULL_POINTER         103
#define KLQ_UNSUPPORTED_TYPE     104
#define KLQ_BAD_TAG              43
#define KLQ_KLMNGR_STOPPED       105
#define KLQ_WRITE_ERROR          106
#define KLQ_WRONG_STATION        107
#define KLQ_PROCESSING_ERROR     108
#define KLQ_UNKNOWN_ERROR        109
#define KLQ_LICENSE              229
#define KLQ_CNTRL_NOT_STARTED    230
#define KLQ_ERR                  255


#define QE_GOOD              0xC0
#define QE_BAD               0x00
#define QE_PARAM_UNDEFINE    201
#define QE_CRC               202
#define QE_TIMEOUT           203
#define QE_BUFFER            204
#define QE_SEND              205
#define QE_READ              206
#define QE_FORMAT            207
#define QE_UNSUPPORTED       208
#define QE_WRONG             209
#define QE_IMPOSSIBLE        210
#define QE_CONNECT           211
#define QE_TERMINATE         212
#define QE_JUST_CONNECT      213
#define QE_NOT_TOPICAL       214
#define QE_INVALID           215
#define QE_OVERFLOW          216

#define QE_UNKNOWN_PARAM     217
#define QE_CONFIG            218
#define QE_NOT_ANSWER        219
#define QE_UNKNOWN_ANSWER    220
#define QE_WRONG_ANSWER      221
#define QE_WRONG_ADDR        222

#define QE_UNKNOWN_LINK_TYPE  223
#define QE_UNSUPP_LINK_TYPE   224
#define QE_SOCKET             225

#define QE_MODEM_NOT_ANSWER   226
#define QE_MODEM_NOT_CONNECT  227
#define QE_MODEM_BUSY         228
#define QE_MODEM_NO_CARRIER   229
#define QE_MODEM_NO_ANSWER    230
#define QE_MODEM_NO_NUMBER    231
#define QE_MODEM_RING         232
#define QE_MODEM_NO_DIALTONE  233

#define QE_NOT_SUPPORTED      234
#define QE_COM_OPEN           235
#define QE_COM_BUSY           236

#define QE_SET_ERROR          237

#define QE_STOP_POLL          238

#define QE_PARAM_NOT_FOUND    239
#define QE_READ_ONLY          240
#define QE_PAR_NOT_SUPPORTED  241
#define QE_IMPOSSIBLE0        242

#define QE_LICENSE            243

#define QE_UNKNOWN            200

#define UNLIM_CONTROLLERS 0xFFFF
#define CONTROLLERS_LIMIT -22


#pragma pack(push, 1)

#ifndef KSFILETIME
typedef uint64_t KSFILETIME;
#endif

#include <charconv>

enum class KSApiRequestCmd { start = 1, next = 2, resend = 3, close = 4 };

struct KSDASRequestHeader {
    uint32_t fnubmer = 0;
    byte reserved[posSignature - sizeof(fnubmer)];
    uint32_t signature;
    word clientVersion = DASAPIVERSION;

    void init() {
        memcpy( &signature, KSDASAPISignature, sizeof(signature) );
    }
    KSDASRequestHeader() {
        init();
    }
};

struct KSDASRequestHeaderWithCmd : public KSDASRequestHeader
{
    word  query = 0;
};

struct KSApiStringHeader {
    dword  signature; // KSQJ signature
    dword  strCrc;
    dword  strLen;
    byte   reserved[32-12];

    void init() {
        memcpy( &signature, KSApiJsonRequestSignature, sizeof(signature) );
    }
    KSApiStringHeader() {
        init();
    }
    int setJson(const std::string& json);
};

struct KSApiJsonRequest {
    KSDASRequestHeader dasapih;
    word               query;
    KSApiStringHeader  ksjq;

    void init() {
        dasapih.init();
        query = QUERY_JSON;
        ksjq.init();
    }

    bool validate(std::string* message=nullptr);
    void getJson(std::string& json);
    int setJson(const std::string& json);
};


// структура, определяющая местоположение тега
struct ADDR_TAG {
    byte  StationID;	    // идентификатор станции
    int   CfgID;		    // идентификатор конфигурации
    int   DeviceID;		    // идентификатор устройства
    int   GroupID;		    // идентификатор группы
    int   TagID;		    // идентификар тега
    byte  TagType;		    // тип тега: 1-аналоговый, 2-дискретный
    int   FastID;		    // идентификатор быстрого доступа
};

// структура для получения информации о теге
struct INFO_TAG {
    int     TypeTag;    	// Действительный тип параметра:
                            // 1 - аналоговый, 2 - дискретный
    byte    ReadAccess;	    // Возможность чтения состояния тега:
                            // 0 - невозможно, не 0 - возможно
    byte    WriteAccess;	// Возможность записи состояния тега:
                            // 0 - невозможно, не 0 - возможно
    byte    UseScaling;	    // Наличие границ измерения у тега:
                            // 0 - нет, не 0 - есть
    float   HighLimit;  	// Верхняя граница измерения
    float   LowLimit;	    // Нижняя граница измерения
};

// объединение для значения тега
union VALUE_TAG {
    float   fValue;		    // значение для аналогового тега
    byte    bValue;		    // значение для дискретного тега
};

struct TIPValue {
    byte a1,a2,a3,a4;
};

// структура для состояния тега
struct STATE_TAG {
    int         TypeValue;	// желаемый тип значения тега
    KSFILETIME  TimeStamp;	// временная метка
    word        Quality;	// признаки качества значения тега
    VALUE_TAG   Value;      // значение тега
};

// объединение для значения паспорта
union VALUE_PASSP {
	float	    fValue;//
	byte	    bValue;//логическое
	//const char* cValue;//текстовое значение
	//TDateTime*  dtValue;//дата время
	int32_t     iValue;//целые числа
	TIPValue    ipValue;//ip адрес
};

union VALUE_PASSP_64 {
	float	    fValue;//
	byte	    bValue;//логическое
	char*       cValue;//текстовое значение
	uint64_t    dtValue;// timestamp
	int32_t     iValue;//целые числа
	TIPValue    ipValue;//ip адрес
	double      dValue;
    uint64_t    i64Value;
};

// структура для состояния паспорта
struct STATE_PASSP_F {
    byte		StationID;	// идентификатор станции
    byte		TypePassp;	// тип паспорта
    byte		GroupID;	// идентификатор группы
    word		PasspID;	// идентификатор паспорта
    dword     	FastID;		// идентификатор быстрого доступа
    KSFILETIME	TimeStamp;	// временная метка
    union  {
        word    Quality;	// признаки качества значения
        struct {
            byte l_Qual;    // младший байт
            byte h_Qual;    // старший байт
        };
    };
    byte		ValueType;	// тип значения
	//VALUE_PASSP	Value;		// значение паспорта

	STATE_PASSP_F(): l_Qual(QE_UNKNOWN), h_Qual(KLQ_TAG_NOT_FOUND) {}
};

struct STATE_PASSP: public STATE_PASSP_F
{
    VALUE_PASSP	Value;

    std::string getStrVal() const;

	template<class T> bool getVal(T& outVal) const {
        switch (ValueType) {
            case ValueTypeAnalog:
                outVal = Value.fValue;
                return true;
            case ValueTypeDiscret:
                outVal = Value.bValue;
                return true;
            case ValueTypeInt32:
                outVal = Value.iValue;
                return true;
            case ValueTypeString: {
                const char* ps = (const char*)this + sizeof(STATE_PASSP);
                auto [ptr, ec] = std::from_chars( ps, ps + Value.iValue, outVal );
                return (ec == std::errc{});
            }
            default: {
                //Log().W( std::format("{}::{} Unhandled value type {}", KSMETHOD, ValueType ) );
                break;
            }
        }
        return false;
	}
};

struct STATE_PASSP_64: public STATE_PASSP_F
{
    VALUE_PASSP_64	Value;
};


struct TPasportEx_64: public STATE_PASSP_64
{
    char stateAlarm_And_checkBack;
    TPasportEx_64(): STATE_PASSP_64(), stateAlarm_And_checkBack(0)
    { }
};

struct TPasportEx: public STATE_PASSP
{
    char stateAlarm_And_checkBack;
    TPasportEx(): STATE_PASSP(), stateAlarm_And_checkBack(0) {}
};

// объединение для значения паспорта
union TPasportValue{
    float	    fValue;//вещественное значение
    byte	    bValue;//логическое значение
    char*       cValue;//текстовое значение
    uint64_t    dtValue;//дата время
    int32_t     iValue;//целые числа
    TIPValue    ipValue;//ip адрес
};

// структура для состояния паспорта
struct TPasport {
    byte		StationID;	// идентификатор станции
    union {
        struct {
            byte		TypePasp;	// тип паспорта
            byte		GroupID;	// идентификатор группы
            word		PaspID;		// идентификатор паспорта
        };
        dword           dwPaspID;
    };
    int     	        FastID;		// идентификатор быстрого доступа
    KSFILETIME         	TimeStamp;	// временная метка
    union {
        struct {
            byte        Lo;
            byte        Hi;
        };
        word            Q;
    }	                Quality;	// признаки качества значения
    byte		        ValueType;	// тип значения:
	     				//0-не определен
	     				//1-вещественное
	     				//2-логическое
	     				//3-целочисленное
	     				//4-Дата/время
	     				//5-IP адрес
	     				//6-Строковый
    TPasportValue       Value;		// значение паспорта
};

// Структура для информации о паспорте
struct TPassportInformation
{
	char *StationName;
	char *StationTypeName;
	char *PassportTypeName;
	char *ParameterID;
	char *GroupName;
	char *FullName;
	char *MeasureUnit;
	float UpMeasuringBorder;
	float DownMeasuringBorder;
	float UpPreCrashingBorder;
	float DownPreCrashingBorder;
	float UpCrashingBorder;
	float DownCrashingBorder;
};

// структура для состояния паспорта
struct PARAM {
    KSFILETIME	TimeStamp;	// временная метка
    union {
        word    Quality;	// признаки качества значения
        struct {
            byte l_Qual;    // младший байт
            byte h_Qual;    // старший байт
        };
    };
    byte		    ValueType;	// тип значения
    TPasportValue	Value;	// значение паспорта
    byte		    SampleEnd;
};

struct MULTI_PASSP {
    PARAM Passp;
    int PasspIdx;
};

#pragma pack(pop)


#endif // SDDAPISTRUCTS_H

