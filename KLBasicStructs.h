#ifndef KLBASICSTRUCTS_H
#define KLBASICSTRUCTS_H

#include <cstdint>
#include <string>
#include "string.h"

#include "KSuperBasic.h"

#include "KSDateTime/KSDateTime.h"

enum TEventType {etSimple = 0, etAlarmPMA, etAlarmPMI, etAlarmAMA, etAlarmAMI, etAlarmNormal, etAlarmFalse, etAlarmTrue,
                 etAlarmSwitch, etAlarmTxt, etAlarmRange, etAlarmLess, etAlarmMore, etAlarmError, etKLTU, etDispTU};

enum TypeMax
{
  DeviceMax, ChanelMax
};


#define KLIOTYPE_FLOAT     0  // 000
#define KLIOTYPE_BOOLEAN   1  // 001
#define KLIOTYPE_INTEGER   2  // 010
#define KLIOTYPE_UNKNOWN   3  // 011
#define KLIOTYPE_DATETIME  4  // 100
#define KLIOTYPE_IPADDR    5  // 101
#define KLIOTYPE_STRING    6  // 110
#define KLIOTYPE_VARIANT   7  // 111

#define KLPARAM_QUALITY_GOOD        0xC0
#define KLPARAM_QUALITY_BAD         0x00
#define KLPARAM_QUALITY_BREAK       0xff
#define KLPARAM_QUALITY_ECONVERT    0x02


#define MAX_IEC_ADDR 0xFFFF
#define MAX_GA_IDX 0xFFFF



#pragma pack(push, 1)

struct KLParamH {
    byte Quality;
    byte Flags:5;
    byte Type:3;

    KLParamH(): Quality(KLPARAM_QUALITY_BAD) { Flags = 0; Type=0; }

    bool isgood();
};

// структура состояния параметра (тега) KLogic
struct KLPARAM : public KLParamH
{
    union {
        float         Float;
        dword         Boolean;
        dword         Integer;
        KLDATETIME32  DateTime;
        dword         StrId;
    } Value;

    bool operator == (const KLPARAM& other ) const {
        //return this->Value.Integer == rhs.Value.Integer;
        return ( 0 == memcmp( this, &other, sizeof(*this) ) );
    }

};

struct KSPARAM : public KLParamH
{
    union {
        //float         Float;
        byte          Boolean;
        uint64_t      DateTime;
        int64_t       StrId;
        double        Double;
        uint64_t      uInt64;
        int64_t       Integer;
    } Value;

    void Assign(const KSPARAM& other);
    void Assign(const KLPARAM& other);
    void AssignTo(KLPARAM& other);

    KSPARAM& operator = ( const KLPARAM& other ) {
        Assign(other);
        return *this;
    }

    KSPARAM( const KLPARAM& other ) {
        Assign( other );
    }

    double getDoubleVal() const;
    void setDoubleVal( double val );

    KSPARAM() = default;

    bool operator == (const KSPARAM& other ) const {
        return ( 0 == memcmp( this, &other, sizeof(*this) ) );
    }

    std::string getString();

    std::string displayString();

};


struct KLParamFields
{
	word IecID = -1;
	word ModbusAddr = -1;
	//GUID ArchiveGUID;
	//byte NumberInArchive;
	float Multiplier = 1;
	float Summand = 0;
	byte ParamType;
	float AMA;
	float AMI;
	float PMA;
	float PMI;
	float NechFrom;
	float NechTo;
	float NechVal;
	float KoefFilt;
	uint8_t   useAM:1;
	uint8_t   usePAM:1;
	uint8_t   useNech:1;
	uint8_t   isDemo:1;
	uint8_t   writable:1;
	uint8_t   invert:1;

	KLParamFields() {
	    useAM = usePAM = useNech = isDemo = writable = invert = 0;
	}
};

struct KLParamDescriptor {
    std::string     shortName;
    std::string     description;
    std::string     Name;
    KLParamFields   paramFields;
};


struct KSPARAMWITHDESCR {
    KLParamDescriptor id;
    uint64_t          timestamp;
    uint8_t           wasInitialized :1;
    uint8_t           fromArchive :1;

    KSPARAM           param;

    int getZoneStatus();

    KSPARAMWITHDESCR() {
        wasInitialized = 0;
        fromArchive = 0;
    }
};

