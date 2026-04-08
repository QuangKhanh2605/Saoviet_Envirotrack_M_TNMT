


#include "siemen_mag8000.h"
#include "math.h"
/*
    float:  45 61
             0000
                    = 3600
 */

/*Bang than ghi modbus*/
sMag8000RegList sMAG8000_REGISTER[] = 
{
           //param           Register(2byte)    nreg        format
    {   __MAG_8000_FLOW_F,          206,        2,          __MAG_FLOAT     },
    {   __MAG_8000_TOTAL_F,         208,        2,          __MAG_FLOAT     },
    {   __MAG_8000_FLOW_U,          210,        6,          __MAG_STRING    },
    {   __MAG_8000_TOTAL_U,         216,        6,          __MAG_STRING    },
    
    {   __MAG_8000_TOTAL_SET1,      400,        1,          __MAG_USHORT    },
    {   __MAG_8000_TOTAL_SET2,      401,        1,          __MAG_USHORT    },
    
    {   __MAG_8000_PIN_PER,         3030,       1,          __MAG_USHORT   },
    
    {   __MAG_8000_FLOW,            3002,       2,          __MAG_FLOAT     },
    {   __MAG_8000_TOTAL_1,         3017,       4,          __MAG_TT_TYPE   },
    {   __MAG_8000_TOTAL_2,         3021,       4,          __MAG_TT_TYPE   },
};



static uint8_t REG_LIST[10] = {
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
};

/*============= Function ==================*/
uint8_t MAG8000_Decode (uint8_t ireg, sData *pData, void *starget)
{
    uint8_t result = pending, mark = false;
    uint8_t reg = REG_LIST[ireg]; 
    sMag8000Data *pMag8000 = starget;
    
    switch (reg)
    {
        case __MAG_8000_FLOW_F:
            MAG8000_Extract_Data (pData, reg, &pMag8000->FlowUnitFactor_f); 
            break;   
        case __MAG_8000_TOTAL_F:
            MAG8000_Extract_Data (pData, reg, &pMag8000->TotalUnitFactor_f); 
            break; 
        case __MAG_8000_FLOW_U:
            UTIL_MEM_set(pMag8000->aFLOW_UNIT, 0, sizeof(pMag8000->aFLOW_UNIT));
            MAG8000_Extract_Data (pData, reg, &pMag8000->aFLOW_UNIT); 
            break; 
        case __MAG_8000_TOTAL_U:
            UTIL_MEM_set(pMag8000->aTOTAL_UNIT, 0, sizeof(pMag8000->aTOTAL_UNIT));
            MAG8000_Extract_Data (pData, reg, &pMag8000->aTOTAL_UNIT); 
            break; 
        
        case __MAG_8000_TOTAL_SET1:
            MAG8000_Extract_Data (pData, reg, &pMag8000->TotalSetup1_u16); 
            break; 
        case __MAG_8000_TOTAL_SET2:
            MAG8000_Extract_Data (pData, reg, &pMag8000->TotalSetup2_u16); 
            break; 
        case __MAG_8000_PIN_PER:
            MAG8000_Extract_Data (pData, reg, &pMag8000->PinPercent_u16); 
            pMag8000->PinPercent_u16 = MIN((uint8_t) (pMag8000->PinPercent_u16), 100);
            break; 
              
        case __MAG_8000_FLOW:
            MAG8000_Extract_Data (pData, reg, &pMag8000->Flow_f); 
            break;
        case __MAG_8000_TOTAL_1:
            MAG8000_Extract_Data (pData, reg, &pMag8000->Total1_f); 
            break;
        case __MAG_8000_TOTAL_2:
            MAG8000_Extract_Data (pData, reg, &pMag8000->Total2_f); 
            
            pMag8000->Net_f = 0;
            switch (pMag8000->TotalSetup1_u16)
            {
                case 0:
                    pMag8000->Forward_f = pMag8000->Total1_f;
                    break;
                case 1:
                    pMag8000->Revert_f = pMag8000->Total1_f;
                    break;
                case 2:
                    mark = true;
                    pMag8000->Net_f = pMag8000->Total1_f;
                    break;
                default:
                    break;
            }
                   
            switch (pMag8000->TotalSetup2_u16)
            {
                case 0:
                    pMag8000->Forward_f = pMag8000->Total2_f;
                    break;
                case 1:
                    pMag8000->Revert_f = pMag8000->Total2_f;
                    break;
                case 2:
                    mark = true;
                    pMag8000->Net_f = pMag8000->Total2_f;
                    break;
                default:
                    break;
            }
            
            if (mark == false) {
                pMag8000->Net_f = pMag8000->Forward_f + pMag8000->Revert_f;
            }
            
            result = true;
            break;
        default:
            break;
    }
    
    return result;
}



