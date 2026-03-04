


#include "woteck_ultrasonic.h"
#include "user_obis_sv.h"
#include "user_json.h"


/*
    luu y ve cach chuyen doi so lieu (trang so 10 cua tai lieu)
        + flow=(N+Nf )×10n-3
            + N: 4 byte interger
            + Nf: phan thap phan
            + n: he so nhan: thanh ghi 1439
            + don vi se lay ơ thanh ghi 1438

        01 03 04 06 51 3F 9E 3B 32（ hexadecimal digits）
            Equipment number function data byte data＝1.234567 checksum
            Four bytes of 3F 9E 06 51 is IEEE754 format float floating point of 1.2345678

    another example,read net totaliser flow,two registers of REG25,REG26,command as follows:
    01 03 00 18 00 02 44 0C（ hexadecimal digits）
    returned datas should be(set net totaliser＝802609,its 4 byte hexadecimal digits is 00 0C 3F 31)
    01 03 04 3F 31 00 0C A7 ED（ hexadecimal digits， A7 ED is
    checksum）


 */

/*Bang than ghi modbus*/
sWoteckUltraRegList sWOTECK_ULTRA_REGISTER[] = 
{
           //param           Register(2byte)     format
    {   __WO_ULTRA_FLOW,            0,      __WO_FLOAT},
    
    {   __WO_ULTRA_POS_INT,         8,      __WO_LONG},
    {   __WO_ULTRA_POS_DEC,         10,     __WO_FLOAT},
    {   __WO_ULTRA_NEG_INT,         12,     __WO_LONG},
    {   __WO_ULTRA_NEG_DEC,         14,     __WO_FLOAT},
    {   __WO_ULTRA_TOT_INT,         24,     __WO_LONG},
    {   __WO_ULTRA_TOT_DEC,         26,     __WO_FLOAT},
    
    {   __WO_ULTRA_FLOW_UNIT,       1436,   __WO_USHORT},
    {   __WO_ULTRA_TOTAL_UNIT,      1437,   __WO_USHORT}, 
    {   __WO_ULTRA_MULTI,           1438,   __WO_USHORT}, 
};


//sWoteckUltraData   sWoteckUltra;

const char WOTECK_ULTRA_UNIT_FLOW [32][10] = 
{
    {" (m3/s)"}, {" (m3/m)"}, {" (m3/h)"}, {" (m3/d)"},
    {" (L/s)"},  {" (L/m)"},  {" (L/h)"},  {" (L/d)"},
    {" (GAL/s)"},{" (GAL/m)"},{" (GAL/h)"},{" (GAL/d)"},
    {" (IGL/s)"},{" (IGL/m)"},{" (IGL/h)"},{" (IGL/d)"},
    {" (MGL/s)"},{" (MGL/m)"},{" (MGL/h)"},{" (MGL/d)"},
    
    {" (CF/s)"}, {" (CF/m)"}, {" (CF/h)"}, {" (CF/d)"},
    {" (OB/s)"}, {" (OB/m)"}, {" (OB/h)"}, {" (OB/d)"},
    {" (IB/s)"}, {" (IB/m)"}, {" (IB/h)"}, {" (IB/d)"},
};


const char WOTECK_ULTRA_UNIT_CUM [8][10] = 
{
    {" (m3)"},
    {" (L)"},
    {" (GAL)"},
    {" (IGL)"},
    {" (MGL)"},
    {" (CF)"},
    {" (OB)"},
    {" (IB)"},
};


static uint8_t REG_LIST[10] = {
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
};