struct KSPARAMWithCtrlId {
    KSPARAMWITHDESCR kspd;
    word rttId;
    //uint8_t    fromArchive:1;

    bool setAddr( const std::string& straddr );

};


// структура для передачи исторического значения
// в процедуре выборки истории
struct KLHISTVALUE
{
    KLDATETIME64    rdt;
    KLPARAM         Param;
};

struct TASK_HEADER
{
  long   CfgSize;
  short  Number;
  char   Priority;
  long   Period;
  char   Type;
};

struct USER_TASK_HEADER
{
  long  CfgSize;
  short   Number;
  char   Priority;
  long  Period;
  char   Type;
  short   QuanParams;
};

struct USER_TASK_PARAM
{
  short NumParamInGlobalArray;
  char ParamFlags;
};

typedef enum
{
  utfCommandStop  = 0x1,
  utfStopped      = 0x2,
  utfAllowOneStep = 0x4
} USER_TASK_FLAGS;

struct SERIAL_TASK_CB
{
  USER_TASK_HEADER* pHeader; // заголовок задачи
  USER_TASK_PARAM*  pListParams; // список сигналов по гл.массиву
  short  ProtocolType;
  short  QuanModules;
  void  *pModules;
};

#define SIZE_PACK_USER_TASK_PARAM  sizeof(USER_TASK_PARAM)

#pragma pack(pop)

#define GET_WORD_FROM_PTR_HW(p)  *((WORD*)(p))
#define GET_DWORD_FROM_PTR_HW(p) *((DWORD*)(p))
#define GET_WORD_FROM_PTR(p)  *((WORD*)(p))
#define GET_DWORD_FROM_PTR(p) *((DWORD*)(p))
#define GET_FLOAT_FROM_PTR(p) *((float*)(p))
#define GET_LONG_FROM_PTR(p) *((long*)(p))

// типы задач
#define USER_TASK           0  // задача пользователя (ФБД)
#define SERIAL_TASK         1  // задача опроса внешних УСО через COM-порт
#define MODBUS_TASK         2  // задача обмена с верхним уровнем по COM-порту
#define LOCAL_IO_TASK       3  // задача опроса внутренних УСО
#define UDP_TASK            4  // задача обмена с верхним уровнем по Ethernet
#define RESERV_TASK         5  // задача синхронизации при резервировании
#define HIST_ARCH_TASK      6  // задача ведения исторического архива
#define OPER_ARCH_TASK      7  // задача ведения оперативного архива
#define EXCHANGE_TASK       8  // задача обмена по контроллерной сети (Ethernet)
#define IEC_TASK            9  // задача МЭК
#define REMOTE_SERIAL_TASK 10 // задача удаленного опроса УСО
#define UNI_IO_TASK        11  // задача универсального ввода-вывода (в новом формате)


typedef enum
{
  PROP_NOT_FOUND = 0, // значение используется для возврата кодов ошибок
  PROP_BYTE = 1,      // целое беззнаковое, 1 байт
  PROP_WORD = 2,      // целое беззнаковое, 2 байта
  PROP_DWORD = 3,     // целое беззнаковое, 4 байта
  PROP_LONGINT = 4,   // целое со знаком, 4 байта (long, integer)
  PROP_FLOAT = 5,     // плавающее, 4 байта (float, single)
  PROP_DATETIME = 6,  // дата время, 4 байта, в формате RDATETIME
  PROP_IP_ADDR = 7,   // IP-адрес, 4 байта
  PROP_STRING = 8,    // строка, заканчивающаяся нулем
  PROP_BYTEBOOL = 9,  // булевское значение, 1 байт. 0 - false, не 0 - true.
  PROP_SMALLINT = 10, // целое со знаком, 2 байта
  // …
  PROP_ERROR = 255    // значение используется для возврата кодов ошибок
} PROP_TYPE;

struct PAAI
{
  bool Present; // Признак наличия параметра в массиве ParamAddrArray
  word PAAindex; // Индекс параметра в массиве ParamAddrArray
};

#define MAX_IEC_ADDR 0xFFFF
#define MAX_GA_IDX 0xFFFF

#define KSEVENT_PARAMCHANGED 1
#define KSEVENT_ARCHPARAMINIT 2
//#define KSEVENT_DBSTART 3
#define KSEVENT_ALARMCHANGE 4


#endif // KLBASICSTRUCTS_H
