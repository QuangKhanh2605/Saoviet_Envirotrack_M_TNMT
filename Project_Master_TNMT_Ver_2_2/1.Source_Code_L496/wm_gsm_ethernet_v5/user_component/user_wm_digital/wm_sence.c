


#include "wm_sence.h"

/*
    Float format：
     
    The IEEE754 format is used for the 4-bytes float data as following:
            Register 1                       Register 2
        BYTE1           BYTE2           BYTE3       BYTE4
        S EEEEEEE      E MMMMMMM      MMMMMMMM      MMMMMMMM

    Read positive cumulative integer： 01 03 00 08 00 02 45 C9
        Lower computer return: 01h 03h 04H 00h d9h 23h 38h 33H 2ah
        00 D9 23 38 converted to decimal system 14230328, i.e. positive cumulative integer part
        
    Read positive cumulative decimal： 01 03 00 0A 00 01 A4 08
        Lower computer return: 01h 03h 02h 01h BCH b8h 65h
        01bc converted to decimal system is 444, decimal is 0.444

    Flow unit: 0: L / h, 1: L / m, 2: L / s, 3: m3 / h, 4: m3 / m, 5: m3 / S
    Flow totalizing unit: 0.001l, 1:0.01l, 2:0.1l, 3:1l, 4:0.001m3, 5:0.01m3, 6:0.1m3, 7:1m3
 */

/*Bang than ghi modbus*/
sSENCERegList sSENCE_REGISTER[] = 
{
           //param           Register(2byte)     format
    {   __SENCE_FLOW,               0,      __SEN_FM_LONG},
    
    {   __SENCE_CUM_TOT,            5,      __SEN_FM_FLOAT},
    {   __SENCE_CUM_POS,            8,      __SEN_FM_FLOAT},
    {   __SENCE_CUM_NEG,            11,     __SEN_FM_FLOAT},
    
    {   __SENCE_FLOW_UNIT,          21,     __SEN_FM_USHORT},
    {   __SENCE_TOTAL_UNIT,         22,     __SEN_FM_USHORT}, 
};


const char SENCE_UNIT_FLOW_1[6][10] = 
{
    {" (l/h)"},
    {" (l/m)"},
    {" (l/s)"},
    {" (m3/h)"},
    {" (m3/m)"},
    {" (m3/s)"},
};

const char SENCE_UNIT_CUM_1[2][10] =
{
    {" (l)"},
    {" (m3)"},
};


/*============= Function ==================*/
uint8_t SENCE_Decode (uint8_t ireg, sData *pData, void *starget)
{
    sSENCEData  *sSence = starget;
    
    SENCE_Extract_Data (pData, __SENCE_FLOW, &sSence->Flow_f);  
    
    SENCE_Extract_Data (pData, __SENCE_CUM_TOT, &sSence->Total_f);  
    SENCE_Extract_Data (pData, __SENCE_CUM_POS, &sSence->Pos_f);  
    SENCE_Extract_Data (pData, __SENCE_CUM_NEG, &sSence->Neg_f);  
    
    SENCE_Extract_Data (pData, __SENCE_FLOW_UNIT, &sSence->FlowUnit_u8);  
    SENCE_Extract_Data (pData, __SENCE_TOTAL_UNIT, &sSence->TotalUnit_u8); 

    return true;
}



void SENCE_Extract_Data (sData *pSource, uint8_t Param, void *data)
{
    uint8_t aTEMP[4] = {0}, i = 0;
    uint32_t TempU32 = 0;
    uint16_t TempU16 = 0;
    float TempFloat = 0;

    if (pSource->Length_u16 < sSENCE_REGISTER[Param].Addr * 2) {   
        UTIL_Printf_Str (DBLEVEL_M, "wm_mt100: not value\r\n" );
        
        return ;
    }
    
    switch (sSENCE_REGISTER[Param].Format_u8)
    {
        case __SEN_FM_USHORT:
            for (i = 0; i < 2; i++) {
                aTEMP[i] = *(pSource->Data_a8 + sSENCE_REGISTER[Param].Addr * 2 + i);
            }
            
            TempU16 = aTEMP[0] << 8 | aTEMP[1];
                
            *(uint16_t *) data = ( TempU16 ); 
            break;
        case __SEN_FM_LONG:
            //4 byte phan integer và 2 byte phan thap phan
            for (i = 0; i < 4; i++) {
                aTEMP[i] = *(pSource->Data_a8 + sSENCE_REGISTER[Param].Addr * 2 + i);
            }
            
            TempU32 = aTEMP[0] << 24 | aTEMP[1] << 16 | aTEMP[2] << 8 | aTEMP[3];
            
            TempFloat = TempU32;
            
            *(float *) data = ( TempFloat );
            break;
        case __SEN_FM_FLOAT:
            //4 byte phan integer và 2 byte phan thap phan
            for (i = 0; i < 4; i++) {
                aTEMP[i] = *(pSource->Data_a8 + sSENCE_REGISTER[Param].Addr * 2 + i);
            }
            
            TempU32 = aTEMP[0] << 24 | aTEMP[1] << 16 | aTEMP[2] << 8 | aTEMP[3];
    
            for (i = 0; i < 2; i++) {
                aTEMP[i] = *(pSource->Data_a8 + sSENCE_REGISTER[Param].Addr * 2 + 4 + i);
            }
            
            TempU16 = aTEMP[0] << 8 | aTEMP[1];
            
            TempFloat = UTIL_Convert_Decimal_Element(TempU16);
            TempFloat += TempU32;
            
            *(float *) data = ( TempFloat );
            break;
        default:
            break;
    }   
}

/*
    Func: decode unit meter SENCE
        type: 0: flow;  1: cumulative
*/
char *SENCE_Decode_Unit (uint8_t type, uint8_t unit)
{
    char *pUnit = NULL;
    
    if (type == 0) {
        if (unit < 6) {
            pUnit = (char *) SENCE_UNIT_FLOW_1[3];
        }
    } else {
        if (unit < 8) {
            switch (unit)
            {
                case 0:  
                case 1:
                case 2:
                case 3:
                    pUnit = (char *) SENCE_UNIT_CUM_1[0];
                    break;
                case 4:
                case 5:
                case 6:
                case 7:
                    pUnit = (char *) SENCE_UNIT_CUM_1[1];
                    break;
            }
        }
    }
  
    return pUnit;
}

/*
    Func: convert flow init to m3/h
*/
float SENCE_Convert_Flow_m3h (float val)
{
    float result = 0;

    result = val / 1000;
    
    return result;
}





