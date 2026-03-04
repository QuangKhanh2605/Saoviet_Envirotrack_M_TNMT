

#include "button.h"
#include "lcd_display.h"

sButtonStruct sButton;


/*============ Function =============*/

void BUTTON_Process(void)
{
    switch(sButton.Value)
    {
        case _ENTER:  
            BUTTON_Enter_Process();
            break;
        case _UP:  
            BUTTON_Up_Process();
            break;
        case _DOWN:               
            BUTTON_Down_Process();
            break;
        case _ESC:
            BUTTON_ESC_Process();
            break;
        default:
            break;
    }
}



uint8_t BUTTON_scan(void)
{
    uint8_t r_value = 0;
    static uint8_t last_value = 0;
    static uint32_t hold = 0;
    
	if (sButton.Status == 0)
	{
		r_value = (!BUTTON_ENTER<<0)|(!BUTTON_UP<<1)|(!BUTTON_DOWN<<2)|(!BUTTON_ESC<<3);
        
		switch(r_value)
		{
			case _ENTER:
            case _UP:
            case _DOWN:
            case _ESC:
                sButton.Count++;
                if (sButton.Count >= BUT_TIME_DETECT) {
                    sButton.Value = r_value;
                    sButton.Status = 1;
                    
                    if ( r_value == last_value ) {
                        hold++;
                        if ( hold >= BUT_COUNT_FAST ) {
                            sButton.Count = BUT_TIME_DETECT;
                            hold = BUT_COUNT_FAST;
                        } else if (hold >= BUT_COUNT_HOLD) {
                            sButton.Count = BUT_TIME_DETECT - 2;
                        } else {
                            sButton.Count = 0;
                        }
                    } else {
                        sButton.Count = 0;  
                        hold = 0;
                    } 
                                
                    last_value = r_value;
                }
  				break;
			default:
                last_value = 0;
				sButton.Count = BUT_TIME_DETECT/2;
                hold = 0;
				break;
		}
	}

    return sButton.Value;
}

char aTEST[20] = {"1234"};
static sPressureLinearInter sPressConfig = {0};
static int16_t PressOldCalib_i16 = 0;
static char sConfirm[2][10] = {"No", "Yes"};
static char sSetLeve11[] = {"*ESC only save Lwire!"}; 
static char sSetLevel2[] = {"*Wait 10s -> ENTER!"};
static uint8_t tempu8 = 0;

void BUTTON_Enter_Process (void)
{
    uint64_t  TempU64 = 0;
    uint8_t   Scale = 0;
    static Struct_Pulse sPulseSett;
    
    switch (sLCD.sScreenNow.Index_u8)
    {
        case _LCD_SCREEN_1:
        case _LCD_SCREEN_2:
        case _LCD_SCREEN_3:
        case _LCD_SCREEN_4:
        case _LCD_SCREEN_5:
        case _LCD_SCREEN_6:
        case _LCD_SCREEN_7:
        case _LCD_SCREEN_7b:
            sButton.Old_value = 0;
            UTIL_MEM_set(aTEST, 0, sizeof(aTEST));
            aTEST[0] = '0'; aTEST[1] = '0'; aTEST[2] = '0'; aTEST[3] = '0';

            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_PASS, 0,
                               __PASS_WORD_2, __PASS_WORD_1, __PASS_WORD_2,
                               aTEST, (sButton.Old_value << 4) + 0x02);
            break;
        case _LCD_SCR_PASS:
            sButton.Old_value++;
            if (sButton.Old_value > 3)
            { 
                if (Display_Check_Password((uint8_t *) aTEST) == true) {
                    //copy screen now to screenback
                    UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                    
                    Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SETTING, 0,
                                       __SET_REQ_1, __SET_REQ_1, __SET_PRESS_CALIB,
                                       NULL, 0xF1);
                } else {
                    sLCD.sScreenNow.Index_u8 = _LCD_SCREEN_1;  //thoat
                }
            } else {
                Display_Set_Screen_Flag(&sLCD.sScreenNow, NULL, (sButton.Old_value << 4) + 0x02);
            }
            break;
        case _LCD_SCR_SETTING:
            switch (sLCD.sScreenNow.Para_u8)
            {
                case __SET_REQ_1:
                    UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                    Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_FREQ, 0,
                                       __SET_REQ_2_2, __SET_REQ_2_1, __SET_REQ_2_2,
                                       &sButton.Old_value, 0xF2);

                    sButton.Old_value = sModemInfor.sFrequence.DurOnline_u32;
                    break;
