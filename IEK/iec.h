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

#define IEC_POLL 0x14 // ������� ������ ������ �������
#define KL_POLL 0x40 // ������� ������ ������ ������� KLogicMngr'��

#include <cstdint>

#include "../KSDateTime/KSDateTime.h"


// ���� APDU
enum TAPDUTYPE {APDU_TYPE_I = 0, APDU_TYPE_S = 1, APDU_TYPE_U = 3, APDU_TYPE_Err};

// APDU ���� U
enum TAPDUu {STARTDT_ACT,STARTDT_CON,STOPDT_ACT,STOPDT_CON,TESTFR_ACT,TESTFR_CON};

// ASDU
enum TASDU
{
  ASDU_UNKNOUN = 0, // ���������������� ��� ASDU
  M_SP_NA_1 = 1,    // 0x01 - �������������� ����������
  M_DP_NA_1 = 3,    // 0x03 - �������������� ����������
  M_SP_TB_1 = 30,   // 0x1E - �������������� ���������� � ������ ������� CP56�����2�
  M_DP_TB_1 = 31,   // 0x1F - �������������� ���������� � ������ ������� CP56�����2�
  M_ME_NC_1 = 13,   // 0x0D - �������� ���������� ��������, �������� ������ � ��������� �������
  M_ME_TF_1 = 36,   // 0x24 - �������� ���������� ��������, �������� ������ � ��������� ������� � ������ ������� CP56�����2�
  M_KL_STR  = 137,  // 0x89 - ������ � ������ ������� CP56�����2� (KLogic)
  C_IC_NA_1 = 100,  // 0x64 - ������� ������
  C_CS_NA_1 = 103,  // 0x67 - ������� ������������� �������
  C_SC_NA_1 = 45,   // 0x2D - ��������������� �������
  C_SC_TA_1 = 58,   // 0x3A - �������������� ������� � ������ ������� CP56�����2�
  C_SE_NB_1 = 49,   // 0x31 - ������� �������, ���������������� ��������
  C_SE_TB_1 = 62,   // 0x3E - ������� �������, ���������������� �������� � ������ ������� CP56�����2�
  C_SE_NC_1 = 50,   // 0x32 - ������� �������, �������� ����� � ��������� �������
  C_SE_TC_1 = 63,   // 0x3F - ������� �������, �������� ����� � ��������� ������� � ������ ������� CP56�����2�
  C_TS_NA_1 = 104,  // 0x68 - ������� ������������
  C_TS_TA_1 = 107,  // 0x6B - �������� ������� � ������ ������� CP56�����2�
  C_KL_HIST = 136,  // 0x88 - ������ ������� (KLogic)
  C_KL_STR  = 138,  // 0x8A - ������� ������ ������ � ������ ������� CP56�����2� (KLogic)
  C_KL_PROXY = 140, // 0x8C - ������������� ������ �� ��������� KLogic
  C_KL_OBJ = 142,   // 0x8E - ��������� ����� EnLogic
  ASDU_ERROR = 255  // ������������ ASDU
};

