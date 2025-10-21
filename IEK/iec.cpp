//---------------------------------------------------------------------------
// Общие функции МЭК-104
//---------------------------------------------------------------------------

#include "iec.h"
#include "../aes/aes.h"

#include <stdlib.h>
#include <cstring>


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "../KSDateTime/KSDateTime.h"


//---------------------------------------------------------------------------

// ----------------------------------------------------
// Проверка APDU
// Наличие и размер буфера не проверяются
// Возвращает true, если в буфере APDU
// ----------------------------------------------------
bool IsAPDU(char* APDUbuffer, int APDUsize)
{
  if ((APDUsize < (int)sizeof(APCI)) || APDUsize > IEC_APDU_MAX)
    return false;
  APCI* pAPCI = (APCI*)APDUbuffer;
  return (pAPCI->APDUStart == APDU_START) && (pAPCI->APDULength == (APDUsize - 2));
}


// ----------------------------------------------------
// Подготовка APDU формата U в буфере APDUbuffer
// Наличие и размер буфера не проверяются
// Возвращает размер заполненной области буфера
// ----------------------------------------------------
int PrepareAPDUu(TAPDUu APDUtype, byte* APDUbuffer)
{
  APCI* pAPCI = (APCI*)APDUbuffer;
  pAPCI->APDUStart = APDU_START;
  pAPCI->APDULength = sizeof(APCI) - 2;
  pAPCI->idU = APDU_TYPE_U;
  pAPCI->actSTARTDT = (APDUtype == STARTDT_ACT);
  pAPCI->conSTARTDT = (APDUtype == STARTDT_CON);
  pAPCI->actSTOPDT = (APDUtype == STOPDT_ACT);
  pAPCI->conSTOPDT = (APDUtype == STOPDT_CON);
  pAPCI->actTESTFR = (APDUtype == TESTFR_ACT);
  pAPCI->conTESTFR = (APDUtype == TESTFR_CON);
  pAPCI->dummyU = 0;
  return sizeof(APCI);
}
//---------------------------------------------------------------------------



// ----------------------------------------------------
// Возвращает тип APDU
// Проверок на правильность переданного APCI не производится
// ----------------------------------------------------
TAPDUTYPE GetTypeAPDU(APCI* pAPCI)
{
  if (pAPCI->idI == APDU_TYPE_I)
    return APDU_TYPE_I;
  if (pAPCI->idS == APDU_TYPE_S)
    return APDU_TYPE_S;
  if (pAPCI->idU == APDU_TYPE_U)
    return APDU_TYPE_U;
  return APDU_TYPE_Err;
}
//---------------------------------------------------------------------------

