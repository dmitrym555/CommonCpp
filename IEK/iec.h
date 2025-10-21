#ifndef IECH
#define IECH

#include "../KLBasicStructs.h"



#define IEC_APDU_MAX 255
#define IEC_ASDU_MAX IEC_APDU_MAX - 6
#define IEC_KLOGIC_ASDU_MAX IEC_ASDU_MAX
#define LEN_APDU_FLAG 3
#define IEC_BUF_SIZE IEC_APDU_MAX + 2 + 2 * LEN_APDU_FLAG

#define IEC_PORT_DEFAULT 2404
#define SSD_IEC_PORT_DEFAULT 2403
#define T0_DEFAULT 30
#define T1_DEFAULT 15
#define T2_DEFAULT 10
#define T3_DEFAULT 20
#define K_DEFAULT 12
#define W_DEFAULT 8

#define APDU_START 0x68

#define IEC_POLL 0x14 // признак общего опроса станции
#define KL_POLL 0x40 // признак общего опроса станции KLogicMngr'ом

#include <cstdint>

#include "../KSDateTime/KSDateTime.h"


// Типы APDU
enum TAPDUTYPE {APDU_TYPE_I = 0, APDU_TYPE_S = 1, APDU_TYPE_U = 3, APDU_TYPE_Err};

// APDU типа U
enum TAPDUu {STARTDT_ACT,STARTDT_CON,STOPDT_ACT,STOPDT_CON,TESTFR_ACT,TESTFR_CON};

// ASDU
enum TASDU
{
  ASDU_UNKNOUN = 0, // неподдерживаемый тип ASDU
  M_SP_NA_1 = 1,    // 0x01 - Одноэлементная информация
  M_DP_NA_1 = 3,    // 0x03 - Двухэлементная информация
  M_SP_TB_1 = 30,   // 0x1E - Одноэлементная информация с меткой времени CP56Время2а
  M_DP_TB_1 = 31,   // 0x1F - Двухэлементная информация с меткой времени CP56Время2а
  M_ME_NC_1 = 13,   // 0x0D - Значение измеряемой величины, короткий формат с плавающей запятой
  M_ME_TF_1 = 36,   // 0x24 - Значение измеряемой величины, короткий формат с плавающей запятой с меткой времени CP56Время2а
  M_KL_STR  = 137,  // 0x89 - Строка с меткой времени CP56Время2а (KLogic)
  C_IC_NA_1 = 100,  // 0x64 - Команда опроса
  C_CS_NA_1 = 103,  // 0x67 - Команда синхронизации времени
  C_SC_NA_1 = 45,   // 0x2D - Однопозиционная команда
  C_SC_TA_1 = 58,   // 0x3A - Одноэлементная команда с меткой времени CP56Время2а
  C_SE_NB_1 = 49,   // 0x31 - Команда уставки, масштабированное значение
  C_SE_TB_1 = 62,   // 0x3E - Команда уставки, масштабированное значение с меткой времени CP56Время2а
  C_SE_NC_1 = 50,   // 0x32 - Команда уставки, короткое число с плавающей запятой
  C_SE_TC_1 = 63,   // 0x3F - Команда уставки, короткое число с плавающей запятой с меткой времени CP56Время2а
  C_TS_NA_1 = 104,  // 0x68 - Команда тестирования
  C_TS_TA_1 = 107,  // 0x6B - Тестовая команда с меткой времени CP56Время2а
  C_KL_HIST = 136,  // 0x88 - Запрос истории (KLogic)
  C_KL_STR  = 138,  // 0x8A - Команда записи строки с меткой времени CP56Время2а (KLogic)
  C_KL_PROXY = 140, // 0x8C - Проксирование обмена по протоколу KLogic
  C_KL_OBJ = 142,   // 0x8E - Объектный опрос EnLogic
  ASDU_ERROR = 255  // некорректный ASDU
};

