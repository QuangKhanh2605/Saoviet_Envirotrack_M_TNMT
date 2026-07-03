


#include "user_wm_digital.h"
#include "user_obis_sv.h"
#include "user_json.h"
#include "user_rs485.h"
#include "math.h"

sWMDigitalInfor sListWmDigital[10] = 
{
    {   __MET_WOTECK,       0,      120,    { 9600, UART_WORDLENGTH_8B, UART_STOPBITS_1, UART_PARITY_NONE },      Woteck_Decode     },  
    {   __MET_LEVEL_LIQ,    0,      6,      { 9600, UART_WORDLENGTH_8B, UART_STOPBITS_1, UART_PARITY_NONE },      Level_S_YW_Decode },
    {   __MET_LEVEL_LIQ_SUP,0,      6,      { 9600, UART_WORDLENGTH_8B, UART_STOPBITS_1, UART_PARITY_NONE },      Level_SUP_Decode  },
    {   __MET_MT100,        4112,   22,     { 9600, UART_WORDLENGTH_8B, UART_STOPBITS_1, UART_PARITY_NONE },      MT100_Decode      },
    {   __MET_LEVEL_ULTRA,  0x0100, 4,      { 9600, UART_WORDLENGTH_8B, UART_STOPBITS_1, UART_PARITY_NONE },      Level_Ultra_Decode},
    {   __MET_WM_SENCE,     0,      24,     { 9600, UART_WORDLENGTH_8B, UART_STOPBITS_1, UART_PARITY_NONE },      SENCE_Decode      },
    {   __MET_WOTECK_ULTRA, 0,      24,     { 9600, UART_WORDLENGTH_8B, UART_STOPBITS_1, UART_PARITY_NONE },      WOTECK_ULTRA_Decode},
    {   __MET_WM_MONG_CAI,  2000,   10,     { 9600, UART_WORDLENGTH_8B, UART_STOPBITS_1, UART_PARITY_NONE },      MONG_CAI_Decode   },
    {   __MET_SI_MAG8000,   0,      24,     { 19200, UART_WORDLENGTH_9B, UART_STOPBITS_1, UART_PARITY_EVEN },     MAG8000_Decode    },
    {   __MET_UNKNOWN,      0,      1,      { 9600, UART_WORDLENGTH_8B, UART_STOPBITS_1, UART_PARITY_NONE },      NULL              },
};


sWMDigitalVariable       sWmDigVar = 
{
    .nModbus_u8 = 1,
    .sModbInfor[0].MType_u8 = __MET_WOTECK,
    .sModbInfor[0].SlaveId_u8 = 0x01,
        
    .sModbDevData[0] =
    {
        .Status_u8 = false,
        .Type_u8 = 0,
        .nTotal_i64 = 234,
        .nForw_i64 = 5667,
        .nRev_i64 = 9789,
        
        .Pressure_i32 = -1,
        
        .Flow_i32 = 96,
        .Factor = 0xFD,
        .sFlowUnit = " (l/s)",
        .sTotalUnit = " (m3)",
    },
};


/*============= Function =============*/
void WM_DIG_Init_Uart (UART_HandleTypeDef *huart, uint8_t type)
{
    huart->Init.BaudRate = sListWmDigital[type].sUart.BaudRate;
    huart->Init.WordLength = sListWmDigital[type].sUart.WordLength;
    huart->Init.StopBits = sListWmDigital[type].sUart.StopBits;
    huart->Init.Parity = sListWmDigital[type].sUart.Parity;
    huart->Init.Mode = UART_MODE_TX_RX;
    huart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart->Init.OverSampling = UART_OVERSAMPLING_16;
    huart->Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart->AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if (HAL_UART_Init(huart) != HAL_OK)
    {
        Error_Handler();
    }
}