// ----------------------------------------------------
// Возвращает тип ASDU (для APDU типа I)
// Проверка на корректность APCI не производится
// ----------------------------------------------------
TASDU GetTypeASDU(const byte* APDUbuffer)
{
  APCI* pAPCI = (APCI*)APDUbuffer;
  ASDU_HEAD* pASDUhead = (ASDU_HEAD*)&APDUbuffer[sizeof(APCI)];

  ASDU_HEAD& asduHead = *pASDUhead;

  const int paramtSize = sizeof(IOA) + sizeof(M_ME_TF_1_PARAM);


  switch (asduHead.type)
  {
    case M_SP_NA_1: // Одноэлементная информация
      if ((pAPCI->APDULength - 4 - sizeof(ASDU_HEAD)) == ((sizeof(IOA) + sizeof(M_SP_NA_1_PARAM)) * pASDUhead->num))
        return M_SP_NA_1;
      break;
    case M_SP_TB_1: // Одноэлементная информация с меткой времени CP56Время2а
      if ((pAPCI->APDULength - 4 - sizeof(ASDU_HEAD)) == ((sizeof(IOA) + sizeof(M_SP_TB_1_PARAM)) * pASDUhead->num))
        return M_SP_TB_1;
      break;
    case M_DP_NA_1: // Двухэлементная информация
      if ((pAPCI->APDULength - 4 - sizeof(ASDU_HEAD)) == ((sizeof(IOA) + sizeof(M_DP_NA_1_PARAM)) * pASDUhead->num))
        return M_DP_NA_1;
      break;
    case M_DP_TB_1: // Двухэлементная информация с меткой времени CP56Время2а
      if ((pAPCI->APDULength - 4 - sizeof(ASDU_HEAD)) == ((sizeof(IOA) + sizeof(M_DP_TB_1_PARAM)) * pASDUhead->num))
        return M_DP_TB_1;
      break;
    case M_ME_NC_1: // Значение измеряемой величины, короткий формат с плавающей запятой
      if ((pAPCI->APDULength - 4 - sizeof(ASDU_HEAD)) == ((sizeof(IOA) + sizeof(M_ME_NC_1_PARAM)) * pASDUhead->num))
        return M_ME_NC_1;
      break;
    case M_ME_TF_1: // Значение измеряемой величины, короткий формат с плавающей запятой с меткой времени CP56Время2а
      if ((pAPCI->APDULength - 4 - sizeof(ASDU_HEAD)) == (paramtSize * pASDUhead->num))
        return M_ME_TF_1;
      break;
    case M_KL_STR: // Строка с меткой времени CP56Время2а
      if ((pAPCI->APDULength - 4 - sizeof(ASDU_HEAD)) >= ((sizeof(IOA) + sizeof(M_KL_STR_PARAM)) * pASDUhead->num))
        return M_KL_STR;
      break;
    case C_IC_NA_1: // Команда опроса (общий опрос)
      if ((pASDUhead->num == 1) && ((pAPCI->APDULength - 4) == sizeof(ASDU_C_IC_NA_1)) &&
          ((((ASDU_C_IC_NA_1*)pASDUhead)->QOI == IEC_POLL) || (((ASDU_C_IC_NA_1*)pASDUhead)->QOI == KL_POLL)))
        return C_IC_NA_1;
      break;
    case C_CS_NA_1: // Команда синхронизации времени
      if ((pASDUhead->num == 1) && ((pAPCI->APDULength - 4) == sizeof(ASDU_C_CS_NA_1)))
        return C_CS_NA_1;
      break;
    case C_SC_NA_1: // Однопозиционная команда
      if ((pASDUhead->num == 1) && ((pAPCI->APDULength - 4) == sizeof(ASDU_C_SC_NA_1)))
        return C_SC_NA_1;
      break;
    case C_SC_TA_1: // Одноэлнементная команда с меткой времени CP56Время2а
      if ((pASDUhead->num == 1) && ((pAPCI->APDULength - 4) == sizeof(ASDU_C_SC_TA_1)))
        return C_SC_TA_1;
      break;
    case C_SE_NB_1: // Команда уставки, масштабированное значение
      if ((pASDUhead->num == 1) && ((pAPCI->APDULength - 4) == sizeof(ASDU_C_SE_NB_1)))
        return C_SE_NB_1;
      break;
    case C_SE_TB_1: // Команда уставки, масштабированное значение с меткой времени CP56Время2а
      if ((pASDUhead->num == 1) && ((pAPCI->APDULength - 4) == sizeof(ASDU_C_SE_TB_1)))
        return C_SE_TB_1;
      break;
    case C_SE_NC_1: // Команда уставки, короткое число с плавающей запятой
      if ((pASDUhead->num == 1) && ((pAPCI->APDULength - 4) == sizeof(ASDU_C_SE_NC_1)))
        return C_SE_NC_1;
      break;
    case C_SE_TC_1: // Команда уставки, короткое число с плавающей запятой с меткой времени CP56Время2а
      if ((pASDUhead->num == 1) && ((pAPCI->APDULength - 4) == sizeof(ASDU_C_SE_TC_1)))
        return C_SE_TC_1;
      break;
    case C_TS_NA_1: // Команда тестирования
      if ((pASDUhead->num == 1) && ((pAPCI->APDULength - 4) == sizeof(ASDU_C_TS_NA_1)))
        return C_TS_NA_1;
      break;
    case C_TS_TA_1: // Тестовая команда с меткой времени CP56Время2а
      if ((pASDUhead->num == 1) && ((pAPCI->APDULength - 4) == sizeof(ASDU_C_TS_TA_1)))
        return C_TS_TA_1;
      break;
    case C_KL_HIST: // Запрос истории
      if ((pASDUhead->num == 1) && ((pAPCI->APDULength - 4) == sizeof(ASDU_C_KL_HIST)))
        return C_KL_HIST;
      break;
    /*case C_KL_STR: // Команда записи строки с меткой времени CP56Время2а
      if ((pASDUhead->num == 1) && ((pAPCI->APDULength - 4) >= sizeof(ASDU_C_KL_STR)))
        return C_KL_STR;
      break;*/
    case C_KL_PROXY: // Проксирование обмена по протоколу KLogic
      if ((pASDUhead->num == 1) && ((pAPCI->APDULength - 4) >= (int)sizeof(ASDU_C_KL_PROXY)))
        return C_KL_PROXY;
      break;
    default:
      return ASDU_UNKNOUN; // неподдерживаемый тип ASDU
  }
  return ASDU_ERROR; // некорректный ASDU
}
//---------------------------------------------------------------------------