//                case __SET_PULSE_SETT:
//                    UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
//                    Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_PULSE_1, 0,
//                                       __SET_PULSE_SET1_1, __SET_PULSE_SET1_1, __SET_PULSE_SET1_4,
//                                       NULL, 0xF1);
//                    break;
//                case __SET_PULSE_RESET:
//                    UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
//                    Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_PULSE_RESET, 0,
//                                       __SET_PULSE_RS_1, __SET_PULSE_RS_1, __SET_PULSE_RS_4,
//                                       NULL, 0xF1);  
//                    break;
//                case __SET_PRESSURE:
//                    UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
//                    Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_PRESS_1, 0,
//                                       __SET_PRESSURE_1_1, __SET_PRESSURE_1_1, __SET_PRESSURE_1_5,
//                                       NULL, 0xF1);
//                    break;
//                case __SET_PRESS_CALIB:
//                    UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
//                    Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_CAL_CHANN_1, 0,
//                                       __CAL_PRESS_CH_1, __CAL_PRESS_CH_1, __CAL_PRESS_CH_5,
//                                       NULL, 0xF1);
//                    break;
                    
                default:
                  break;
            }
            
            break;
        case _LCD_SCR_SETTING_2:
            switch (sLCD.sScreenNow.Para_u8)
            {
//                case __SET_LEVEL_SETT:
//                    UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
//                    Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_LEV_SET_1, 0,
//                                       __SET_LEV_SET1_1, __SET_LEV_SET1_1, __SET_LEV_SET1_4,
//                                       NULL, 0xF1); 
//                    break;
//                case __SET_RESTORE_DEV:
//                    sButton.Old_value = 0;
//                    UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
//                    Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_RESTORE_DEV, 0,
//                                       __SET_RES_CONF_2, __SET_RES_CONF_1, __SET_RES_CONF_2,
//                                       sConfirm[sButton.Old_value], 0xF2);  
//                    break;
                    
                case __SET_OPTION_SENSOR:
                    UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                    Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_OPTION_SENSOR, 0,
                                       __SET_OPTION_SS_PH, __SET_OPTION_SS_PH, __SET_OPTION_SS_COD,
                                       NULL, 0xF1);
                    break; 
                    
                case __SET_STATE_CALIB:
                    sButton.Old_value = StateCalibSensor;
                    UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                    Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_CALIB_SS, 0,
                                       __SET_CALIB_SS_2, __SET_CALIB_SS_1, __SET_CALIB_SS_2,
                                       sConfirm[sButton.Old_value], 0xF2);
                    break; 
                    
                case __SET_MANUFACTOR:
                    UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                    Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCREEN_8, 0,
                                       __INFORMATION, __INFORMATION, __FW_VERSION_1,
                                       NULL, 0xF0);
                    break;
            }
            break;
            
        case _LCD_SCR_PULSE_RESET:
            tempu8 = sLCD.sScreenNow.Para_u8 - __SET_PULSE_RS_1;
            sButton.Old_value = 0;
            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_RS_CONFIRM, 0,
                               __SET_RESET_CONF_2, __SET_RESET_CONF_1, __SET_RESET_CONF_2,
                               sConfirm[sButton.Old_value], 0xF2);  
            break;
        case _LCD_SCR_SET_FREQ:
            sModemInfor.sFrequence.DurOnline_u32 = sButton.Old_value; 
            //Luu lai gia tri
            Modem_Save_Var();
            AppComm_Set_Next_TxTimer();

            //quay lai man cu
            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_SUCCESS, 0,
                               __SET_SUCCESS, __SET_SUCCESS, __SET_SUCCESS,
                               NULL, 0xF0); 
            break;
        case _LCD_SCR_SET_PRESS_1:
        case _LCD_SCR_SET_PRESS_1_2:
            //Set lai con tro data press
            tempu8 = sLCD.sScreenNow.Para_u8 - __SET_PRESSURE_1_1;
            UTIL_MEM_cpy(&sPressConfig, &sWmVar.aPRESSURE[tempu8].sLinearInter, sizeof(sPressureLinearInter));
            
            sLCDObject[__SET_PRESSURE_2_0].pData = AnalogType[sPressConfig.Type_u8]; 
            
            sLCDObject[__SET_PRESSURE_2_1].pData = &sPressConfig.Factor_u16; 
            sLCDObject[__SET_PRESSURE_2_1].Scale_u8 = sPressConfig.FactorDec_u8; 
            sLCDObject[__SET_PRESSURE_2_1].dType_u8 = _DTYPE_U16;
            
            sLCDObject[__SET_PRESSURE_2_2].pData = &sPressConfig.InMin_u16; 
            sLCDObject[__SET_PRESSURE_2_2].Scale_u8 = 0;
            sLCDObject[__SET_PRESSURE_2_2].dType_u8 = _DTYPE_U16;
            
            sLCDObject[__SET_PRESSURE_2_3].pData = &sPressConfig.InMax_u16; 
            sLCDObject[__SET_PRESSURE_2_3].Scale_u8 = 0;
            sLCDObject[__SET_PRESSURE_2_3].dType_u8 = _DTYPE_U16;
            
            sLCDObject[__SET_PRESSURE_2_4].pData = aUnitWm[sPressConfig.InUnit_u8]; 
            
            sLCDObject[__SET_PRESSURE_2_5].pData = &sPressConfig.OutMin_u16; 
            sLCDObject[__SET_PRESSURE_2_5].Scale_u8 = 0;
            sLCDObject[__SET_PRESSURE_2_5].dType_u8 = _DTYPE_U16;
            
            sLCDObject[__SET_PRESSURE_2_6].pData = &sPressConfig.OutMax_u16; 
            sLCDObject[__SET_PRESSURE_2_6].Scale_u8 = 0;
            sLCDObject[__SET_PRESSURE_2_6].dType_u8 = _DTYPE_U16;
            
            sLCDObject[__SET_PRESSURE_2_7].pData = aUnitWm[sPressConfig.OutUnit_u8];
            //
            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_PRESS_2, 0,
                                       __SET_PRESSURE_2_0, __SET_PRESSURE_2_0, __SET_PRESSURE_2_7,
                                       NULL, 0xF3);
            sLCD.rSett_u8 = false;
            break;
        case _LCD_SCR_SET_PRESS_2:  
            switch (sLCD.sScreenNow.Para_u8)
            {
                case __SET_PRESSURE_2_0:
                case __SET_PRESSURE_2_4: 
                case __SET_PRESSURE_2_7: 
                    if (sLCD.rSett_u8 == false) {
                        sLCD.rSett_u8 = true;
                        Display_Set_Screen_Flag(&sLCD.sScreenNow, NULL, 0xF2);
                        break;
                    }
                    
                    sLCD.sScreenNow.Para_u8++;
                    Display_Set_Screen_Flag(&sLCD.sScreenNow, NULL, 0xF3);
                    sLCD.rSett_u8 = false;
                    break;
                case __SET_PRESSURE_2_1: 
                case __SET_PRESSURE_2_2: 
                case __SET_PRESSURE_2_3: 
                
                case __SET_PRESSURE_2_5: 
                case __SET_PRESSURE_2_6: 
                    if (sLCD.rSett_u8 == false) {
                        sLCD.rSett_u8 = true;
                        
                        sButton.Old_value = 0;
                        UTIL_MEM_set(aTEST, 0, sizeof(aTEST));
                        aTEST[0] = '1';

                        sLCD.pData = sLCDObject[sLCD.sScreenNow.Para_u8].pData;
                        sLCD.Type_u8 = sLCDObject[sLCD.sScreenNow.Para_u8].dType_u8;
                            
                        sLCDObject[sLCD.sScreenNow.Para_u8].pData = aTEST; 
                        sLCDObject[sLCD.sScreenNow.Para_u8].dType_u8 = _DTYPE_STRING; 
                        Display_Set_Screen_Flag(&sLCD.sScreenNow, NULL, (sButton.Old_value << 4) + 0x02);
                        break;
                    }
                    
                    if (Button_Is_End_Sett_Str() == true) {
                        Scale = 0xFF - UTIL_Count_Decimal_Places(aTEST) + 1;
                        TempU64 = (uint64_t) ( (float) (UtilStringToFloat(aTEST) * Convert_Scale(0xFF - Scale + 1) ) ) ; 
                        
                        Display_Set_Data(sLCD.pData, TempU64, sLCD.Type_u8);
                        sLCDObject[sLCD.sScreenNow.Para_u8].pData = sLCD.pData;
                        sLCDObject[sLCD.sScreenNow.Para_u8].dType_u8 = sLCD.Type_u8;
                        sLCDObject[sLCD.sScreenNow.Para_u8].Scale_u8 = Scale;
                        
                        sLCD.sScreenNow.Para_u8++;
                        Display_Set_Screen_Flag(&sLCD.sScreenNow, NULL, 0xF3);
                        sLCD.rSett_u8 = false;
                    } 
                    break;
            }
            
            if (sLCD.sScreenNow.Para_u8 > sLCD.sScreenNow.ParaMax_u8) {
                sPressConfig.FactorDec_u8 = sLCDObject[__SET_PRESSURE_2_1].Scale_u8;
                //Copy data ra bien 
                UTIL_MEM_cpy(&sWmVar.aPRESSURE[tempu8].sLinearInter, &sPressConfig, sizeof(sPressureLinearInter));
                //luu gia tri
                AppWm_Save_Press_Infor();
                //cap nhat len server
                sMessage.aMESS_PENDING[DATA_HANDSHAKE] = TRUE;
                
                //ket thuc chuyen man
                Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_SUCCESS, 0,
                                   __SET_SUCCESS, __SET_SUCCESS, __SET_SUCCESS,
                                   NULL, 0xF0); 
            }
            
            break;
        case _LCD_SCR_SET_PULSE_1:
            tempu8 = sLCD.sScreenNow.Para_u8 - __SET_PULSE_SET1_1;
            UTIL_MEM_cpy(&sPulseSett, &sPulse[tempu8], sizeof(Struct_Pulse));
            
            sLCDObject[__SET_PULSE_SET2_1].pData = &sPulseSett.FactorInt_i16;  
            sLCDObject[__SET_PULSE_SET2_1].Scale_u8 = sPulseSett.FactorDec_u8;
            sLCDObject[__SET_PULSE_SET2_1].dType_u8 = _DTYPE_U8;
            
            sLCDObject[__SET_PULSE_SET2_2].pData = &sPulseSett.Start_lf; 
            sLCDObject[__SET_PULSE_SET2_2].Scale_u8 = sPulseSett.FactorDec_u8; 
            sLCDObject[__SET_PULSE_SET2_2].dType_u8 = _DTYPE_U64;
            
            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_PULSE_2, 0,
                                       __SET_PULSE_SET2_1, __SET_PULSE_SET2_1, __SET_PULSE_SET2_2,
                                       NULL, 0xF3);
            
            sLCD.rSett_u8 = false;
            break;
        case _LCD_SCR_SET_PULSE_2:
            if (sLCD.rSett_u8 == false) {
                sLCD.rSett_u8 = true;
                
                sButton.Old_value = 0;
                UTIL_MEM_set(aTEST, 0, sizeof(aTEST));
                aTEST[0] = '1';

                sLCD.pData = sLCDObject[sLCD.sScreenNow.Para_u8].pData;
                sLCD.Type_u8 = sLCDObject[sLCD.sScreenNow.Para_u8].dType_u8;
                    
                sLCDObject[sLCD.sScreenNow.Para_u8].pData = aTEST; 
                sLCDObject[sLCD.sScreenNow.Para_u8].dType_u8 = _DTYPE_STRING; 
                Display_Set_Screen_Flag(&sLCD.sScreenNow, NULL, (sButton.Old_value << 4) + 0x02);
                break;
            }
    
            if (Button_Is_End_Sett_Str() == true) {
                if (sLCD.sScreenNow.Para_u8 == __SET_PULSE_SET2_2) {
                    Scale = sLCDObject[__SET_PULSE_SET2_1].Scale_u8;
                } else {
                    Scale = 0xFF - UTIL_Count_Decimal_Places(aTEST) + 1;
                }
                
                TempU64 = (uint64_t) ( (float) (UtilStringToFloat(aTEST) * Convert_Scale(0xFF - Scale + 1) ) ) ; 
                
                Display_Set_Data(sLCD.pData, TempU64, sLCD.Type_u8);
                
                sLCDObject[sLCD.sScreenNow.Para_u8].pData = sLCD.pData;
                sLCDObject[sLCD.sScreenNow.Para_u8].dType_u8 = sLCD.Type_u8;
                sLCDObject[sLCD.sScreenNow.Para_u8].Scale_u8 = Scale;
                
                sLCD.sScreenNow.Para_u8++;
                Display_Set_Screen_Flag(&sLCD.sScreenNow, NULL, 0xF3);
                sLCD.rSett_u8 = false;
            }            
            
            if (sLCD.sScreenNow.Para_u8 > sLCD.sScreenNow.ParaMax_u8) {
                //convert lai so start
                sPulseSett.FactorDec_u8 = sLCDObject[__SET_PULSE_SET2_1].Scale_u8;
                UTIL_MEM_cpy(&sPulse[tempu8], &sPulseSett, sizeof(Struct_Pulse));
                
                //luu gia tri
                AppWm_Save_Pulse();
                //cap nhat len server
                sMessage.aMESS_PENDING[DATA_HANDSHAKE] = TRUE;
                //ket thuc chuyen man
                Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_SUCCESS, 0,
                                   __SET_SUCCESS, __SET_SUCCESS, __SET_SUCCESS,
                                   NULL, 0xF0); 
            }
            break;
        case _LCD_SCR_RS_CONFIRM:  
            if (sButton.Old_value == 1) {
                sPulse[tempu8].Number_i64 = 0;
                sPulse[tempu8].NumberInit_i64 = sPulse[tempu8].Number_i64;   

                AppWm_Save_Pulse();
   
                Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_SUCCESS, 0,
                                   __SET_SUCCESS, __SET_SUCCESS, __SET_SUCCESS,
                                   NULL, 0xF0); 
            } else {
                UTIL_MEM_cpy(&sLCD.sScreenNow, &sLCD.sScreenBack, sizeof(sScreenInformation));
            }
            break; 
        case _LCD_SCR_RESTORE_DEV:  
            if (sButton.Old_value == 1) {
                AppWm_Default_Pulse();
                
                Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_SUCCESS, 0,
                                   __SET_SUCCESS, __SET_SUCCESS, __SET_SUCCESS,
                                   NULL, 0xF0); 
            } else {
                UTIL_MEM_cpy(&sLCD.sScreenNow, &sLCD.sScreenBack, sizeof(sScreenInformation));
            }
            break;
        case _LCD_SCR_SET_SUCCESS:
            UTIL_MEM_cpy(&sLCD.sScreenNow, &sLCD.sScreenBack, sizeof(sScreenInformation));
            break;
        case _LCD_SCR_CAL_CHANN_1:
        case _LCD_SCR_CAL_CHANN_2:
            tempu8 = sLCD.sScreenNow.Para_u8 - __CAL_PRESS_CH_1;
            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_CAL_PRESS, 0,
                               __CAL_PRESSURE_2, __CAL_PRESSURE_1, __CAL_PRESSURE_4,
                               &sWmVar.aPRESSURE[tempu8].mVol_i32, 0xF2);
            
            sLCDObject[__CAL_PRESSURE_1].pData  = &PressOldCalib_i16;
            
            PressOldCalib_i16 = sWmVar.aPRESSURE[tempu8].Calib_i16;
            sButton.Old_value = tempu8;
            sWmVar.aPRESSURE[tempu8].Calib_i16 = 0;
            
            fevent_active(sEventAppWM, _EVENT_MEAS_PRESSURE);  
            break;
        case _LCD_SCR_CAL_PRESS:
            //ghi nhan gia tri va luu lai
            sWmVar.aPRESSURE[tempu8].Calib_i16 = sWmVar.aPRESSURE[tempu8].mVol_i32;
            AppWm_Save_Press_Infor();
            
            //ket thuc chuyen man
            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_SUCCESS, 0,
                                   __SET_SUCCESS, __SET_SUCCESS, __SET_SUCCESS,
                                   NULL, 0xF0); 
            break;
        case _LCD_SCR_LEV_SET_1:
            tempu8 = sLCD.sScreenNow.Para_u8 - __SET_LEV_SET1_1;

            sLCDObject[__SET_LEV_SET2_1].pData = &sWmDigVar.sModbDevData[tempu8].Lwire_u16; 
            sLCDObject[__SET_LEV_SET2_1].Scale_u8 = 0xFE;
            sLCDObject[__SET_LEV_SET2_1].dType_u8 = _DTYPE_U16; 
            
            sLCDObject[__SET_LEV_SET2_2].pData = &sWmDigVar.sModbDevData[tempu8].Lstatic_u16; 
            sLCDObject[__SET_LEV_SET2_2].Scale_u8 = 0xFF - sWmDigVar.sModbDevData[tempu8].LDecimal_u16 + 1 ;
            
            sLCDObject[__SET_LEV_SET2_3].pData = &sWmDigVar.sModbDevData[tempu8].Ldynamic_u16; 
            sLCDObject[__SET_LEV_SET2_3].Scale_u8 = 0xFF - sWmDigVar.sModbDevData[tempu8].LDecimal_u16 + 1;
            
            sLCDObject[__SET_LEV_SET2_4].pData = sSetLeve11;
            
            fevent_active(sEventAppWM, _EVENT_RS485_MODBUS); 
            fevent_active(sEventAppWM, _EVENT_RS485_2_MODBUS); 
            
            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_LEV_SET_2, 0,
                               __SET_LEV_SET2_1, __SET_LEV_SET2_1, __SET_LEV_SET2_4,
                               NULL, 0xF3);
            
            sLCD.rSett_u8 = false;
            break;
        case _LCD_SCR_LEV_SET_2:
            if (sLCD.sScreenNow.Para_u8 == __SET_LEV_SET2_1) {
                if (sLCD.rSett_u8 == false) {
                    sLCD.rSett_u8 = true;
                    
                    sButton.Old_value = 0;
                    UTIL_MEM_set(aTEST, 0, sizeof(aTEST));
                    aTEST[0] = '1';
                        
                    sLCDObject[sLCD.sScreenNow.Para_u8].pData = aTEST; 
                    sLCDObject[sLCD.sScreenNow.Para_u8].dType_u8 = _DTYPE_STRING; 
                    Display_Set_Screen_Flag(&sLCD.sScreenNow, NULL, (sButton.Old_value << 4) + 0x02);
                    break;
                }
        
                if (Button_Is_End_Sett_Str() == true) {
                    sWmDigVar.sModbDevData[tempu8].Lwire_u16 = (uint64_t) ( (float) (UtilStringToFloat(aTEST) * 100 ) ) ; 
                    
                    sLCDObject[sLCD.sScreenNow.Para_u8].pData = &sWmDigVar.sModbDevData[tempu8].Lwire_u16;
                    sLCDObject[sLCD.sScreenNow.Para_u8].dType_u8 = _DTYPE_U16;
                    
                    sLCDObject[__SET_LEV_SET2_4].pData = sSetLevel2;
                    
                    sLCD.sScreenNow.Para_u8 = __SET_LEV_SET2_3;
                    Display_Set_Screen_Flag(&sLCD.sScreenNow, NULL, 0xF2);
                    sLCD.rSett_u8 = false;
                }   
                
                break;
            }
            
            //luu lai gia tri va cap nhat
            sWmDigVar.sModbDevData[tempu8].Lstatic_u16 = sWmDigVar.sModbDevData[tempu8].Ldynamic_u16;
            AppWm_Save_WM_Dig_Infor();
            
            //ket thuc chuyen man
            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_SUCCESS, 0,
                               __SET_SUCCESS, __SET_SUCCESS, __SET_SUCCESS,
                               NULL, 0xF0); 
            break;

        case _LCD_SCR_SET_OPTION_SENSOR:
            switch (sLCD.sScreenNow.Para_u8)
            {  
                case __SET_OPTION_SS_PH:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_OPTION_SENSOR, (sLCD.sScreenNow.SubIndex_u8+1),
                                               __SET_OPTION_SS_PH, __SET_OPTION_SS_PH, __SET_OPTION_SS_COD,
                                               &sButton.Old_value, 0xF2);
                            sButton.Old_value = s485Measure[_SS_PH].sUser;
                            break;
                            
                        case 1:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_OPTION_SENSOR, 0,
                                               __SET_OPTION_SS_PH, __SET_OPTION_SS_PH, __SET_OPTION_SS_COD,
                                               &s485Measure[_SS_PH].sUser, 0xF1);
                            Save_UserSensor(_SS_PH, sButton.Old_value);
                            break;
                        
                        default:
                            break;
                    }
                    break;
                    
                case __SET_OPTION_SS_CLO:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_OPTION_SENSOR, (sLCD.sScreenNow.SubIndex_u8+1),
                                               __SET_OPTION_SS_CLO, __SET_OPTION_SS_PH, __SET_OPTION_SS_COD,
                                               &sButton.Old_value, 0xF2);
                            sButton.Old_value = s485Measure[_SS_CLO].sUser;
                            break;
                            
                        case 1:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_OPTION_SENSOR, 0,
                                               __SET_OPTION_SS_CLO, __SET_OPTION_SS_PH, __SET_OPTION_SS_COD,
                                               &s485Measure[_SS_CLO].sUser, 0xF1);
                            Save_UserSensor(_SS_CLO, sButton.Old_value);
                            break;
                        
                        default:
                            break;
                    }
                    break;
                    
                case __SET_OPTION_SS_EC:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_OPTION_SENSOR, (sLCD.sScreenNow.SubIndex_u8+1),
                                               __SET_OPTION_SS_EC, __SET_OPTION_SS_PH, __SET_OPTION_SS_COD,
                                               &sButton.Old_value, 0xF2);
                            sButton.Old_value = s485Measure[_SS_EC].sUser;
                            break;
                            
                        case 1:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_OPTION_SENSOR, 0,
                                               __SET_OPTION_SS_EC, __SET_OPTION_SS_PH, __SET_OPTION_SS_COD,
                                               &s485Measure[_SS_EC].sUser, 0xF1);
                            Save_UserSensor(_SS_EC, sButton.Old_value);
                            break;
                        
                        default:
                            break;
                    }
                    break;
                    
                case __SET_OPTION_SS_TURB:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_OPTION_SENSOR, (sLCD.sScreenNow.SubIndex_u8+1),
                                               __SET_OPTION_SS_TURB, __SET_OPTION_SS_PH, __SET_OPTION_SS_COD,
                                               &sButton.Old_value, 0xF2);
                            sButton.Old_value = s485Measure[_SS_TURB].sUser;
                            break;
                            
                        case 1:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_OPTION_SENSOR, 0,
                                               __SET_OPTION_SS_TURB, __SET_OPTION_SS_PH, __SET_OPTION_SS_COD,
                                               &s485Measure[_SS_TURB].sUser, 0xF1);
                            Save_UserSensor(_SS_TURB, sButton.Old_value);
                            break;
                        
                        default:
                            break;
                    }
                    break;
                    
                case __SET_OPTION_SS_COD:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_OPTION_SENSOR, (sLCD.sScreenNow.SubIndex_u8+1),
                                               __SET_OPTION_SS_COD, __SET_OPTION_SS_PH, __SET_OPTION_SS_COD,
                                               &sButton.Old_value, 0xF2);
                            sButton.Old_value = s485Measure[_SS_COD].sUser;
                            break;
                            
                        case 1:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_OPTION_SENSOR, 0,
                                               __SET_OPTION_SS_COD, __SET_OPTION_SS_PH, __SET_OPTION_SS_COD,
                                               &s485Measure[_SS_COD].sUser, 0xF1);
                            Save_UserSensor(_SS_COD, sButton.Old_value);
                            break;
                        
                        default:
                            break;
                    }
                    break;
                    
                    default:
                      break;
            }
            break;
            
        case _LCD_SCR_SET_OPTION_SENSOR_TAB_2:
            switch (sLCD.sScreenNow.Para_u8)
            {  
                case __SET_OPTION_SS_TSS:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_OPTION_SENSOR_TAB_2, (sLCD.sScreenNow.SubIndex_u8+1),
                                               __SET_OPTION_SS_TSS, __SET_OPTION_SS_TSS, __SET_OPTION_SS_TDS,
                                               &sButton.Old_value, 0xF2);
                            sButton.Old_value = s485Measure[_SS_TSS].sUser;
                            break;
                            
                        case 1:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_OPTION_SENSOR_TAB_2, 0,
                                               __SET_OPTION_SS_TSS, __SET_OPTION_SS_TSS, __SET_OPTION_SS_TDS,
                                               &s485Measure[_SS_TSS].sUser, 0xF1);
                            Save_UserSensor(_SS_TSS, sButton.Old_value);
                            break;
                        
                        default:
                            break;
                    }
                    break;
                    
                case __SET_OPTION_SS_NH4:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_OPTION_SENSOR_TAB_2, (sLCD.sScreenNow.SubIndex_u8+1),
                                               __SET_OPTION_SS_NH4, __SET_OPTION_SS_TSS, __SET_OPTION_SS_TDS,
                                               &sButton.Old_value, 0xF2);
                            sButton.Old_value = s485Measure[_SS_NH4].sUser;
                            break;
                            
                        case 1:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_OPTION_SENSOR_TAB_2, 0,
                                               __SET_OPTION_SS_NH4, __SET_OPTION_SS_TSS, __SET_OPTION_SS_TDS,
                                               &s485Measure[_SS_NH4].sUser, 0xF1);
                            Save_UserSensor(_SS_NH4, sButton.Old_value);
                            break;
                        
                        default:
                            break;
                    }
                    break;
                    
                case __SET_OPTION_SS_DO:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_OPTION_SENSOR_TAB_2, (sLCD.sScreenNow.SubIndex_u8+1),
                                               __SET_OPTION_SS_DO, __SET_OPTION_SS_TSS, __SET_OPTION_SS_TDS,
                                               &sButton.Old_value, 0xF2);
                            sButton.Old_value = s485Measure[_SS_DO].sUser;
                            break;
                            
                        case 1:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_OPTION_SENSOR_TAB_2, 0,
                                               __SET_OPTION_SS_DO, __SET_OPTION_SS_TSS, __SET_OPTION_SS_TDS,
                                               &s485Measure[_SS_DO].sUser, 0xF1);
                            Save_UserSensor(_SS_DO, sButton.Old_value);
                            break;
                        
                        default:
                            break;
                    }
                    break;
                    
                case __SET_OPTION_SS_SALT:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_OPTION_SENSOR_TAB_2, (sLCD.sScreenNow.SubIndex_u8+1),
                                               __SET_OPTION_SS_SALT, __SET_OPTION_SS_TSS, __SET_OPTION_SS_TDS,
                                               &sButton.Old_value, 0xF2);
                            sButton.Old_value = s485Measure[_SS_SALT].sUser;
                            break;
                            
                        case 1:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_OPTION_SENSOR_TAB_2, 0,
                                               __SET_OPTION_SS_SALT, __SET_OPTION_SS_TSS, __SET_OPTION_SS_TDS,
                                               &s485Measure[_SS_SALT].sUser, 0xF1);
                            Save_UserSensor(_SS_SALT, sButton.Old_value);
                            break;
                        
                        default:
                            break;
                    }
                    break;
                    
                case __SET_OPTION_SS_TDS:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_OPTION_SENSOR_TAB_2, (sLCD.sScreenNow.SubIndex_u8+1),
                                               __SET_OPTION_SS_TDS, __SET_OPTION_SS_TSS, __SET_OPTION_SS_TDS,
                                               &sButton.Old_value, 0xF2);
                            sButton.Old_value = s485Measure[_SS_TDS].sUser;
                            break;
                            
                        case 1:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_OPTION_SENSOR_TAB_2, 0,
                                               __SET_OPTION_SS_TDS, __SET_OPTION_SS_TSS, __SET_OPTION_SS_TDS,
                                               &s485Measure[_SS_TDS].sUser, 0xF1);
                            Save_UserSensor(_SS_TDS, sButton.Old_value);
                            break;
                        
                        default:
                            break;
                    }
                    break;
                    
                    default:
                      break;
            }
            break;
        
        case _LCD_SCR_SET_OPTION_SENSOR_TAB_3:
            switch (sLCD.sScreenNow.Para_u8)
            {  
                case __SET_OPTION_SS_NO3:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_OPTION_SENSOR_TAB_3, (sLCD.sScreenNow.SubIndex_u8+1),
                                               __SET_OPTION_SS_NO3, __SET_OPTION_SS_NO3, __SET_OPTION_SS_NO3,
                                               &sButton.Old_value, 0xF2);
                            sButton.Old_value = s485Measure[_SS_NO3].sUser;
                            break;
                            
                        case 1:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_OPTION_SENSOR_TAB_3, 0,
                                               __SET_OPTION_SS_NO3, __SET_OPTION_SS_NO3, __SET_OPTION_SS_NO3,
                                               &s485Measure[_SS_NO3].sUser, 0xF1);
                            Save_UserSensor(_SS_NO3, sButton.Old_value);
                            break;
                        
                        default:
                            break;
                    }
                    break;
                    
                default:
                  break;
            }
            break;
            
        case _LCD_SCR_CALIB_SS:  
            if (sButton.Old_value <= 1) 
            {
                Save_StateCalibSensor(sButton.Old_value);
                
                Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_SUCCESS, 0,
                                   __SET_SUCCESS, __SET_SUCCESS, __SET_SUCCESS,
                                   NULL, 0xF0); 
            } else {
                UTIL_MEM_cpy(&sLCD.sScreenNow, &sLCD.sScreenBack, sizeof(sScreenInformation));
            }
            break;
            
        case _LCD_SCR_CHECK_SETTING:
