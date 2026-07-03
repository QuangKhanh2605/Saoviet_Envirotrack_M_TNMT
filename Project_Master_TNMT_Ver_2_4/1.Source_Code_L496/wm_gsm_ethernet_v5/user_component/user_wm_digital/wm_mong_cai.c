


#include "wm_mong_cai.h"


/*Bang thanh ghi modbus*/
sMONG_CAIRegList sMONG_CAI_REGISTER[] = 
{
           //param           Register(2byte)     format
    {   __MONG_CAI_FLOW,               8,      __MONG_CAI_FLOAT},
};


/*============= Function ==================*/
uint8_t MONG_CAI_Decode (uint8_t ireg, sData *pData, void *starget)
{
    sMongCaiData *psMongCai = starget;
    MONG_CAI_Extract_Data (pData, __MONG_CAI_FLOW, &psMongCai->Flow_f);  
    
    return true;
}



void MONG_CAI_Extract_Data (sData *pSource, uint8_t Param, void *data)
{
    uint8_t aTEMP[4] = {0};
    uint32_t TempU32 = 0;
    
    if (pSource->Length_u16 < sMONG_CAI_REGISTER[Param].Addr * 2 + 4)
    {        
        UTIL_Printf_Str (DBLEVEL_M, "mong_cai: not value\r\n" );
        
        return ;
    }
    
    switch (sMONG_CAI_REGISTER[Param].Format_u8)
    {
        case __MONG_CAI_FLOAT:
            for (uint8_t i = 0; i < 4; i++)
                aTEMP[i] = *(pSource->Data_a8 + sMONG_CAI_REGISTER[Param].Addr * 2 + i);
            
            TempU32 = aTEMP[2] << 24 | aTEMP[3] << 16 | aTEMP[0] << 8 | aTEMP[1];

            *(float *)data = Convert_FloatPoint_2Float(TempU32);
            break;
        default:
            break;
    }   
}




