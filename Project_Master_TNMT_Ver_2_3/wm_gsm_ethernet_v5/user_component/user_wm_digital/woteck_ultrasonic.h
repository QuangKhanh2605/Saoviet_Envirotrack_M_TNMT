

#ifndef WOTECK_ULTRA_H
#define WOTECK_ULTRA_H


#include "user_util.h"

typedef enum
{
    __WO_USHORT,
    __WO_LONG,
    __WO_FLOAT,
}eWoteckUltraFormat;


typedef enum
{
    __WO_ULTRA_FLOW,
    
    __WO_ULTRA_POS_INT,
    __WO_ULTRA_POS_DEC,  
    __WO_ULTRA_NEG_INT,
    __WO_ULTRA_NEG_DEC,
    __WO_ULTRA_TOT_INT,
    __WO_ULTRA_TOT_DEC,
    
    __WO_ULTRA_FLOW_UNIT,
    __WO_ULTRA_TOTAL_UNIT,
    __WO_ULTRA_MULTI,
    __WO_ULTRA_END,
}sWoteckUltraParameter;


typedef struct
{
    uint8_t             Param_u8;
    uint16_t            Addr;
    eWoteckUltraFormat  Format_u8;        
}sWoteckUltraRegList;


typedef struct
{
    float   Flow_f;
    
    uint32_t PosInt_u32;
    float   PosDec_f;
    
    uint32_t NegInt_u32;
    float   NegDec_f;
    
    uint32_t TotInt_u32;
    float   TotDec_f;
    
    uint16_t FlowUnit_u16;
    uint16_t TotalUnit_u16;
    uint16_t Multi_u16;
}sWoteckUltraData;

extern sWoteckUltraRegList sWOTECK_ULTRA_REGISTER[];

/*=========== Function =========*/
uint8_t WOTECK_ULTRA_Decode (uint8_t ireg, sData *pData, void *starget);
void    WOTECK_ULTRA_Extract_Data (sData *pSource, uint8_t Param, void *data);

char*   WO_ULTRA_Decode_Unit (uint8_t type, uint8_t unit);
float   WO_ULTRA_Convert_Flow_m3h (uint8_t unit);

uint8_t WOTECK_ULTRA_Get_Reg (uint8_t index, uint16_t *addr, uint8_t *nReg);


#endif