//          switch(sLCD.sScreenBack.Index_u8)
//          {
//              case _LCD_SCR_SET_CALIB_SS_CLO:
//                DCU_Enter_Calib();
//                break;
//                
//              default:
//                RS485_Enter_Calib();
//                break;
//          }
          break;
            
        case _LCD_SCREEN_8:
            UTIL_MEM_cpy(&sLCD.sScreenNow, &sLCD.sScreenBack, sizeof(sScreenInformation));
            break;
            
        default:
            break;
    }
}

void BUTTON_Up_Process (void)
{
    switch (sLCD.sScreenNow.Index_u8)
    {
        case _LCD_SCREEN_1:
            if(((sLCDPageSensor.Number + 5) /6) > 2) 
                sLCD.sScreenNow.Index_u8 = _LCD_SCREEN_CM44_3;
            else if(((sLCDPageSensor.Number + 5) /6) > 1) 
                sLCD.sScreenNow.Index_u8 = _LCD_SCREEN_CM44_2;
            else
                sLCD.sScreenNow.Index_u8 = _LCD_SCREEN_CM44;
            break;
            
        case _LCD_SCREEN_CM44:
                sLCD.sScreenNow.Index_u8 = _LCD_SCREEN_1;
            break;
            
        case _LCD_SCREEN_CM44_2:
                sLCD.sScreenNow.Index_u8 = _LCD_SCREEN_CM44;
            break;
            
        case _LCD_SCREEN_CM44_3:
                sLCD.sScreenNow.Index_u8 = _LCD_SCREEN_CM44_2;
            break;
      
//        case _LCD_SCREEN_1:
//            sButton.Old_value = _LCD_SCREEN_1;
//            sLCD.sScreenNow.Index_u8 = _LCD_SCREEN_7b;
//            if (sWmDigVar.nModbus_u8 >= 1) {
//                sLCD.sScreenNow.SubIndex_u8 = sWmDigVar.nModbus_u8 - 1;
//            } else {
//                sLCD.sScreenNow.SubIndex_u8 = 0;
//            }
//            break;
        case _LCD_SCREEN_2:
        case _LCD_SCREEN_3:
        case _LCD_SCREEN_4:
        case _LCD_SCREEN_5:
        case _LCD_SCREEN_6:
        case _LCD_SCREEN_7:
            sLCD.sScreenNow.Index_u8--;
            break;
        case _LCD_SCREEN_7b:
            sButton.Old_value = _LCD_SCREEN_1;
            if (sLCD.sScreenNow.SubIndex_u8 > 0) {
                sLCD.sScreenNow.SubIndex_u8--;
            } else {
                sLCD.sScreenNow.Index_u8--;
            }
            break;
        case _LCD_SCR_PASS:
            aTEST[sButton.Old_value] ++;
            if (aTEST[sButton.Old_value] > '9')
                aTEST[sButton.Old_value] = '0';
            break;
        case _LCD_SCR_SETTING:
        case _LCD_SCR_SET_PRESS_1:
        case _LCD_SCR_CAL_CHANN_1:
        case _LCD_SCR_LEV_SET_1:
            if (sLCD.sScreenNow.Para_u8 > sLCD.sScreenNow.ParaMin_u8 ) {
                sLCD.sScreenNow.Para_u8--;
            }
            Display_Set_Screen_Flag(&sLCD.sScreenNow, NULL, 0xF1);
            break;
        case _LCD_SCR_SETTING_2:
            if (sLCD.sScreenNow.Para_u8 > sLCD.sScreenNow.ParaMin_u8 ) {
                sLCD.sScreenNow.Para_u8--;
                Display_Set_Screen_Flag(&sLCD.sScreenNow, NULL, 0xF1);
            } else {
                Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SETTING, 0,
                                       __SET_PRESS_CALIB, __SET_REQ_1, __SET_PRESS_CALIB,
                                       NULL, 0xF1);
            }
            break;
        case _LCD_SCR_SET_PULSE_1:
        case _LCD_SCR_PULSE_RESET: 
            if (sLCD.sScreenNow.Para_u8 > sLCD.sScreenNow.ParaMin_u8) {
                sLCD.sScreenNow.Para_u8--;
            } else {
                sLCD.sScreenNow.Para_u8 = sLCD.sScreenNow.ParaMax_u8;
            }
            Display_Set_Screen_Flag(&sLCD.sScreenNow, NULL, 0xF1);
            break;
        case _LCD_SCR_CAL_CHANN_2:
            if (sLCD.sScreenNow.Para_u8 > sLCD.sScreenNow.ParaMin_u8 ) {
                sLCD.sScreenNow.Para_u8--;
                Display_Set_Screen_Flag(&sLCD.sScreenNow, NULL, 0xF1);
            } else {
                Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_CAL_CHANN_1, 0,
                                       __CAL_PRESS_CH_5, __CAL_PRESS_CH_1, __CAL_PRESS_CH_5,
                                       NULL, 0xF1);
            }
            break;
        case _LCD_SCR_SET_FREQ:
            sButton.Old_value ++; 
            break;
            
         case _LCD_SCR_SET_OPTION_SENSOR:
              if(sLCD.sScreenNow.SubIndex_u8 == 0)
              {
                    if (sLCD.sScreenNow.Para_u8 > sLCD.sScreenNow.ParaMin_u8 ) {
                        sLCD.sScreenNow.Para_u8--;
                    }
                    Display_Set_Screen_Flag(&sLCD.sScreenNow, NULL, 0xF1);
              }
              else
              {
                switch (sLCD.sScreenNow.Para_u8)
                {
                    case __SET_OPTION_SS_PH:
                    case __SET_OPTION_SS_CLO:
                    case __SET_OPTION_SS_EC:
                    case __SET_OPTION_SS_TURB:
                    case __SET_OPTION_SS_COD:
                        switch(sLCD.sScreenNow.SubIndex_u8)
                        {
                            case 0:
                                break;
                                
                            case 1:
                                if(sButton.Old_value == 0)
                                  sButton.Old_value = 1;
                                else
                                  sButton.Old_value = 0;
                                break;
                            
                            default:
                                break;
                        }
                        break;
                        
                    default:
                      break;
                }
            }
            break;
            
         case _LCD_SCR_SET_OPTION_SENSOR_TAB_2:
              if(sLCD.sScreenNow.SubIndex_u8 == 0)
              {
                    if (sLCD.sScreenNow.Para_u8 > sLCD.sScreenNow.ParaMin_u8 ) {
                        sLCD.sScreenNow.Para_u8--;
                        Display_Set_Screen_Flag(&sLCD.sScreenNow, NULL, 0xF1);
                    } else {
                        Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_OPTION_SENSOR, 0,
                                               __SET_OPTION_SS_COD, __SET_OPTION_SS_PH, __SET_OPTION_SS_COD,
                                               NULL, 0xF1);
                    }
              }
              else
              {
                switch (sLCD.sScreenNow.Para_u8)
                {
                    case __SET_OPTION_SS_TSS:
                    case __SET_OPTION_SS_NH4:
                    case __SET_OPTION_SS_DO:
                    case __SET_OPTION_SS_SALT:
                    case __SET_OPTION_SS_TDS:
                        switch(sLCD.sScreenNow.SubIndex_u8)
                        {
                            case 0:
                                break;
                                
                            case 1:
                                if(sButton.Old_value == 0)
                                  sButton.Old_value = 1;
                                else
                                  sButton.Old_value = 0;
                                break;
                            
                            default:
                                break;
                        }
                        break;
                        
                    default:
                      break;
                }
            }
            break;
            
         case _LCD_SCR_SET_OPTION_SENSOR_TAB_3:
              if(sLCD.sScreenNow.SubIndex_u8 == 0)
              {
                    if (sLCD.sScreenNow.Para_u8 > sLCD.sScreenNow.ParaMin_u8 ) {
                        sLCD.sScreenNow.Para_u8--;
                        Display_Set_Screen_Flag(&sLCD.sScreenNow, NULL, 0xF1);
                    } else {
                        Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_OPTION_SENSOR_TAB_2, 0,
                                               __SET_OPTION_SS_TDS, __SET_OPTION_SS_TSS, __SET_OPTION_SS_TDS,
                                               NULL, 0xF1);
                    }
              }
              else
              {
                switch (sLCD.sScreenNow.Para_u8)
                {
                    case __SET_OPTION_SS_NO3:
                        switch(sLCD.sScreenNow.SubIndex_u8)
                        {
                            case 0:
                                break;
                                
                            case 1:
                                if(sButton.Old_value == 0)
                                  sButton.Old_value = 1;
                                else
                                  sButton.Old_value = 0;
                                break;
                            
                            default:
                                break;
                        }
                        break;
                        
                    default:
                      break;
                }
            }
            break;
            
        case _LCD_SCR_SET_PRESS_1_2:
            if (sLCD.sScreenNow.Para_u8 > sLCD.sScreenNow.ParaMin_u8 ) {
                sLCD.sScreenNow.Para_u8--;
                Display_Set_Screen_Flag(&sLCD.sScreenNow, NULL, 0xF1);
            } else {
                Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_PRESS_1, 0,
                                    __SET_PRESSURE_1_5, __SET_PRESSURE_1_1, __SET_PRESSURE_1_5,
                                    NULL, 0xF1);
            }
            break;
        case _LCD_SCR_SET_PRESS_2:
             Display_Process_Up_Pressure_2(&sPressConfig);
            break;
        case _LCD_SCR_SET_PULSE_2:
            if (sLCD.rSett_u8 == false) {
                if (sLCD.sScreenNow.Para_u8 > sLCD.sScreenNow.ParaMin_u8) {
                    sLCD.sScreenNow.Para_u8--;
                }
                
                Display_Set_Screen_Flag(&sLCD.sScreenNow, NULL, 0xF3);
                break;
            }

            if (aTEST[sButton.Old_value] == '.') {
                aTEST[sButton.Old_value] = '_';
                break;
            }
            
            if (aTEST[sButton.Old_value] == '_') {
                aTEST[sButton.Old_value] = '0';
                break;
            }
    
            if (aTEST[sButton.Old_value] < '9') {
                aTEST[sButton.Old_value] ++;
            } else {
                aTEST[sButton.Old_value] = '.';
            }
            break;
            
        case _LCD_SCR_CALIB_SS:    
        case _LCD_SCR_RS_CONFIRM: 
        case _LCD_SCR_RESTORE_DEV:
            sButton.Old_value = 1 - sButton.Old_value;
            sLCDObject[sLCD.sScreenNow.Para_u8].pData = sConfirm[sButton.Old_value];  
            break;
        case _LCD_SCR_LEV_SET_2:
            if (sLCD.sScreenNow.Para_u8 == __SET_LEV_SET2_1) {
                if (aTEST[sButton.Old_value] == '.') {
                    aTEST[sButton.Old_value] = '_';
                    break;
                }
                
                if (aTEST[sButton.Old_value] == '_') {
                    aTEST[sButton.Old_value] = '0';
                    break;
                }
        
                if (aTEST[sButton.Old_value] < '9') {
                    aTEST[sButton.Old_value] ++;
                } else {
                    aTEST[sButton.Old_value] = '.';
                }
            }
            break;
        default:
            break;
    }
}