void MAG8000_Extract_Data (sData *pSource, uint8_t Param, void *data)
{
    uint8_t aTEMP[4] = {0};
    uint32_t TempU32 = 0;
    uint16_t TempU16 = 0;
    float Tempf = 0, TempDecF = 0;
    
    if (pSource->Length_u16 < 4) {
        UTIL_Printf_Str (DBLEVEL_M, "wm_wo_ultra: not value\r\n" );
        return ;
    }
    
    switch (sMAG8000_REGISTER[Param].Format_u8)
    {
        case __MAG_USHORT:
            for (uint8_t i = 0; i < 2; i++)
                aTEMP[i] = *(pSource->Data_a8 + i);
            
            TempU16 = aTEMP[0] << 8 | aTEMP[1];
                
            *(uint16_t *) data = ( TempU16 ); 
            break;
        case __MAG_LONG:
            for (uint8_t i = 0; i < 4; i++)
                aTEMP[i] = *(pSource->Data_a8 + i);
            
            TempU32 = aTEMP[0] << 24 | aTEMP[1] << 16 | aTEMP[2] << 8 | aTEMP[3];

            *(int32_t *) data = (int32_t) TempU32; 
            break;
        case __MAG_FLOAT:
            for (uint8_t i = 0; i < 4; i++)
                aTEMP[i] = *(pSource->Data_a8 + i);
            
            TempU32 = aTEMP[0] << 24 | aTEMP[1] << 16 | aTEMP[2] << 8 | aTEMP[3];

            *(float *)data = Convert_FloatPoint_2Float(TempU32);
            break;
        case __MAG_STRING:
            aTEMP[0] = strlen((char *)pSource->Data_a8);
            
            if ( (aTEMP[0] > 0) && (aTEMP[0] < 10) ) {
                sprintf((char *) data, " (%s)", (char *) pSource->Data_a8);
            }
            break;
        case __MAG_TT_TYPE:
            for (uint8_t i = 0; i < 4; i++)
                aTEMP[i] = *(pSource->Data_a8 + i);
            
            TempU32 = aTEMP[0] << 24 | aTEMP[1] << 16 | aTEMP[2] << 8 | aTEMP[3];

            Tempf = (int32_t) TempU32; 
            
            for (uint8_t i = 4; i < 8; i++)
                aTEMP[i] = *(pSource->Data_a8 + i);
            
            TempU32 = aTEMP[0] << 24 | aTEMP[1] << 16 | aTEMP[2] << 8 | aTEMP[3];
            
            TempDecF = (int32_t) TempU32; 
            TempDecF = TempDecF / pow (10, 9);
            
            Tempf += TempDecF;  
            *(float *)data = Tempf;
            break;
        default:
            break;
    }   
}

///*
//    Func: decode unit meter MAG_8000
//        type: 0: flow;  1: cumulative
//*/
//char *MAG_8000_Decode_Unit (uint8_t type, uint8_t unit)
//{
//    char *pUnit = NULL;
//    
//    if (type == 0) {
//        if (unit < 8) {
//            pUnit = (char *) MAG8000_UNIT_FLOW[unit];
//        }
//    } else {
//        if (unit < 8) {
//            pUnit = (char *) MAG8000_UNIT_CUM[unit];
//        }
//    }
//    
//    return pUnit;
//}


float MAG_8000_Convert_Flow_m3h (uint8_t unit)
{
    float result = 0;
    
    return result;
}


uint8_t MAG8000_Get_Reg (uint8_t index, uint16_t *addr, uint8_t *nReg)
{
    uint8_t reg = REG_LIST[index];
    
//    if (index < MaxReg) {
        *addr = sMAG8000_REGISTER[reg].Addr;
        *nReg = sMAG8000_REGISTER[reg].nreg_u8;
//        return true;
//    }
//    
    return false;
}