bool IsAsduNeedConfirm(TASDU AsduType)
{
  return (((AsduType >= 45) && (AsduType <= 69)) ||
          ((AsduType >= 100) && (AsduType <= 109)) ||
          ((AsduType >= 110) && (AsduType <= 119)));
}
//---------------------------------------------------------------------------

// -----------------------------------------------------------
// PrepareASDU
// -----------------------------------------------------------
bool PrepareASDU(KLPARAM* pKLPARAM, const KSDateTime& TimeStamp, DWORD IecAddr, BYTE Cause, char* ASDUbuf, char* ASDUsize, BYTE CommonAsduAddr)
{
  ASDU_HEAD* pASDUheader = (ASDU_HEAD*)&ASDUbuf[0];
  IOA* pParamAddr = (IOA*)&ASDUbuf[sizeof(ASDU_HEAD)];
  iec_type36* piec_type36 = (iec_type36*)(&ASDUbuf[sizeof(ASDU_HEAD) + sizeof(IOA)]);
  iec_type30* piec_type30 = (iec_type30*)piec_type36;

  // Заголовок
  switch (pKLPARAM->Type) // присваиваем тип параметра
  {
    case KLIOTYPE_BOOLEAN:
      pASDUheader->type = M_SP_TB_1;
      break;
    default:
      pASDUheader->type = M_ME_TF_1;
  }

  pASDUheader->num = 1; // 1 параметр в одном ASDU
  pASDUheader->sq = 0; // у каждого параметра свой адрес
  pASDUheader->cause = Cause; // причина передачи
  pASDUheader->pn = 0; // положительное подтверждение
  pASDUheader->t = 0; // не тест
  pASDUheader->cause_h = 0;
  pASDUheader->ca = CommonAsduAddr; // мл. байт - общий адрес ASDU
  pASDUheader->ca_h = 0; // ст. байт - 0

  // Адрес параметра
  pParamAddr->ioa = IecAddr & 0xFFFF;
  pParamAddr->ioa_h = IecAddr >> 16;

  // Сам параметр
  switch (pKLPARAM->Type)
  {
    case KLIOTYPE_BOOLEAN:
      piec_type30->sp = pKLPARAM->Value.Boolean;
      piec_type30->res = 0;  // резерв
      piec_type30->bl = 0; // not blocked
      piec_type30->sb = 0; // not substituted
      piec_type30->nt = 0; // актуальное значение
      piec_type30->iv = pKLPARAM->Quality == KLPARAM_QUALITY_GOOD ? 0:1; // valid/invalid
      DateTimeToCP56Time2a(TimeStamp, IsDayLight(), piec_type30->time);
      *ASDUsize = sizeof(ASDU_HEAD) + sizeof(IOA) + sizeof(iec_type30);

      break;
    default:
      switch(pKLPARAM->Type)
      {
        case KLIOTYPE_FLOAT:
        case KLIOTYPE_INTEGER:
          piec_type36->mv = pKLPARAM->Type == KLIOTYPE_FLOAT ? pKLPARAM->Value.Float : pKLPARAM->Value.Integer;
          break;
        case KLIOTYPE_DATETIME:
          memcpy(&piec_type36->mv, &pKLPARAM->Value.DateTime, sizeof(piec_type36->mv));
          break;
        default:
          memcpy(&piec_type36->mv, &pKLPARAM->Value.DateTime, sizeof(piec_type36->mv));
      }
      piec_type36->ov = 0; // no overflow
      piec_type36->res = 0;  // резерв
      piec_type36->bl = 0; // not blocked
      piec_type36->sb = 0; // not substituted
      piec_type36->nt = 0; // актуальное значение
      piec_type36->iv = pKLPARAM->Quality == KLPARAM_QUALITY_GOOD ? 0:1; // valid/invalid
      DateTimeToCP56Time2a(TimeStamp, IsDayLight(), piec_type36->time);
      *ASDUsize = sizeof(ASDU_HEAD) + sizeof(IOA) + sizeof(iec_type36);
      break;
  }

  return 1;
}
//---------------------------------------------------------------------------

