

#ifndef _WM_SENCE_H
#define _WM_SENCE_H


#include "user_util.h"

typedef enum
{
    __SEN_FM_USHORT,
    __SEN_FM_LONG,
    __SEN_FM_FLOAT,
}eSENCEFormat;


typedef enum
{
    __SENCE_FLOW,
    
    __SENCE_CUM_TOT,
    __SENCE_CUM_POS,  
    __SENCE_CUM_NEG,
    
    __SENCE_FLOW_UNIT,
    __SENCE_TOTAL_UNIT,

}sSENCEParameter;


typedef struct
{
    uint8_t         Param_u8;
    uint16_t        Addr;
    eSENCEFormat    Format_u8;        
}sSENCERegList;


typedef struct
{
    float   Flow_f;

    float   Total_f;
    float   Pos_f;
    float   Neg_f;
    
    uint16_t FlowUnit_u8;
    uint16_t TotalUnit_u8;
}sSENCEData;

/*=========== Function =========*/
uint8_t SENCE_Decode (uint8_t ireg, sData *pData, void *starget);
void SENCE_Extract_Data (sData *pSource, uint8_t Param, void *data);

char *SENCE_Decode_Unit (uint8_t type, uint8_t unit);
float SENCE_Convert_Flow_m3h (float val);

#endif




