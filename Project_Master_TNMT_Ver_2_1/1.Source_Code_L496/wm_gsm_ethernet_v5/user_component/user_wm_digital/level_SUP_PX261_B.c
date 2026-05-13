


#include "level_SUP_PX261_B.h"


uint8_t Level_SUP_Decode (uint8_t ireg, sData *pData, void *starget)
{
    uint16_t TempU16 = 0;   
    sLevelSupParameter *sLevel = starget;
    
    sLevel->Unit_u16 = *(pData->Data_a8 + SUP_REG_UNIT * 2) * 256 + *(pData->Data_a8 + SUP_REG_UNIT * 2 + 1);
    sLevel->Decimal_u16 = *(pData->Data_a8 + SUP_REG_DECIMAL * 2) * 256 + *(pData->Data_a8 + SUP_REG_DECIMAL * 2 + 1);
    
    TempU16 = *(pData->Data_a8 + SUP_REG_VALUE * 2) * 256 + *(pData->Data_a8 + SUP_REG_VALUE * 2 + 1);
    sLevel->Value_i16 = (int16_t) TempU16;
    
    switch (sLevel->Unit_u16)
    {
        case 0:
        default:
            sLevel->Unit_u16 = 1;
            break;
    }
    
    return true;
}




uint32_t Level_SUP_Reverse_Decimal (uint16_t Decimal)
{
    uint32_t Result = 1;
    
    for (uint16_t i = 0; i < Decimal; i++)
        Result *= 10;
    
    return Result;
}

/*
    Func: Convert gia tri calib tu mm -> Value with decimal and Unit curent
*/
int16_t Level_SUP_Convert_Value_Calib (int16_t Calib_mm, uint16_t Decimal, uint16_t Unit)
{
    int16_t CalibVal = 0;
    
    CalibVal = Level_SUP_Reverse_Decimal (Decimal) * Calib_mm;
        
    if (Unit == 1)
    {
        CalibVal = CalibVal / 10;
    } 
    
    return CalibVal;
}