void WM_DIG_Get_Data (uint8_t chann, uint8_t type)
{
    float TempFloat = 0;
    float multi = 0;
    
    uint32_t Temp_u32 = 0;
    
    switch (type)
    {
        case __MET_WOTECK:
            sWmDigVar.sModbDevData[chann].Status_u8 = true;
            sWmDigVar.sModbDevData[chann].Type_u8 = type;
            
            sWmDigVar.sModbDevData[chann].nTotal_i64  = (int64_t ) (sWmDigVar.sModbDevData[chann].sWoteck01.TotalVal_f * 1000);
            sWmDigVar.sModbDevData[chann].nRev_i64    = (int64_t ) (sWmDigVar.sModbDevData[chann].sWoteck01.TotalRev_f * 1000);
            sWmDigVar.sModbDevData[chann].nForw_i64   = (int64_t ) (sWmDigVar.sModbDevData[chann].sWoteck01.TotalForw_f * 1000);
            sWmDigVar.sModbDevData[chann].Flow_i32    = (int32_t ) (sWmDigVar.sModbDevData[chann].sWoteck01.Flow_f * 1000);
            sWmDigVar.sModbDevData[chann].FlowM3_i32  = sWmDigVar.sModbDevData[chann].Flow_i32; 
                
            sWmDigVar.sModbDevData[chann].sFlowUnit   = " (m3/h)";
            sWmDigVar.sModbDevData[chann].sTotalUnit  = " (m3)";
            
            sWmDigVar.sModbDevData[chann].Factor      = 0xFD;
            
            break;
        case __MET_LEVEL_LIQ:
            sWmDigVar.sModbDevData[chann].Status_u8 = true;
            sWmDigVar.sModbDevData[chann].Type_u8 = type;

            sWmDigVar.sModbDevData[chann].LVal_i16  = sWmDigVar.sModbDevData[chann].sLevel01.Value_i16;
            sWmDigVar.sModbDevData[chann].LDecimal_u16 = sWmDigVar.sModbDevData[chann].sLevel01.Decimal_u16;
            sWmDigVar.sModbDevData[chann].LUnit_u16 = sWmDigVar.sModbDevData[chann].sLevel01.Unit_u16;
            sWmDigVar.sModbDevData[chann].PinPercent_u8 = 100;
            break;
            
        case __MET_LEVEL_LIQ_SUP:
            sWmDigVar.sModbDevData[chann].Status_u8 = true;
            sWmDigVar.sModbDevData[chann].Type_u8 = type;

            sWmDigVar.sModbDevData[chann].LVal_i16  = sWmDigVar.sModbDevData[chann].sLevelSup.Value_i16;
            sWmDigVar.sModbDevData[chann].LDecimal_u16 = sWmDigVar.sModbDevData[chann].sLevelSup.Decimal_u16;
            sWmDigVar.sModbDevData[chann].LUnit_u16 = sWmDigVar.sModbDevData[chann].sLevelSup.Unit_u16;
            break;
        case __MET_LEVEL_ULTRA:
            sWmDigVar.sModbDevData[chann].Status_u8 = true;
            sWmDigVar.sModbDevData[chann].Type_u8 = type;

            sWmDigVar.sModbDevData[chann].LVal_i16  = sWmDigVar.sModbDevData[chann].sLevelUltra.Value_i16;
            sWmDigVar.sModbDevData[chann].LDecimal_u16 = sWmDigVar.sModbDevData[chann].sLevelUltra.Decimal_u16;
            sWmDigVar.sModbDevData[chann].LUnit_u16 = sWmDigVar.sModbDevData[chann].sLevelUltra.Unit_u16;
            break;
            
        case __MET_MT100:
            sWmDigVar.sModbDevData[chann].Status_u8 = true;
            sWmDigVar.sModbDevData[chann].Type_u8 = type;

            TempFloat = (sWmDigVar.sModbDevData[chann].sMt100.PosInt_i32 + sWmDigVar.sModbDevData[chann].sMt100.PosDec_f) \
                        - (sWmDigVar.sModbDevData[chann].sMt100.NegInt_i32 + sWmDigVar.sModbDevData[chann].sMt100.NegDec_f);
            sWmDigVar.sModbDevData[chann].nTotal_i64   = (int64_t ) (TempFloat * 1000);
            
            TempFloat = sWmDigVar.sModbDevData[chann].sMt100.PosInt_i32 + sWmDigVar.sModbDevData[chann].sMt100.PosDec_f;
            sWmDigVar.sModbDevData[chann].nForw_i64   = (int64_t ) (TempFloat * 1000);
            
            TempFloat = sWmDigVar.sModbDevData[chann].sMt100.NegInt_i32 + sWmDigVar.sModbDevData[chann].sMt100.NegDec_f;
            sWmDigVar.sModbDevData[chann].nRev_i64    = (int64_t ) (TempFloat * 1000);
            
            sWmDigVar.sModbDevData[chann].Flow_i32    = (int32_t ) (sWmDigVar.sModbDevData[chann].sMt100.Flow_f * 1000);
            sWmDigVar.sModbDevData[chann].FlowM3_i32  = (int32_t ) (sWmDigVar.sModbDevData[chann].sMt100.Flow_f * 1000     \
                                                                     * MT100_Convert_Flow_m3h(sWmDigVar.sModbDevData[chann].sMt100.FlowUnit_u8)); 
            
            sWmDigVar.sModbDevData[chann].sFlowUnit   = MT100_Decode_Unit(0, sWmDigVar.sModbDevData[chann].sMt100.FlowUnit_u8);
            sWmDigVar.sModbDevData[chann].sTotalUnit  = MT100_Decode_Unit(1, sWmDigVar.sModbDevData[chann].sMt100.TotalUnit_u8);
            
            sWmDigVar.sModbDevData[chann].Factor      = 0xFD;
            break;
        case __MET_WM_SENCE:
            sWmDigVar.sModbDevData[chann].Status_u8 = true;
            sWmDigVar.sModbDevData[chann].Type_u8 = type;

            sWmDigVar.sModbDevData[chann].nTotal_i64   = (int64_t ) (sWmDigVar.sModbDevData[chann].sSence.Total_f * 1000);
            sWmDigVar.sModbDevData[chann].nForw_i64   = (uint64_t ) (sWmDigVar.sModbDevData[chann].sSence.Pos_f* 1000);
            sWmDigVar.sModbDevData[chann].nRev_i64    = (uint64_t ) (sWmDigVar.sModbDevData[chann].sSence.Neg_f * 1000);
            
            sWmDigVar.sModbDevData[chann].Flow_i32    = (int32_t ) sWmDigVar.sModbDevData[chann].sSence.Flow_f;
            sWmDigVar.sModbDevData[chann].FlowM3_i32  = sWmDigVar.sModbDevData[chann].Flow_i32; 
            
            sWmDigVar.sModbDevData[chann].sFlowUnit   = SENCE_Decode_Unit(0, sWmDigVar.sModbDevData[chann].sSence.FlowUnit_u8);
            sWmDigVar.sModbDevData[chann].sTotalUnit  = SENCE_Decode_Unit(1, sWmDigVar.sModbDevData[chann].sSence.TotalUnit_u8);
            
            sWmDigVar.sModbDevData[chann].Factor      = 0xFD;
            break;
        case __MET_WOTECK_ULTRA:
            sWmDigVar.sModbDevData[chann].Status_u8 = true;
            sWmDigVar.sModbDevData[chann].Type_u8 = type;

            multi = pow(10, sWmDigVar.sModbDevData[chann].sWoteckUltra.Multi_u16 - 3);
            
            TempFloat = sWmDigVar.sModbDevData[chann].sWoteckUltra.TotInt_u32 + sWmDigVar.sModbDevData[chann].sWoteckUltra.TotDec_f;
            TempFloat *= multi; 
            sWmDigVar.sModbDevData[chann].nTotal_i64   = (int64_t ) (TempFloat * 1000);
            
            TempFloat = sWmDigVar.sModbDevData[chann].sWoteckUltra.PosInt_u32 + sWmDigVar.sModbDevData[chann].sWoteckUltra.PosDec_f;
            TempFloat *= multi;
            sWmDigVar.sModbDevData[chann].nForw_i64   = (int64_t ) (TempFloat * 1000);
            
            TempFloat = sWmDigVar.sModbDevData[chann].sWoteckUltra.NegInt_u32 + sWmDigVar.sModbDevData[chann].sWoteckUltra.NegDec_f;
            TempFloat *= multi;
            sWmDigVar.sModbDevData[chann].nRev_i64    = (int64_t ) (TempFloat * 1000);
            
            sWmDigVar.sModbDevData[chann].Flow_i32    = (int32_t ) (sWmDigVar.sModbDevData[chann].sWoteckUltra.Flow_f * 1000);
            sWmDigVar.sModbDevData[chann].FlowM3_i32  = (uint32_t ) (sWmDigVar.sModbDevData[chann].sWoteckUltra.Flow_f * 1000
                                                                     * WO_ULTRA_Convert_Flow_m3h(sWmDigVar.sModbDevData[chann].sWoteckUltra.FlowUnit_u16));
            
            sWmDigVar.sModbDevData[chann].sFlowUnit   = WO_ULTRA_Decode_Unit(0, sWmDigVar.sModbDevData[chann].sWoteckUltra.FlowUnit_u16);
            sWmDigVar.sModbDevData[chann].sTotalUnit  = WO_ULTRA_Decode_Unit(1, sWmDigVar.sModbDevData[chann].sWoteckUltra.TotalUnit_u16);
            
            sWmDigVar.sModbDevData[chann].Factor      = 0xFD;
            break;
        case __MET_WM_MONG_CAI:
            sWmDigVar.sModbDevData[chann].Status_u8 = true;
            sWmDigVar.sModbDevData[chann].Type_u8 = type;

            sWmDigVar.sModbDevData[chann].nTotal_i64   = 0;
            sWmDigVar.sModbDevData[chann].nForw_i64   = 0;
            sWmDigVar.sModbDevData[chann].nRev_i64    = 0;
            
            sWmDigVar.sModbDevData[chann].Flow_i32    = (int32_t ) sWmDigVar.sModbDevData[chann].sWoteckUltra.Flow_f * 1000;
            sWmDigVar.sModbDevData[chann].FlowM3_i32  = sWmDigVar.sModbDevData[chann].Flow_i32;
            
            sWmDigVar.sModbDevData[chann].sFlowUnit   = " (m3/h)";
            sWmDigVar.sModbDevData[chann].sTotalUnit  = " (m3)";
            
            sWmDigVar.sModbDevData[chann].Factor      = 0xFD;
            break;
        case __MET_SI_MAG8000:
            sWmDigVar.sModbDevData[chann].Status_u8 = true;
            sWmDigVar.sModbDevData[chann].Type_u8 = type;

            sWmDigVar.sModbDevData[chann].nTotal_i64   = (int64_t ) sWmDigVar.sModbDevData[chann].sMag8000.Net_f * 1000;;
            sWmDigVar.sModbDevData[chann].nForw_i64   = (int64_t ) sWmDigVar.sModbDevData[chann].sMag8000.Forward_f * 1000;
            sWmDigVar.sModbDevData[chann].nRev_i64    = (int64_t ) sWmDigVar.sModbDevData[chann].sMag8000.Revert_f * 1000;
            
            sWmDigVar.sModbDevData[chann].Flow_i32    = (int32_t ) sWmDigVar.sModbDevData[chann].sMag8000.Flow_f * 1000;
            sWmDigVar.sModbDevData[chann].FlowM3_i32  = sWmDigVar.sModbDevData[chann].Flow_i32;
            
            sWmDigVar.sModbDevData[chann].sFlowUnit   = sWmDigVar.sModbDevData[chann].sMag8000.aFLOW_UNIT;
            sWmDigVar.sModbDevData[chann].sTotalUnit  = sWmDigVar.sModbDevData[chann].sMag8000.aTOTAL_UNIT;
            
            sWmDigVar.sModbDevData[chann].PinPercent_u8  = (uint8_t ) sWmDigVar.sModbDevData[chann].sMag8000.PinPercent_u16;
            
            sWmDigVar.sModbDevData[chann].Factor      = 0xFD;
            break;
        default:
            break;
    }
  
    switch (type)
    {
        case __MET_LEVEL_LIQ:
        case __MET_LEVEL_LIQ_SUP:
        case __MET_LEVEL_ULTRA:
            //tinh lai level dong: qui doi lwire (cm) ra cung don vi voi Lval
            Temp_u32 = sWmDigVar.sModbDevData[chann].Lwire_u16;
            Temp_u32 *= Convert_Scale(sWmDigVar.sModbDevData[chann].LDecimal_u16);
            Temp_u32 = Temp_u32 / WM_DIG_cm_To_cUnit(sWmDigVar.sModbDevData[chann].LUnit_u16);
            
            sWmDigVar.sModbDevData[chann].Ldynamic_u16 = (int16_t) ( Temp_u32 ) - sWmDigVar.sModbDevData[chann].LVal_i16;
            sWmDigVar.sModbDevData[chann].Ldelta_i16 = sWmDigVar.sModbDevData[chann].Ldynamic_u16 - sWmDigVar.sModbDevData[chann].Lstatic_u16;
            break;
    }
}