// Идентификаторы причины передачи
enum TCAUSE
{
  DEFAULT_CAUSE = 0,      // по умолчанию
//       <1>              // периодически,циклически
//       <2>              // фоновое сканирование
  SPORADIC = 3, // спорадически
//       <4>              // сообщение о инициализации
//       <5>              // запрос или запрашиваемое
  ACTIVATION = 6, // активация
  ACTIVATION_CON = 7, // подтверждение активации
//       <8>              // деактивация
//       <9>              // подтверждение деактивации
  ACTIVATION_END = 10, // завершение активации
//       <11>             // обратная информация, вызванная удаленной командой
//       <12>             // обратная информация, вызванная местной командой
//       <13>             // передача файлов
  COMMON_ANSWER = 20, // ответ на общий опрос
//       <21>             // ответ на опрос группы 1
//       <22>             // ответ на опрос группы 2
//       <23>             // ответ на опрос группы 3
//       <24>             // ответ на опрос группы 4
//       <25>             // ответ на опрос группы 5
//       <26>             // ответ на опрос группы 6
//       <27>             // ответ на опрос группы 7
//       <28>             // ответ на опрос группы 8
//       <29>             // ответ на опрос группы 9
//       <30>             // ответ на опрос группы 10
//       <31>             // ответ на опрос группы 11
//       <32>             // ответ на опрос группы 12
//       <33>             // ответ на опрос группы 13
//       <34>             // ответ на опрос группы 14
//       <35>             // ответ на опрос группы 15
//       <36>             // ответ на опрос группы 16
//       <37>             // ответ на общий запрос счетчиков
//       <38>             // ответ на запрос группы счетчиков 1
//       <39>             // ответ на запрос группы счетчиков 2
//       <40>             // ответ на запрос группы счетчиков 3
//       <41>             // ответ на запрос группы счетчиков 4
  KL_HIST_ANSWER = 48 // ответ на запрос истории
};

#pragma pack(push, 1)

typedef struct _APCI
{
  BYTE APDUStart; // APDU_START
  BYTE APDULength;
  union //ControlFields
  {
    struct //I
    {
      unsigned int idI : 1; // id = APDU_TYPE_I
      unsigned int nsI : 15;
      unsigned int dummyI  : 1;
      unsigned int nrI  : 15;
    };
    struct //S
    {
      unsigned int idS : 2; // id = APDU_TYPE_S
      unsigned int dummyS : 15;
      unsigned int nrS  : 15;
    };
    struct //U
    {
      unsigned int idU : 2; // id = APDU_TYPE_U
      unsigned int actSTARTDT : 1;
      unsigned int conSTARTDT : 1;
      unsigned int actSTOPDT : 1;
      unsigned int conSTOPDT : 1;
      unsigned int actTESTFR : 1;
      unsigned int conTESTFR : 1;
      unsigned int dummyU : 24;
    };
    struct {
        BYTE cf1;
        BYTE cf2;
        BYTE cf3;
        BYTE cf4;
    };
  };
}APCI;


// Data unit identifier block - ASDU header

typedef struct _ASDU_HEAD // 6 bytes
{
  BYTE type;        /* type identification */
  BYTE num      :7; /* number of information objects */
  BYTE sq       :1; /* sequenced/not sequenced address */
  BYTE cause    :6; /* cause of transmission */
  BYTE pn       :1; /* positive/negative app. confirmation */
  BYTE t        :1; /* test */
  BYTE cause_h  :8; /* cause of transmission  - station addr*/
  BYTE ca;          /* common address of ASDU */
  BYTE ca_h;        /* common address of ASDU - high byte*/
}ASDU_HEAD;


/* Information Object Address */

typedef struct _IOA
{
  WORD ioa;
  BYTE ioa_h; /* 0 */
}IOA; // 3 bytes



typedef struct _QDS // описатель качества

{
  BYTE ov :1; // overflow/no overflow (0 - нет переполнение)
  BYTE res :3; // резерв
  BYTE bl :1; // blocked/not blocked (0 - нет блокировки)
  BYTE sb :1; // substituted/not substituted (0 - нет замещения)
  BYTE nt :1; // not topical/topical (0 - актуальное значение)
  BYTE iv :1; // valid/invalid (0 - действительное значение)
}QDS;



// C_CS_NA_1 - Команда синхронизации времени

typedef struct _ASDU_C_CS_NA_1
{
  ASDU_HEAD head; // Заголовок (type = 0x67, num = 1, sq = 0)
  IOA ioa; // 0
  CP56TIME2A time; // время
}ASDU_C_CS_NA_1;


typedef struct _C_CS_NA_1_PARAM

{
  CP56TIME2A time; // время
}C_CS_NA_1_PARAM;


// C_IC_NA_1 - Команда опроса

typedef struct _ASDU_C_IC_NA_1
{
  ASDU_HEAD head; // Заголовок (type = 0x64, num = 1, sq = 0)
  IOA ioa; // 0
  BYTE QOI; // описатель запроса  QOI == 20 - общий опрос станции; 64 - опрос KLogicMngr'ом
}ASDU_C_IC_NA_1;


typedef struct _C_IC_NA_1_PARAM

{
  BYTE QOI; // описатель запроса  QOI == 20 - общий опрос станции; 64 - опрос KLogicMngr'ом
}C_IC_NA_1_PARAM;


// M_SP_TB_1 - Одноэлементная информация с меткой времени CP56Время2а