/*============= Function ==================*/
uint8_t WOTECK_ULTRA_Decode (uint8_t ireg, sData *pData, void *starget)
{
    uint8_t result = pending;
    uint8_t reg = REG_LIST[ireg]; 
    sWoteckUltraData *sWoteckUltra = starget;
    
    switch (reg)
    {
        case __WO_ULTRA_FLOW:
            WOTECK_ULTRA_Extract_Data (pData, __WO_ULTRA_FLOW, &sWoteckUltra->Flow_f);  
            break;
        case __WO_ULTRA_POS_INT:
            WOTECK_ULTRA_Extract_Data (pData, __WO_ULTRA_POS_INT, &sWoteckUltra->PosInt_u32); 
            break;
        case __WO_ULTRA_POS_DEC:
            WOTECK_ULTRA_Extract_Data (pData, __WO_ULTRA_POS_DEC, &sWoteckUltra->PosDec_f);  
            break;
        case __WO_ULTRA_NEG_INT:
            WOTECK_ULTRA_Extract_Data (pData, __WO_ULTRA_NEG_INT, &sWoteckUltra->NegInt_u32); 
            break;
        case __WO_ULTRA_NEG_DEC:
            WOTECK_ULTRA_Extract_Data (pData, __WO_ULTRA_NEG_DEC, &sWoteckUltra->NegDec_f); 
            break;
        case __WO_ULTRA_TOT_INT:
            WOTECK_ULTRA_Extract_Data (pData, __WO_ULTRA_NEG_INT, &sWoteckUltra->TotInt_u32); 
            break;
        case __WO_ULTRA_TOT_DEC:
            WOTECK_ULTRA_Extract_Data (pData, __WO_ULTRA_NEG_DEC, &sWoteckUltra->TotDec_f); 
            break;
        case __WO_ULTRA_FLOW_UNIT:
            WOTECK_ULTRA_Extract_Data (pData, __WO_ULTRA_FLOW_UNIT, &sWoteckUltra->FlowUnit_u16); 
            break;
        case __WO_ULTRA_TOTAL_UNIT:
            WOTECK_ULTRA_Extract_Data (pData, __WO_ULTRA_TOTAL_UNIT, &sWoteckUltra->TotalUnit_u16); 
            break;
        case __WO_ULTRA_MULTI:
            WOTECK_ULTRA_Extract_Data (pData, __WO_ULTRA_TOTAL_UNIT, &sWoteckUltra->Multi_u16); 
            result = true;
            break;
        default:
            break;
    }
    
    return result;
}



void WOTECK_ULTRA_Extract_Data (sData *pSource, uint8_t Param, void *data)
{
    uint8_t aTEMP[4] = {0};
    uint32_t TempU32 = 0;
    uint16_t TempU16 = 0;
    
    if (pSource->Length_u16 < 4) {
        UTIL_Printf_Str (DBLEVEL_M, "wm_wo_ultra: not value\r\n" );
        return ;
    }
    
    switch (sWOTECK_ULTRA_REGISTER[Param].Format_u8)
    {
        case __WO_USHORT:
            for (uint8_t i = 0; i < 2; i++)
                aTEMP[i] = *(pSource->Data_a8 + i);
            
            TempU16 = aTEMP[0] << 8 | aTEMP[1];
                
            *(uint16_t *) data = ( TempU16 ); 
            break;
        case __WO_LONG:
            for (uint8_t i = 0; i < 4; i++)
                aTEMP[i] = *(pSource->Data_a8 + i);
            
            TempU32 = aTEMP[2] << 24 | aTEMP[3] << 16 | aTEMP[0] << 8 | aTEMP[1];

            *(int32_t *) data = (int32_t) TempU32; 
            break;
        case __WO_FLOAT:
            for (uint8_t i = 0; i < 4; i++)
                aTEMP[i] = *(pSource->Data_a8 + i);
            
            TempU32 = aTEMP[2] << 24 | aTEMP[3] << 16 | aTEMP[0] << 8 | aTEMP[1];

            *(float *)data = Convert_FloatPoint_2Float(TempU32);
            break;
        default:
            break;
    }   
}

/*
    Func: decode unit meter WO_ULTRA
        type: 0: flow;  1: cumulative
*/
char *WO_ULTRA_Decode_Unit (uint8_t type, uint8_t unit)
{
    char *pUnit = NULL;
    
    if (type == 0) {
        if (unit < 8) {
            pUnit = (char *) WOTECK_ULTRA_UNIT_FLOW[unit];
        }
    } else {
        if (unit < 8) {
            pUnit = (char *) WOTECK_ULTRA_UNIT_CUM[unit];
        }
    }
    
    return pUnit;
}

//  {" (m3/s)"}, {" (m3/m)"}, {" (m3/h)"}, {" (m3/d)"},
// {" (L/s)"},  {" (L/m)"},  {" (L/h)"},  {" (L/d)"},
float WO_ULTRA_Convert_Flow_m3h (uint8_t unit)
{
    float result = 0;
    
    switch (unit)
    {
        case 0:  //m3/s
            result = 1 * 3600;
            break;
        case 1:  //m3/m
            result = 1 * 60;
            break;
        case 2:  //m3/h
            result = 1;
            break;
        case 3:  //m3/d
            result = 1 / 24;
            break;
        case 4: //L/s
            result = 1 * 3600 / 1000;
            break;
        case 5: // L/m
            result = 1 * 60 / 1000;
            break;
        case 6:  // L/h
            result = 1 / 1000;
            break;
        case 7: // L/d
            result = 1 / 1000 / 24;
            break;
        default:
            break;
    }
    
    return result;
}


uint8_t WOTECK_ULTRA_Get_Reg (uint8_t index, uint16_t *addr, uint8_t *nReg)
{
    uint8_t reg = REG_LIST[index];
    
//    if (index < MaxReg) {
        *addr = sWOTECK_ULTRA_REGISTER[reg].Addr;
        *nReg = 2;
//        return true;
//    }
//    
    return false;
}