void WM_DIG_Packet_Mess (sData *pData, uint8_t chann)
{
    uint16_t TempU16 = 0;  
    
    switch (sWmDigVar.sModbDevData[chann].Type_u8)
    {
        case __MET_WOTECK:
        case __MET_MT100:
        case __MET_WM_SENCE: 
        case __MET_WOTECK_ULTRA: 
        case __MET_WM_MONG_CAI:   
        case __MET_SI_MAG8000:
            SV_Protocol_Packet_Data(pData->Data_a8, &pData->Length_u16, 
                                    OBIS_WM_PULSE_FORWARD, &sWmDigVar.sModbDevData[chann].nForw_i64, 8, sWmDigVar.sModbDevData[chann].Factor);

            SV_Protocol_Packet_Data(pData->Data_a8, &pData->Length_u16, 
                                    OBIS_WM_PULSE_REVERSE, &sWmDigVar.sModbDevData[chann].nRev_i64, 8, sWmDigVar.sModbDevData[chann].Factor);

            SV_Protocol_Packet_Data(pData->Data_a8, &pData->Length_u16, 
                                    OBIS_WM_PULSE, &sWmDigVar.sModbDevData[chann].nTotal_i64, 8, sWmDigVar.sModbDevData[chann].Factor);

            SV_Protocol_Packet_Data(pData->Data_a8, &pData->Length_u16, 
                                    OBIS_WM_DIG_FLOW, &sWmDigVar.sModbDevData[chann].FlowM3_i32, 4, sWmDigVar.sModbDevData[chann].Factor);
            
            SV_Protocol_Packet_Data(pData->Data_a8, &pData->Length_u16, 
                                    OBIS_EMET_VOL, &sWmDigVar.sModbDevData[chann].PinPercent_u8, 1, 0x00);
            break;
        case __MET_LEVEL_LIQ:
        case __MET_LEVEL_LIQ_SUP:
        case __MET_LEVEL_ULTRA:
                SV_Protocol_Packet_Data(pData->Data_a8, &pData->Length_u16, OBIS_WM_PULSE, &TempU16, 2, 0x00);

                //----------Level Unit --------------------
                SV_Protocol_Packet_Data(pData->Data_a8, &pData->Length_u16, OBIS_WM_LEVEL_UNIT, &sWmDigVar.sModbDevData[chann].LUnit_u16, 1, 0xAA);
   
                //----------Level Value Sensor --------------------
                SV_Protocol_Packet_Data(pData->Data_a8, &pData->Length_u16, OBIS_WM_LEVEL_VAL_SENSOR, &sWmDigVar.sModbDevData[chann].LVal_i16, 2, 
                                                0 - (uint8_t)sWmDigVar.sModbDevData[chann].LDecimal_u16);
                //gia tri muc nuoc tinh
                SV_Protocol_Packet_Data(pData->Data_a8, &pData->Length_u16, OBIS_WM_LEVEL_VAL_STA, &sWmDigVar.sModbDevData[chann].Lstatic_u16, 2, 
                                                0 - (uint8_t)sWmDigVar.sModbDevData[chann].LDecimal_u16);
                
                //gia tri muc nuoc dong
                SV_Protocol_Packet_Data(pData->Data_a8, &pData->Length_u16, OBIS_WM_LEVEL_VAL_DYM, &sWmDigVar.sModbDevData[chann].Ldynamic_u16, 2, 
                                                0 - (uint8_t)sWmDigVar.sModbDevData[chann].LDecimal_u16);
                
                //gia tri muc nuoc delta
                SV_Protocol_Packet_Data(pData->Data_a8, &pData->Length_u16, OBIS_WM_LEVEL_VAL_DEL, &sWmDigVar.sModbDevData[chann].Ldelta_i16, 2, 
                                                0 - (uint8_t)sWmDigVar.sModbDevData[chann].LDecimal_u16);  
                
                SV_Protocol_Packet_Data(pData->Data_a8, &pData->Length_u16, 
                                    OBIS_EMET_VOL, &sWmDigVar.sModbDevData[chann].PinPercent_u8, 1, 0x00);
            break;
        default:
            break;
    }
}



uint16_t WM_DIG_cm_To_cUnit (uint8_t unit)
{
    uint16_t div = 1;
    
    //don vi thuc te m -> chia 100
    if (unit == 7) { 
        div = 100;
    } 
    
    return div;
}
   



uint8_t WM_DIG_Get_Infor (uint8_t chann, uint8_t type, uint16_t *addr, uint8_t *nReg)
{
    uint8_t result = true;
    
    switch (type)
    {
        case __MET_WOTECK_ULTRA:
            result = WOTECK_ULTRA_Get_Reg(sWmDigVar.sModbDevData[chann].inReg, addr, nReg);
            break;
        case __MET_SI_MAG8000:
            result = MAG8000_Get_Reg(sWmDigVar.sModbDevData[chann].inReg, addr, nReg);
            break;
        default:
            *addr = sListWmDigital[sWmDigVar.sModbInfor[chann].MType_u8].Addr_u32;
            *nReg = sListWmDigital[sWmDigVar.sModbInfor[chann].MType_u8].MaxRegis_u8;
            break;
    }
    
    return result;
}







