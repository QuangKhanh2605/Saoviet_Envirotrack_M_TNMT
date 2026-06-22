


#include "level_ultrasonic.h"




uint8_t Level_Ultra_Decode (uint8_t ireg, sData *pData, void *starget)
{
    sLevelUltrasonic *sLevel = starget;
        
    sLevel->Unit_u16 = 1;
    sLevel->Decimal_u16 = 1;

    sLevel->Value_i16 = *(pData->Data_a8 + REGIS_REALTIME_VAL * 2) * 256    \
                            + *(pData->Data_a8 + REGIS_REALTIME_VAL * 2 + 1);

    sLevel->ZeroPoint_i16 = 0;
    
    return true;
}




