


#include "wm_mt100.h"
#include "user_app_wm.h"
#include "user_obis_sv.h"
#include "user_json.h"

/*
    Float format：
     
    The IEEE754 format is used for the 4-bytes float data as following:
            Register 1                       Register 2
        BYTE1           BYTE2           BYTE3       BYTE4
        S EEEEEEE      E MMMMMMM      MMMMMMMM      MMMMMMMM
 */

/*Bang than ghi modbus*/
sMT100RegList sMT100_REGISTER[] = 
{
           //param           Register(2byte)     format
    {   __MT100_FLOW,               0,      __FM_FLOAT},
    
    {   __MT100_POS_INT,            8,      __FM_LONG},
    {   __MT100_POS_DEC,            10,     __FM_FLOAT},
    {   __MT100_NEG_INT,            12,     __FM_LONG},
    {   __MT100_NEG_DEC,            14,     __FM_FLOAT},
    
    {   __MT100_FLOW_UNIT,          16,     __FM_USHORT},
    {   __MT100_TOTAL_UNIT,         17,     __FM_USHORT}, 
};


const char MT100_UNIT_FLOW_1[6][10] = 
{
    {" (l/s)"},
    {" (l/m)"},
    {" (l/h)"},
    {" (m3/s)"},
    {" (m3/m)"},
    {" (m3/h)"},
};

const char MT100_UNIT_CUM_1[2][10] = 
{
    {" (l)"},
    {" (m3)"},
};

/*============= Function ==================*/
uint8_t MT100_Decode (uint8_t ireg, sData *pData, void *starget)
{
    sMT100Data  *sMT100 = starget;
    
    MT100_Extract_Data (pData, __MT100_FLOW, &sMT100->Flow_f);  
    
    MT100_Extract_Data (pData, __MT100_POS_INT, &sMT100->PosInt_i32);  
    MT100_Extract_Data (pData, __MT100_POS_DEC, &sMT100->PosDec_f);  
    
    MT100_Extract_Data (pData, __MT100_NEG_INT, &sMT100->NegInt_i32);  
    MT100_Extract_Data (pData, __MT100_NEG_DEC, &sMT100->NegDec_f);  
    
    MT100_Extract_Data (pData, __MT100_FLOW_UNIT, &sMT100->FlowUnit_u8);  
    MT100_Extract_Data (pData, __MT100_TOTAL_UNIT, &sMT100->TotalUnit_u8); 
    
    return true;
}



void MT100_Extract_Data (sData *pSource, uint8_t Param, void *data)
{
    uint8_t aTEMP[4] = {0};
    uint32_t TempU32 = 0;
    uint16_t TempU16 = 0;
    
    if (pSource->Length_u16 < sMT100_REGISTER[Param].Addr * 2 + 4)
    {        
        UTIL_Printf_Str (DBLEVEL_M, "wm_mt100: not value\r\n" );
        
        return ;
    }
    
    switch (sMT100_REGISTER[Param].Format_u8)
    {
        case __FM_USHORT:
            for (uint8_t i = 0; i < 2; i++)
                aTEMP[i] = *(pSource->Data_a8 + sMT100_REGISTER[Param].Addr * 2 + i);
            
            TempU16 = aTEMP[0] << 8 | aTEMP[1];
                
            *(uint16_t *) data = ( TempU16 ); 
            break;
        case __FM_LONG:
            for (uint8_t i = 0; i < 4; i++)
                aTEMP[i] = *(pSource->Data_a8 + sMT100_REGISTER[Param].Addr * 2 + i);
            
            TempU32 = aTEMP[0] << 24 | aTEMP[1] << 16 | aTEMP[2] << 8 | aTEMP[3];

            *(int32_t *) data = (int32_t) TempU32; 
            break;
        case __FM_FLOAT:
            for (uint8_t i = 0; i < 4; i++)
                aTEMP[i] = *(pSource->Data_a8 + sMT100_REGISTER[Param].Addr * 2 + i);
            
            TempU32 = aTEMP[0] << 24 | aTEMP[1] << 16 | aTEMP[2] << 8 | aTEMP[3];

            *(float *)data = Convert_FloatPoint_2Float(TempU32);
            break;
        default:
            break;
    }   
}

/*
    Func: decode unit meter MT100
        type: 0: flow;  1: cumulative
*/
char *MT100_Decode_Unit (uint8_t type, uint8_t unit)
{
    char *pUnit = NULL;
    
    if (type == 0) {
        if (unit < 6) {
            pUnit = (char *) MT100_UNIT_FLOW_1[unit];
        }
    } else {
        if (unit < 2) {
            pUnit = (char *) MT100_UNIT_CUM_1[unit];
        }
    }
    
    return pUnit;
}


float MT100_Convert_Flow_m3h (uint8_t unit)
{
    float result = 0;
    
    switch (unit)
    {
        case 0:  //l/s -> m3/h
            result = 3600 / 1000;
            break;
        case 1:  //l/m
            result = 60 / 1000;
            break;
        case 2:  //l/h
            result = 1 / 1000;
            break;
        case 3:  //m3/s
            result = 1 * 3600;
            break;
        case 4: //m3/m
            result = 1 * 60;
            break;
        case 5: // m3/h
            result = 1;
            break;
        default:
            break;
    }
    
    return result;
}