void BUTTON_Down_Process (void)
{
    switch (sLCD.sScreenNow.Index_u8)
    {
//        case _LCD_SCREEN_1:
        case _LCD_SCREEN_1:
            sLCD.sScreenNow.Index_u8 = _LCD_SCREEN_CM44;
            break;
            
        case _LCD_SCREEN_CM44:
            if(((sLCDPageSensor.Number + 5) /6) > 1) 
                sLCD.sScreenNow.Index_u8 = _LCD_SCREEN_CM44_2;
            else
                sLCD.sScreenNow.Index_u8 = _LCD_SCREEN_1;
            break;
            
        case _LCD_SCREEN_CM44_2:
            if(((sLCDPageSensor.Number + 5) /6) > 2) 
                sLCD.sScreenNow.Index_u8 = _LCD_SCREEN_CM44_3;
            else
                sLCD.sScreenNow.Index_u8 = _LCD_SCREEN_1;
            break;
            
        case _LCD_SCREEN_CM44_3:
                sLCD.sScreenNow.Index_u8 = _LCD_SCREEN_1;
            break;      
        case _LCD_SCREEN_2:
        case _LCD_SCREEN_3:
        case _LCD_SCREEN_4:
        case _LCD_SCREEN_5:
        case _LCD_SCREEN_6:
        case _LCD_SCREEN_7:
            sLCD.sScreenNow.Index_u8 ++;
            sLCD.sScreenNow.SubIndex_u8 = 0;
            sButton.Old_value = _LCD_SCREEN_7;
            break;
        case _LCD_SCREEN_7b:
            sButton.Old_value = _LCD_SCREEN_7;
            if ( (sLCD.sScreenNow.SubIndex_u8 + 1) >= sWmDigVar.nModbus_u8 ) {
                sLCD.sScreenNow.SubIndex_u8 = 0;
                sLCD.sScreenNow.Index_u8 = _LCD_SCREEN_1;
            } else {
                sLCD.sScreenNow.SubIndex_u8++;
            }
            break;
        case _LCD_SCR_PASS:
            aTEST[sButton.Old_value] --;
            if (aTEST[sButton.Old_value] < '0') {
                aTEST[sButton.Old_value] = '9';
            }
            break;
        case _LCD_SCR_SETTING:
            if (sLCD.sScreenNow.Para_u8 == sLCD.sScreenNow.ParaMax_u8) {
                Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SETTING_2, 0,
                                    __SET_LEVEL_SETT, __SET_LEVEL_SETT, __SET_MANUFACTOR,
                                    NULL, 0xF1);
            } else {
                sLCD.sScreenNow.Para_u8++;
                Display_Set_Screen_Flag(&sLCD.sScreenNow, NULL, 0xF1);
            }
            break;
        case _LCD_SCR_SETTING_2:
        case _LCD_SCR_SET_PRESS_1_2:
        case _LCD_SCR_CAL_CHANN_2:
        case _LCD_SCR_LEV_SET_1:
            if (sLCD.sScreenNow.Para_u8 < sLCD.sScreenNow.ParaMax_u8) {
                sLCD.sScreenNow.Para_u8++;
            }
            Display_Set_Screen_Flag(&sLCD.sScreenNow, NULL, 0xF1);
            break;
        case _LCD_SCR_SET_PULSE_1:   
        case _LCD_SCR_PULSE_RESET: 
            if (sLCD.sScreenNow.Para_u8 < sLCD.sScreenNow.ParaMax_u8) {
                sLCD.sScreenNow.Para_u8++;
            } else {
                sLCD.sScreenNow.Para_u8 = sLCD.sScreenNow.ParaMin_u8;
            }
            Display_Set_Screen_Flag(&sLCD.sScreenNow, NULL, 0xF1);
            break;
        case _LCD_SCR_CAL_CHANN_1:
            if (sLCD.sScreenNow.Para_u8 == sLCD.sScreenNow.ParaMax_u8) {
                Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_CAL_CHANN_2, 0,
                                    __CAL_PRESS_CH_6, __CAL_PRESS_CH_6, __CAL_PRESS_CH_6,
                                    NULL, 0xF1);
            } else {
                sLCD.sScreenNow.Para_u8++;
                Display_Set_Screen_Flag(&sLCD.sScreenNow, NULL, 0xF1);
            }
            break;
        case _LCD_SCR_SET_FREQ:
            if (sButton.Old_value > 0) {
                sButton.Old_value --;
            }
            break; 
            
        case _LCD_SCR_SET_OPTION_SENSOR:
              if(sLCD.sScreenNow.SubIndex_u8 == 0)
              {
                    if (sLCD.sScreenNow.Para_u8 == sLCD.sScreenNow.ParaMax_u8) {
                        Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_OPTION_SENSOR_TAB_2, 0,
                                            __SET_OPTION_SS_TSS, __SET_OPTION_SS_TSS, __SET_OPTION_SS_TDS,
                                            NULL, 0xF1);
                    } else {
                        sLCD.sScreenNow.Para_u8++;
                        Display_Set_Screen_Flag(&sLCD.sScreenNow, NULL, 0xF1);
                    }
              }
              else
              {
                switch (sLCD.sScreenNow.Para_u8)
                {
                    case __SET_OPTION_SS_PH:
                    case __SET_OPTION_SS_CLO:
                    case __SET_OPTION_SS_EC:
                    case __SET_OPTION_SS_TURB:
                    case __SET_OPTION_SS_COD:
                        switch(sLCD.sScreenNow.SubIndex_u8)
                        {
                            case 0:
                                break;
                                
                            case 1:
                                if(sButton.Old_value == 0)
                                  sButton.Old_value = 1;
                                else
                                  sButton.Old_value = 0;
                                break;
                            
                            default:
                                break;
                        }
                        break;
                        
                    default:
                      break;
                }
            }
            break;
            
        case _LCD_SCR_SET_OPTION_SENSOR_TAB_2:
              if(sLCD.sScreenNow.SubIndex_u8 == 0)
              {
                    if (sLCD.sScreenNow.Para_u8 == sLCD.sScreenNow.ParaMax_u8) {
                        Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_OPTION_SENSOR_TAB_3, 0,
                                            __SET_OPTION_SS_NO3, __SET_OPTION_SS_NO3, __SET_OPTION_SS_NO3,
                                            NULL, 0xF1);
                    } else {
                        sLCD.sScreenNow.Para_u8++;
                        Display_Set_Screen_Flag(&sLCD.sScreenNow, NULL, 0xF1);
                    }
              }
              else
              {
                switch (sLCD.sScreenNow.Para_u8)
                {
                    case __SET_OPTION_SS_TSS:
                    case __SET_OPTION_SS_NH4:
                    case __SET_OPTION_SS_DO:
                    case __SET_OPTION_SS_SALT:
                    case __SET_OPTION_SS_TDS:
                        switch(sLCD.sScreenNow.SubIndex_u8)
                        {
                            case 0:
                                break;
                                
                            case 1:
                                if(sButton.Old_value == 0)
                                  sButton.Old_value = 1;
                                else
                                  sButton.Old_value = 0;
                                break;
                            
                            default:
                                break;
                        }
                        break;
                        
                    default:
                      break;
                }
            }
            break;
            
        case _LCD_SCR_SET_OPTION_SENSOR_TAB_3:
              if(sLCD.sScreenNow.SubIndex_u8 == 0)
              {
                    if (sLCD.sScreenNow.Para_u8 < sLCD.sScreenNow.ParaMax_u8 ) {
                        sLCD.sScreenNow.Para_u8++;
                    }
                    Display_Set_Screen_Flag(&sLCD.sScreenNow, NULL, 0xF1);
              }
              else
              {
                switch (sLCD.sScreenNow.Para_u8)
                {
                    case __SET_OPTION_SS_NO3:
                        switch(sLCD.sScreenNow.SubIndex_u8)
                        {
                            case 0:
                                break;
                                
                            case 1:
                                if(sButton.Old_value == 0)
                                  sButton.Old_value = 1;
                                else
                                  sButton.Old_value = 0;
                                break;
                            
                            default:
                                break;
                        }
                        break;
                        
                    default:
                      break;
                }
            }
            break;
            
        case _LCD_SCR_SET_PRESS_1:
            if (sLCD.sScreenNow.Para_u8 == sLCD.sScreenNow.ParaMax_u8) {
                Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_PRESS_1_2, 0,
                                    __SET_PRESSURE_1_6, __SET_PRESSURE_1_6, __SET_PRESSURE_1_6,
                                    NULL, 0xF1);
            } else {
                sLCD.sScreenNow.Para_u8++;
                Display_Set_Screen_Flag(&sLCD.sScreenNow, NULL, 0xF1);
            }
            break;
        case _LCD_SCR_SET_PRESS_2:
            Display_Process_Down_Pressure_2(&sPressConfig);
            break;
        case _LCD_SCR_SET_PULSE_2:
            if (sLCD.rSett_u8 == false) {
                if (sLCD.sScreenNow.Para_u8 <= sLCD.sScreenNow.ParaMax_u8) {
                    sLCD.sScreenNow.Para_u8++;
                    Display_Set_Screen_Flag(&sLCD.sScreenNow, NULL, 0xF3);
                } else {
                    Display_Set_Screen_Flag(&sLCD.sScreenNow, NULL, 0xF0);
                }

                break;
            }
            
            if (aTEST[sButton.Old_value] == '_') {
                aTEST[sButton.Old_value] = '.';
                break;
            }
            
            if (aTEST[sButton.Old_value] == '.') {
                aTEST[sButton.Old_value] = '9';
                break;
            }
    
            if (aTEST[sButton.Old_value] > '0') {
                aTEST[sButton.Old_value] --;
            } else {
                aTEST[sButton.Old_value] = '_';
            }
            break;
            
        case _LCD_SCR_CALIB_SS:
        case _LCD_SCR_RS_CONFIRM: 
        case _LCD_SCR_RESTORE_DEV:
            sButton.Old_value = 1 - sButton.Old_value;
            sLCDObject[sLCD.sScreenNow.Para_u8].pData = sConfirm[sButton.Old_value];  
            break;
        case _LCD_SCR_LEV_SET_2:
            if (sLCD.sScreenNow.Para_u8 == __SET_LEV_SET2_1) {
                if (aTEST[sButton.Old_value] == '_') {
                    aTEST[sButton.Old_value] = '.';
                    break;
                }
                
                if (aTEST[sButton.Old_value] == '.') {
                    aTEST[sButton.Old_value] = '9';
                    break;
                }
        
                if (aTEST[sButton.Old_value] > '0') {
                    aTEST[sButton.Old_value] --;
                } else {
                    aTEST[sButton.Old_value] = '_';
                }
            }
            break;
        default:
            break;
    }
}