typedef struct _ASDU_M_SP_TB_1
{
  ASDU_HEAD head; // Заголовок (type = 30)
  IOA ioa; // Information Object Address
  BYTE sp :1; // single point information
  BYTE res :3;
  BYTE bl :1; // blocked/not blocked
  BYTE sb :1; // substituted/not substituted
  BYTE nt :1; // not topical/topical
  BYTE iv :1; // valid/invalid
  CP56TIME2A time;
}ASDU_M_SP_TB_1;


typedef struct _M_SP_TB_1_PARAM // 8 bytes

{
  BYTE sp :1; // single point information
  BYTE res :3;
  BYTE bl :1; // blocked/not blocked
  BYTE sb :1; // substituted/not substituted
  BYTE nt :1; // not topical/topical
  BYTE iv :1; // valid/invalid
  CP56TIME2A time;
}M_SP_TB_1_PARAM;


typedef struct _M_SP_NA_1_PARAM // 1 byte

{
  BYTE sp :1; // single point information
  BYTE res :3;
  BYTE bl :1; // blocked/not blocked
  BYTE sb :1; // substituted/not substituted
  BYTE nt :1; // not topical/topical
  BYTE iv :1; // valid/invalid
}M_SP_NA_1_PARAM;


typedef struct _M_DP_TB_1_PARAM

{
  BYTE dpi :2; // double point information
  BYTE res :2;
  BYTE bl :1; // blocked/not blocked
  BYTE sb :1; // substituted/not substituted
  BYTE nt :1; // not topical/topical
  BYTE iv :1; // valid/invalid
  CP56TIME2A time;
}M_DP_TB_1_PARAM;


typedef struct _M_DP_NA_1_PARAM
{
  BYTE dpi :2; // double point information
  BYTE res :2;
  BYTE bl :1; // blocked/not blocked
  BYTE sb :1; // substituted/not substituted
  BYTE nt :1; // not topical/topical
  BYTE iv :1; // valid/invalid
}M_DP_NA_1_PARAM;


// M_ME_TF_1 - Значение измеряемой величины, короткий формат с плавающей запятой с меткой времени CP56Время2а

typedef struct _ASDU_M_ME_TF_1
{
  ASDU_HEAD head; // Заголовок (type = 36)
  IOA ioa; // Information Object Address
  float mv; // значение
  QDS qds; // описатель качества
  CP56TIME2A time; // временная метка
}ASDU_M_ME_TF_1;


typedef struct _M_ME_TF_1_PARAM
{
  float mv;
  QDS qds;
  CP56TIME2A time;
}M_ME_TF_1_PARAM;

typedef struct _M_ME_NC_1_PARAM
{
  float mv;
  QDS qds;
}M_ME_NC_1_PARAM;


typedef struct _M_KL_STR_PARAM
{
  BYTE res :4;
  BYTE bl :1; // blocked/not blocked
  BYTE sb :1; // substituted/not substituted
  BYTE nt :1; // not topical/topical
  BYTE iv :1; // valid/invalid
  CP56TIME2A time;
  BYTE StrLen; // длина строки (завершающий 0 не учитывается, но передаётся)
  char Str[1]; // строка
}M_KL_STR_PARAM;

// C_SE_NC_1 - Однопозиционная команда
typedef struct _ASDU_C_SC_NA_1
{
  ASDU_HEAD head; // Заголовок (type = 45)
  IOA ioa; // адрес
  BYTE SCS : 1; // 0 - ВЫКЛ, 1 - ВКЛ
  BYTE res : 1; // 0
  BYTE QU  : 5; // 0 - нет дополнительного определения
  BYTE SE  : 1; // 0 - исполнение, 1 - выбор
}ASDU_C_SC_NA_1;

// C_SC_TA_1 - Одноэлементная команда с меткой времени CP56Время2а
typedef struct _ASDU_C_SC_TA_1
{
  ASDU_C_SC_NA_1 asdu_C_SC_NA_1;
  CP56TIME2A time;
}ASDU_C_SC_TA_1;

// C_SE_NB_1 - Команда уставки, масштабированное значение
typedef struct _ASDU_C_SE_NB_1
{
  ASDU_HEAD head; // Заголовок (type = 49)
  IOA ioa; // адрес
  short int NVA; // значение
  BYTE QL : 7; // 0
  BYTE SE : 1; // 0 - исполнение, 1 - выбор
}ASDU_C_SE_NB_1;


// C_SE_TB_1 - Команда уставки, масштабированное значение с меткой времени CP56Время2а

typedef struct _ASDU_C_SE_TB_1
{
  ASDU_C_SE_NB_1 asdu_C_SE_NB_1;
  CP56TIME2A time;
}ASDU_C_SE_TB_1;


// C_SE_NC_1 - Команда уставки, короткое число с плавающей запятой

typedef struct _ASDU_C_SE_NC_1
{
  ASDU_HEAD head; // Заголовок (type = 50)
  IOA ioa; // адрес
  float mv; // значение
  BYTE QL : 7; // 0
  BYTE SE : 1; // 0 - исполнение, 1 - выбор
}ASDU_C_SE_NC_1;