// �������������� ������� ��������
enum TCAUSE
{
  DEFAULT_CAUSE = 0,      // �� ���������
//       <1>              // ������������,����������
//       <2>              // ������� ������������
  SPORADIC = 3, // ������������
//       <4>              // ��������� � �������������
//       <5>              // ������ ��� �������������
  ACTIVATION = 6, // ���������
  ACTIVATION_CON = 7, // ������������� ���������
//       <8>              // �����������
//       <9>              // ������������� �����������
  ACTIVATION_END = 10, // ���������� ���������
//       <11>             // �������� ����������, ��������� ��������� ��������
//       <12>             // �������� ����������, ��������� ������� ��������
//       <13>             // �������� ������
  COMMON_ANSWER = 20, // ����� �� ����� �����
//       <21>             // ����� �� ����� ������ 1
//       <22>             // ����� �� ����� ������ 2
//       <23>             // ����� �� ����� ������ 3
//       <24>             // ����� �� ����� ������ 4
//       <25>             // ����� �� ����� ������ 5
//       <26>             // ����� �� ����� ������ 6
//       <27>             // ����� �� ����� ������ 7
//       <28>             // ����� �� ����� ������ 8
//       <29>             // ����� �� ����� ������ 9
//       <30>             // ����� �� ����� ������ 10
//       <31>             // ����� �� ����� ������ 11
//       <32>             // ����� �� ����� ������ 12
//       <33>             // ����� �� ����� ������ 13
//       <34>             // ����� �� ����� ������ 14
//       <35>             // ����� �� ����� ������ 15
//       <36>             // ����� �� ����� ������ 16
//       <37>             // ����� �� ����� ������ ���������
//       <38>             // ����� �� ������ ������ ��������� 1
//       <39>             // ����� �� ������ ������ ��������� 2
//       <40>             // ����� �� ������ ������ ��������� 3
//       <41>             // ����� �� ������ ������ ��������� 4
  KL_HIST_ANSWER = 48 // ����� �� ������ �������
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



typedef struct _QDS // ��������� ��������

{
  BYTE ov :1; // overflow/no overflow (0 - ��� ������������)
  BYTE res :3; // ������
  BYTE bl :1; // blocked/not blocked (0 - ��� ����������)
  BYTE sb :1; // substituted/not substituted (0 - ��� ���������)
  BYTE nt :1; // not topical/topical (0 - ���������� ��������)
  BYTE iv :1; // valid/invalid (0 - �������������� ��������)
}QDS;



// C_CS_NA_1 - ������� ������������� �������

typedef struct _ASDU_C_CS_NA_1
{
  ASDU_HEAD head; // ��������� (type = 0x67, num = 1, sq = 0)
  IOA ioa; // 0
  CP56TIME2A time; // �����
}ASDU_C_CS_NA_1;


typedef struct _C_CS_NA_1_PARAM

{
  CP56TIME2A time; // �����
}C_CS_NA_1_PARAM;


// C_IC_NA_1 - ������� ������

typedef struct _ASDU_C_IC_NA_1
{
  ASDU_HEAD head; // ��������� (type = 0x64, num = 1, sq = 0)
  IOA ioa; // 0
  BYTE QOI; // ��������� �������  QOI == 20 - ����� ����� �������; 64 - ����� KLogicMngr'��
}ASDU_C_IC_NA_1;


typedef struct _C_IC_NA_1_PARAM

{
  BYTE QOI; // ��������� �������  QOI == 20 - ����� ����� �������; 64 - ����� KLogicMngr'��
}C_IC_NA_1_PARAM;


// M_SP_TB_1 - �������������� ���������� � ������ ������� CP56�����2�

typedef struct _ASDU_M_SP_TB_1
{
  ASDU_HEAD head; // ��������� (type = 30)
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


// M_ME_TF_1 - �������� ���������� ��������, �������� ������ � ��������� ������� � ������ ������� CP56�����2�

typedef struct _ASDU_M_ME_TF_1
{
  ASDU_HEAD head; // ��������� (type = 36)
  IOA ioa; // Information Object Address
  float mv; // ��������
  QDS qds; // ��������� ��������
  CP56TIME2A time; // ��������� �����
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
  BYTE StrLen; // ����� ������ (����������� 0 �� �����������, �� ���������)
  char Str[1]; // ������
}M_KL_STR_PARAM;

// C_SE_NC_1 - ��������������� �������
typedef struct _ASDU_C_SC_NA_1
{
  ASDU_HEAD head; // ��������� (type = 45)
  IOA ioa; // �����
  BYTE SCS : 1; // 0 - ����, 1 - ���
  BYTE res : 1; // 0
  BYTE QU  : 5; // 0 - ��� ��������������� �����������
  BYTE SE  : 1; // 0 - ����������, 1 - �����
}ASDU_C_SC_NA_1;

// C_SC_TA_1 - �������������� ������� � ������ ������� CP56�����2�
typedef struct _ASDU_C_SC_TA_1
{
  ASDU_C_SC_NA_1 asdu_C_SC_NA_1;
  CP56TIME2A time;
}ASDU_C_SC_TA_1;

// C_SE_NB_1 - ������� �������, ���������������� ��������
typedef struct _ASDU_C_SE_NB_1
{
  ASDU_HEAD head; // ��������� (type = 49)
  IOA ioa; // �����
  short int NVA; // ��������
  BYTE QL : 7; // 0
  BYTE SE : 1; // 0 - ����������, 1 - �����
}ASDU_C_SE_NB_1;


// C_SE_TB_1 - ������� �������, ���������������� �������� � ������ ������� CP56�����2�

typedef struct _ASDU_C_SE_TB_1
{
  ASDU_C_SE_NB_1 asdu_C_SE_NB_1;
  CP56TIME2A time;
}ASDU_C_SE_TB_1;


// C_SE_NC_1 - ������� �������, �������� ����� � ��������� �������

typedef struct _ASDU_C_SE_NC_1
{
  ASDU_HEAD head; // ��������� (type = 50)
  IOA ioa; // �����
  float mv; // ��������
  BYTE QL : 7; // 0
  BYTE SE : 1; // 0 - ����������, 1 - �����
}ASDU_C_SE_NC_1;


// C_SE_TC_1 - ������� �������, �������� ����� � ��������� ������� � ������ ������� CP56�����2�

typedef struct _ASDU_C_SE_TC_1
{
  ASDU_C_SE_NC_1 asdu_C_SE_NC_1;
  CP56TIME2A time;
}ASDU_C_SE_TC_1;

// C_KL_STR - ������� ������ ������ � ������ ������� CP56�����2�
typedef struct _ASDU_C_KL_STR
{
  ASDU_HEAD head; // ��������� (type = 138)
  IOA ioa; // �����
  BYTE QL : 7; // 0
  BYTE SE : 1; // 0 - ����������, 1 - �����
  CP56TIME2A time;
  BYTE StrLen; // ����� ������ (����������� 0 �� �����������, �� ���������) // ����������, ��� StrLen ����������� ����� ���� ����� 230
  char Str[1]; // ������
}ASDU_C_KL_STR;

// C_TS_NA_1 - ������� ������������
typedef struct _ASDU_C_TS_NA_1
{
  ASDU_HEAD head; // ��������� (type = 104)
  IOA ioa; // ����� ������� ����������
  WORD FSP; // ������������� ������� ����� = 0x55AA
}ASDU_C_TS_NA_1; // 11


// C_TS_TA_1 - �������� ������� � ������ ������� CP56�����2�
typedef struct _ASDU_C_TS_TA_1
{
  ASDU_HEAD head; // ��������� (type = 107)
  IOA ioa; // ����� ������� ����������
  WORD TSC; // ������� �������� ������������������
  CP56TIME2A time;
}ASDU_C_TS_TA_1;


// C_KL_HIST - ������ ������� �� ���������

typedef struct _ASDU_C_KL_HIST
{
  ASDU_HEAD head; // ��������� (type = 136)
  IOA ioa; // ����� ������� ����������
  WORD MaxQuan; // ������������ ���-�� �����
  CP56TIME2A time; // ��������� �����
}ASDU_C_KL_HIST;


// C_KL_PROXY - ������������� ������ �� ��������� KLogic

typedef struct _ASDU_C_KL_PROXY
{
  ASDU_HEAD head; // ��������� (type = 140)
  BYTE First : 1; // ������ �����
  BYTE Last  : 1; // ��������� �����
  BYTE Res : 6;   // ������
  BYTE Len;       // ����� ������
  BYTE Data[1];   // ��� ����� (����������� IEC_KLOGIC_ASDU_MAX - sizeof(ASDU_HEAD) - 2 ����)
}ASDU_C_KL_PROXY;


// C_KL_OBJ - ��������� ����� EnLogic

typedef struct _ASDU_C_KL_OBJ
{
  ASDU_HEAD head; // ��������� (type = 142)
  BYTE Len;       // ����� ������
  BYTE Data[];    // ��� ����� (����������� IEC_KLOGIC_ASDU_MAX - sizeof(ASDU_HEAD) - 1 ����)
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