void BUTTON_ESC_Process (void)
{
    switch (sLCD.sScreenNow.Index_u8)
    {
        case _LCD_SCREEN_CM44:
        case _LCD_SCREEN_CM44_2:
        case _LCD_SCREEN_CM44_3:
            sLCD.sScreenNow.Index_u8 = _LCD_SCREEN_1;
            break;
      
        case _LCD_SCR_PASS:
            sLCD.sScreenNow.Index_u8 = _LCD_SCREEN_1;
            break;
        case _LCD_SCR_SETTING:
        case _LCD_SCR_SETTING_2:
            sLCD.sScreenNow.Index_u8 = _LCD_SCREEN_1;
            break;

        case _LCD_SCR_SET_OPTION_SENSOR:   
          switch(sLCD.sScreenNow.Para_u8)
          {
            case __SET_OPTION_SS_PH:
                switch(sLCD.sScreenNow.SubIndex_u8)
                {
                    case 0:
                        Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SETTING_2, 0,
                                           __SET_OPTION_SENSOR, __SET_LEVEL_SETT, __SET_MANUFACTOR,
                                           NULL, 0xF1);
                        UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                        break;        
                      
                    case 1:
                        UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                        Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_OPTION_SENSOR, (sLCD.sScreenNow.SubIndex_u8-1),
                                           __SET_OPTION_SS_PH, __SET_OPTION_SS_PH, __SET_OPTION_SS_COD,
                                           &s485Measure[_SS_PH].sUser, 0xF1);
                        break;
                        
                    default:
                        break;
                }
                break;
                
            case __SET_OPTION_SS_CLO:
                switch(sLCD.sScreenNow.SubIndex_u8)
                {
                    case 0:
                        Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SETTING_2, 0,
                                           __SET_OPTION_SENSOR, __SET_LEVEL_SETT, __SET_MANUFACTOR,
                                           NULL, 0xF1);
                        UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                        break;        
                      
                    case 1:
                        UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                        Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_OPTION_SENSOR, (sLCD.sScreenNow.SubIndex_u8-1),
                                           __SET_OPTION_SS_CLO, __SET_OPTION_SS_PH, __SET_OPTION_SS_COD,
                                           &s485Measure[_SS_CLO].sUser, 0xF1);
                        break;
                        
                    default:
                        break;
                }
                break;
                
            case __SET_OPTION_SS_EC:
                switch(sLCD.sScreenNow.SubIndex_u8)
                {
                    case 0:
                        Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SETTING_2, 0,
                                           __SET_OPTION_SENSOR, __SET_LEVEL_SETT, __SET_MANUFACTOR,
                                           NULL, 0xF1);
                        UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                        break;        
                      
                    case 1:
                        UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                        Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_OPTION_SENSOR, (sLCD.sScreenNow.SubIndex_u8-1),
                                           __SET_OPTION_SS_EC, __SET_OPTION_SS_PH, __SET_OPTION_SS_COD,
                                           &s485Measure[_SS_EC].sUser, 0xF1);
                        break;
                        
                    default:
                        break;
                }
                break;
                
            case __SET_OPTION_SS_TURB:
                switch(sLCD.sScreenNow.SubIndex_u8)
                {
                    case 0:
                        Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SETTING_2, 0,
                                           __SET_OPTION_SENSOR, __SET_LEVEL_SETT, __SET_MANUFACTOR,
                                           NULL, 0xF1);
                        UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                        break;        
                      
                    case 1:
                        UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                        Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_OPTION_SENSOR, (sLCD.sScreenNow.SubIndex_u8-1),
                                           __SET_OPTION_SS_TURB, __SET_OPTION_SS_PH, __SET_OPTION_SS_COD,
                                           &s485Measure[_SS_TURB].sUser, 0xF1);
                        break;
                        
                    default:
                        break;
                }
                break;
                
            case __SET_OPTION_SS_COD:
                switch(sLCD.sScreenNow.SubIndex_u8)
                {
                    case 0:
                        Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SETTING_2, 0,
                                           __SET_OPTION_SENSOR, __SET_LEVEL_SETT, __SET_MANUFACTOR,
                                           NULL, 0xF1);
                        UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                        break;        
                      
                    case 1:
                        UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                        Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_OPTION_SENSOR, (sLCD.sScreenNow.SubIndex_u8-1),
                                           __SET_OPTION_SS_COD, __SET_OPTION_SS_PH, __SET_OPTION_SS_COD,
                                           &s485Measure[_SS_COD].sUser, 0xF1);
                        break;
                        
                    default:
                        break;
                }
                break;
                
              default:
                break;
          }
          break;
          
        case _LCD_SCR_SET_OPTION_SENSOR_TAB_2:   
          switch(sLCD.sScreenNow.Para_u8)
          {
            case __SET_OPTION_SS_TSS:
                switch(sLCD.sScreenNow.SubIndex_u8)
                {
                    case 0:
                        Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SETTING_2, 0,
                                           __SET_OPTION_SENSOR, __SET_LEVEL_SETT, __SET_MANUFACTOR,
                                           NULL, 0xF1);
                        UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                        break;        
                      
                    case 1:
                        UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                        Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_OPTION_SENSOR_TAB_2, (sLCD.sScreenNow.SubIndex_u8-1),
                                           __SET_OPTION_SS_TSS, __SET_OPTION_SS_TSS, __SET_OPTION_SS_TDS,
                                           &s485Measure[_SS_TSS].sUser, 0xF1);
                        break;
                        
                    default:
                        break;
                }
                break;
                
            case __SET_OPTION_SS_NH4:
                switch(sLCD.sScreenNow.SubIndex_u8)
                {
                    case 0:
                        Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SETTING_2, 0,
                                           __SET_OPTION_SENSOR, __SET_LEVEL_SETT, __SET_MANUFACTOR,
                                           NULL, 0xF1);
                        UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                        break;        
                      
                    case 1:
                        UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                        Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_OPTION_SENSOR_TAB_2, (sLCD.sScreenNow.SubIndex_u8-1),
                                           __SET_OPTION_SS_NH4, __SET_OPTION_SS_TSS, __SET_OPTION_SS_TDS,
                                           &s485Measure[_SS_NH4].sUser, 0xF1);
                        break;
                        
                    default:
                        break;
                }
                break;
                
            case __SET_OPTION_SS_DO:
                switch(sLCD.sScreenNow.SubIndex_u8)
                {
                    case 0:
                        Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SETTING_2, 0,
                                           __SET_OPTION_SENSOR, __SET_LEVEL_SETT, __SET_MANUFACTOR,
                                           NULL, 0xF1);
                        UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                        break;        
                      
                    case 1:
                        UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                        Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_OPTION_SENSOR_TAB_2, (sLCD.sScreenNow.SubIndex_u8-1),
                                           __SET_OPTION_SS_DO, __SET_OPTION_SS_TSS, __SET_OPTION_SS_TDS,
                                           &s485Measure[_SS_DO].sUser, 0xF1);
                        break;
                        
                    default:
                        break;
                }
                break;
                
            case __SET_OPTION_SS_SALT:
                switch(sLCD.sScreenNow.SubIndex_u8)
                {
                    case 0:
                        Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SETTING_2, 0,
                                           __SET_OPTION_SENSOR, __SET_LEVEL_SETT, __SET_MANUFACTOR,
                                           NULL, 0xF1);
                        UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                        break;        
                      
                    case 1:
                        UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                        Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_OPTION_SENSOR_TAB_2, (sLCD.sScreenNow.SubIndex_u8-1),
                                           __SET_OPTION_SS_SALT, __SET_OPTION_SS_TSS, __SET_OPTION_SS_TDS,
                                           &s485Measure[_SS_SALT].sUser, 0xF1);
                        break;
                        
                    default:
                        break;
                }
                break;
                
            case __SET_OPTION_SS_TDS:
                switch(sLCD.sScreenNow.SubIndex_u8)
                {
                    case 0:
                        Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SETTING_2, 0,
                                           __SET_OPTION_SENSOR, __SET_LEVEL_SETT, __SET_MANUFACTOR,
                                           NULL, 0xF1);
                        UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                        break;        
                      
                    case 1:
                        UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                        Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_OPTION_SENSOR_TAB_2, (sLCD.sScreenNow.SubIndex_u8-1),
                                           __SET_OPTION_SS_TDS, __SET_OPTION_SS_TSS, __SET_OPTION_SS_TDS,
                                           &s485Measure[_SS_TDS].sUser, 0xF1);
                        break;
                        
                    default:
                        break;
                }
                break;
                
              default:
                break;
          }
          break;
          
        case _LCD_SCR_SET_OPTION_SENSOR_TAB_3:   
          switch(sLCD.sScreenNow.Para_u8)
          {
            case __SET_OPTION_SS_NO3:
                switch(sLCD.sScreenNow.SubIndex_u8)
                {
                    case 0:
                        Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SETTING_2, 0,
                                           __SET_OPTION_SENSOR, __SET_LEVEL_SETT, __SET_MANUFACTOR,
                                           NULL, 0xF1);
                        UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                        break;        
                      
                    case 1:
                        UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                        Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_OPTION_SENSOR_TAB_3, (sLCD.sScreenNow.SubIndex_u8-1),
                                           __SET_OPTION_SS_NO3, __SET_OPTION_SS_NO3, __SET_OPTION_SS_NO3,
                                           &s485Measure[_SS_NO3].sUser, 0xF1);
                        break;
                        
                    default:
                        break;
                }
                break;
 
              default:
                break;
          }
          break;
            
        case _LCD_SCR_CHECK_SETTING:
          if(sHandleRs485.State_Wait_Calib != _STATE_CALIB_WAIT)
          {
            UTIL_MEM_cpy(&sLCD.sScreenNow, &sLCD.sScreenBack, sizeof(sScreenInformation));
            sHandleRs485.State_Wait_Calib = _STATE_CALIB_FREE;
          }
          break; 
            
        case _LCD_SCR_SET_PRESS_1:
        case _LCD_SCR_SET_PRESS_1_2:
            sLCD.sScreenNow.Index_u8 = _LCD_SCR_SETTING; 
            sLCD.sScreenNow.Para_u8 = __SET_PRESSURE;
            sLCD.sScreenNow.ParaMin_u8 = __SET_REQ_1;
            sLCD.sScreenNow.ParaMax_u8 = __SET_PRESS_CALIB;
            break;
        case _LCD_SCR_SET_PULSE_1:
            sLCD.sScreenNow.Index_u8 = _LCD_SCR_SETTING; 
            sLCD.sScreenNow.Para_u8 = __SET_PULSE_SETT;
            sLCD.sScreenNow.ParaMin_u8 = __SET_REQ_1;
            sLCD.sScreenNow.ParaMax_u8 = __SET_PRESS_CALIB;
            break;
        case _LCD_SCR_PULSE_RESET: 
            sLCD.sScreenNow.Index_u8 = _LCD_SCR_SETTING; 
            sLCD.sScreenNow.Para_u8 = __SET_PULSE_RESET;
            sLCD.sScreenNow.ParaMin_u8 = __SET_REQ_1;
            sLCD.sScreenNow.ParaMax_u8 = __SET_PRESS_CALIB;
            break;
        case _LCD_SCR_SET_FREQ:
        case _LCD_SCR_SET_PRESS_2:
        case _LCD_SCR_SET_SUCCESS:
        case _LCD_SCREEN_8:
        case _LCD_SCR_CALIB_SS:
        case _LCD_SCR_RS_CONFIRM:
        case _LCD_SCR_LEV_SET_2:
        case _LCD_SCR_RESTORE_DEV:
            UTIL_MEM_cpy(&sLCD.sScreenNow, &sLCD.sScreenBack, sizeof(sScreenInformation));
            break;
        case _LCD_SCR_SET_PULSE_2:
            UTIL_MEM_cpy(&sLCD.sScreenNow, &sLCD.sScreenBack, sizeof(sScreenInformation));
            break;
        case _LCD_SCR_CAL_CHANN_1:
        case _LCD_SCR_CAL_CHANN_2:
            sLCD.sScreenNow.Index_u8 = _LCD_SCR_SETTING; 
            sLCD.sScreenNow.Para_u8 = __SET_PRESS_CALIB;
            sLCD.sScreenNow.ParaMin_u8 = __SET_REQ_1;
            sLCD.sScreenNow.ParaMax_u8 = __SET_PRESS_CALIB;
            break;
        case _LCD_SCR_CAL_PRESS:
            //lay lai gia tri calib trươc
            sWmVar.aPRESSURE[sButton.Old_value].Calib_i16 = PressOldCalib_i16;
            UTIL_MEM_cpy(&sLCD.sScreenNow, &sLCD.sScreenBack, sizeof(sScreenInformation));
            break;
        case _LCD_SCR_LEV_SET_1:
            sLCD.sScreenNow.Index_u8 = _LCD_SCR_SETTING_2; 
            sLCD.sScreenNow.Para_u8 = __SET_LEVEL_SETT;
            sLCD.sScreenNow.ParaMin_u8 = __SET_LEVEL_SETT;
            sLCD.sScreenNow.ParaMax_u8 = __SET_MANUFACTOR;
        default:
            break;
    }
}


/*
    Func: check end sett string
    return  true: finish -> get data
            false: continue position sett
*/
uint8_t Button_Is_End_Sett_Str (void)
{
    if ( (sButton.Old_value >= 15) || (aTEST[sButton.Old_value] == '_') ) {
        if (aTEST[sButton.Old_value] == '_') {
            aTEST[sButton.Old_value] = 0;
        }
        return true;
    } else {  
        sButton.Old_value++; 
        aTEST[sButton.Old_value] = '_';
        Display_Set_Screen_Flag(&sLCD.sScreenNow, NULL, (sButton.Old_value << 4) + 0x02);
    }

    return false;
}