// C_SE_TC_1 - Команда уставки, короткое число с плавающей запятой с меткой времени CP56Время2а

typedef struct _ASDU_C_SE_TC_1
{
  ASDU_C_SE_NC_1 asdu_C_SE_NC_1;
  CP56TIME2A time;
}ASDU_C_SE_TC_1;

// C_KL_STR - Команда записи строки с меткой времени CP56Время2а
typedef struct _ASDU_C_KL_STR
{
  ASDU_HEAD head; // Заголовок (type = 138)
  IOA ioa; // адрес
  BYTE QL : 7; // 0
  BYTE SE : 1; // 0 - исполнение, 1 - выбор
  CP56TIME2A time;
  BYTE StrLen; // длина строки (завершающий 0 не учитывается, но передаётся) // получается, что StrLen максимально может быть равен 230
  char Str[1]; // строка
}ASDU_C_KL_STR;

// C_TS_NA_1 - Команда тестирования
typedef struct _ASDU_C_TS_NA_1
{
  ASDU_HEAD head; // Заголовок (type = 104)
  IOA ioa; // адрес объекта информации
  WORD FSP; // Фиксированный образец теста = 0x55AA
}ASDU_C_TS_NA_1; // 11


// C_TS_TA_1 - Тестовая команда с меткой времени CP56Время2а
typedef struct _ASDU_C_TS_TA_1
{
  ASDU_HEAD head; // Заголовок (type = 107)
  IOA ioa; // адрес объекта информации
  WORD TSC; // Счетчик тестовой последовательности
  CP56TIME2A time;
}ASDU_C_TS_TA_1;


// C_KL_HIST - Запрос истории по параметру

typedef struct _ASDU_C_KL_HIST
{
  ASDU_HEAD head; // Заголовок (type = 136)
  IOA ioa; // адрес объекта информации
  WORD MaxQuan; // Максимальное кол-во точек
  CP56TIME2A time; // Начальное время
}ASDU_C_KL_HIST;


// C_KL_PROXY - Проксирование обмена по протоколу KLogic

typedef struct _ASDU_C_KL_PROXY
{
  ASDU_HEAD head; // Заголовок (type = 140)
  BYTE First : 1; // Первый пакет
  BYTE Last  : 1; // Последний пакет
  BYTE Res : 6;   // Резерв
  BYTE Len;       // Длина пакета
  BYTE Data[1];   // Сам пакет (максимально IEC_KLOGIC_ASDU_MAX - sizeof(ASDU_HEAD) - 2 байт)
}ASDU_C_KL_PROXY;


// C_KL_OBJ - Объектный опрос EnLogic

typedef struct _ASDU_C_KL_OBJ
{
  ASDU_HEAD head; // Заголовок (type = 142)
  BYTE Len;       // Длина пакета
  BYTE Data[];    // Сам пакет (максимально IEC_KLOGIC_ASDU_MAX - sizeof(ASDU_HEAD) - 1 байт)
}ASDU_C_KL_OBJ;


/* M_SP_TB_1 - single point information with quality description and time tag */

struct iec_type30
{
    BYTE        sp  :1; /* single point information */
    BYTE        res :3;
    BYTE        bl  :1; /* blocked/not blocked */
    BYTE        sb  :1; /* substituted/not substituted */
    BYTE        nt  :1; /* not topical/topical */
    BYTE        iv  :1; /* valid/invalid */
    CP56TIME2A  time;
};


/* M_ME_TF_1 - short floating point measurement value and time tag */

struct iec_type36
{
    float       mv;
    BYTE        ov  :1; /* overflow/no overflow */
    BYTE        res :3;
    BYTE        bl  :1; /* blocked/not blocked */
    BYTE        sb  :1; /* substituted/not substituted */
    BYTE        nt  :1; /* not topical/topical */
    BYTE        iv  :1; /* valid/invalid */
    CP56TIME2A  time;
};
#pragma pack(pop)


extern bool IsAPDU(char* APDUbuffer, int APDUsize);

extern int PrepareAPDUu(TAPDUu APDUtype, byte* APDUbuffer);

extern TAPDUTYPE GetTypeAPDU(APCI* pAPCI);

extern TASDU GetTypeASDU(const byte* APDUbuffer);

extern bool IsAsduNeedConfirm(TASDU AsduType);

extern bool PrepareASDU(KLPARAM* pKLPARAM, const KSDateTime& TimeStamp, dword IecAddr, BYTE Cause, char* ASDUbuf, char* ASDUsize, BYTE CommonAsduAddr);

extern dword GetIOAasUlong(IOA ioa);

#endif
