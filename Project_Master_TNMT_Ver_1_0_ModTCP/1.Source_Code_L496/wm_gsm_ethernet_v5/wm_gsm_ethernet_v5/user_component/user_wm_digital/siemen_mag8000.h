

#ifndef MAG_8000_H
#define MAG_8000_H


#include "user_util.h"

typedef enum
{
    __MAG_USHORT,
    __MAG_LONG,
    __MAG_FLOAT,
    __MAG_STRING,
    __MAG_TT_TYPE,
}eMag8000Format;


typedef enum
{
    __MAG_8000_FLOW_F,
    __MAG_8000_TOTAL_F,
    __MAG_8000_FLOW_U,
    __MAG_8000_TOTAL_U,
    
    __MAG_8000_TOTAL_SET1,
    __MAG_8000_TOTAL_SET2,
    
    __MAG_8000_PIN_PER,
    
    __MAG_8000_FLOW,
    __MAG_8000_TOTAL_1,
    __MAG_8000_TOTAL_2,
        
    __MAG_8000_END,
}sMag8000Parameter;


typedef struct
{
    uint8_t             Param_u8;
    uint16_t            Addr;
    uint16_t            nreg_u8;
    eMag8000Format      Format_u8;        
}sMag8000RegList;


typedef struct
{
    float   Flow_f;
    
    float   Total1_f;
    float   Total2_f;
    
    float   Forward_f;
    float   Revert_f;
    float   Net_f;
    
    uint16_t TotalSetup1_u16;
    uint16_t TotalSetup2_u16;
    
    uint16_t PinPercent_u16;
    
    char    aFLOW_UNIT[10];
    char    aTOTAL_UNIT[10];
    
    float   FlowUnitFactor_f;
    float   TotalUnitFactor_f;
}sMag8000Data;

extern sMag8000RegList sMAG8000_REGISTER[];

/*=========== Function =========*/
uint8_t MAG8000_Decode (uint8_t ireg, sData *pData, void *starget);
void    MAG8000_Extract_Data (sData *pSource, uint8_t Param, void *data);

char*   MAG_8000_Decode_Unit (uint8_t type, uint8_t unit);
float   MAG_8000_Convert_Flow_m3h (uint8_t unit);

uint8_t MAG8000_Get_Reg (uint8_t index, uint16_t *addr, uint8_t *nReg);


#endif




