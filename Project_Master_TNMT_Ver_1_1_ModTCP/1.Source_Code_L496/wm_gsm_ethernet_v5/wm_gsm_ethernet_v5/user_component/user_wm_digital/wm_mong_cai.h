

#ifndef MONG_CAI_H
#define MONG_CAI_H


#include "user_util.h"



typedef enum
{
    __MONG_CAI_FLOAT,
}eMongCaiFormat;


typedef enum
{
    __MONG_CAI_FLOW,
}sMongCaiParameter;


typedef struct
{
    uint8_t     Param_u8;
    uint16_t         Addr;
    eMongCaiFormat   Format_u8;        
}sMONG_CAIRegList;


typedef struct
{
    float   Flow_f;
}sMongCaiData;


/*=========== Function =========*/
uint8_t MONG_CAI_Decode (uint8_t ireg, sData *pData, void *starget);
void MONG_CAI_Extract_Data (sData *pSource, uint8_t Param, void *data);


#endif




