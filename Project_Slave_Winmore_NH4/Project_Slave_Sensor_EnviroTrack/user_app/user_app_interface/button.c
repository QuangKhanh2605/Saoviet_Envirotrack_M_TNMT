

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
    static uint16_t block_time = 0;

    if (block_time > 0) {
        block_time--;
        return 0;
    }

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

                if (sButton.Count >= BUT_TIME_DETECT)
                {
                    sButton.Value = r_value;
                    sButton.Status = 1;

                    if (hold < BUT_COUNT_HOLD)
                        block_time = 25;

                    if ( r_value == last_value )
                    {
                        hold++;
                        
                        if (hold < BUT_COUNT_FAST/2) {
                            sButton.Count = 0;
                        }
                        else if (hold < BUT_COUNT_FAST*3) {
                            sButton.Count = BUT_TIME_DETECT - 15; 
                        }
                        else if (hold < BUT_COUNT_FAST*4) {
                            sButton.Count = BUT_TIME_DETECT - 10; 
                        }
                        else {
                            sButton.Count = BUT_TIME_DETECT - 2;                
                            hold = BUT_COUNT_FAST*4;               
                        }
                    }
                    else
                    {
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

//uint8_t BUTTON_scan(void)
//{
//    uint8_t r_value = 0;
//    static uint8_t last_value = 0;
//    static uint32_t hold = 0;
//    
//	if (sButton.Status == 0)
//	{
//		r_value = (!BUTTON_ENTER<<0)|(!BUTTON_UP<<1)|(!BUTTON_DOWN<<2)|(!BUTTON_ESC<<3);
//        
//		switch(r_value)
//		{
//			case _ENTER:
//            case _UP:
//            case _DOWN:
//            case _ESC:
//                sButton.Count++;
//                if (sButton.Count >= BUT_TIME_DETECT) {
//                    sButton.Value = r_value;
//                    sButton.Status = 1;
//                    
//                    if ( r_value == last_value ) {
//                        hold++;
//                        if ( hold >= BUT_COUNT_FAST ) {
//                            sButton.Count = BUT_TIME_DETECT;
//                            hold = BUT_COUNT_FAST;
//                        } else if (hold >= BUT_COUNT_HOLD) {
//                            sButton.Count = BUT_TIME_DETECT - 2;
//                        } else {
//                            sButton.Count = 0;
//                        }
//                    } else {
//                        sButton.Count = 0;  
//                        hold = 0;
//                    } 
//                                
//                    last_value = r_value;
//                }
//  				break;
//			default:
//                last_value = 0;
//				sButton.Count = BUT_TIME_DETECT/2;
//                hold = 0;
//				break;
//		}
//	}
//
//    return sButton.Value;
//}

static char aTEST[10] = {"1234"};
//static sPressureLinearInter sPressConfig = {0};
//static Struct_Pulse sPulseConfig = {0};
//static int16_t PressOldCalib_i16 = 0;
//static char sConfirm[2][10] = {"No", "Yes"};

float       Old_Value_f = 0;
uint32_t    Old_Value_u32 = 0;
int16_t     Old_Value_i16 = 0;
float       Button_Stamp1_f = 0;
float       Button_Stamp2_f = 0;
      
void BUTTON_Enter_Process (void)
{
//    static uint8_t tempu8 = 0;
//    static uint8_t TempScale = 1;
    switch (sLCD.sScreenNow.Index_u8)
    {
        case _LCD_SCREEN_1:
            sButton.Old_value = 0;
            UTIL_MEM_set(aTEST, 0, sizeof(aTEST));
            aTEST[0] = '0'; aTEST[1] = '0', aTEST[2] = '0', aTEST[3] = '0';
          
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
                                       __SCR_SET_MODBUS, __SCR_SET_MODBUS, __SCR_SET_INFOR,
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
                case __SCR_SET_MODBUS:
                    UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                    Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_MODBUS, 0,
                                       __SET_MODBUS_ID, __SET_MODBUS_ID, __SET_MODBUS_BR,
                                       NULL, 0xF1);
                    break;
                    
                case __SCR_SET_CALIB:
                    UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                    Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_MODE_CALIB, 0,
                                       __SET_MODE_CALIB_NH4, __SET_MODE_CALIB_NH4, __SET_MODE_CALIB_K,
                                       NULL, 0xF1);
                    break;
                    
                case __SCR_SET_OFFSET:
                    UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                    Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_OFFSET, 0,
                                       __SET_OFFSET_PH, __SET_OFFSET_PH, __SET_OFFSET_TEMP,
                                       NULL, 0xF1);
                    break;
                    
                case __SCR_SET_ALARM:
                    UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                    Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_ALARM, 0,
                                       __SET_ALARM_STATE, __SET_ALARM_STATE, __SET_ALARM_LOWER,
                                       NULL, 0xF1);
                    break;
                    
                case __SCR_SET_RANGE:
                    UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                    Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_RANGE, 0,
                                       __SET_RANGE_U_KEY, __SET_RANGE_U_KEY, __SET_RANGE_L_TEMP,
                                       NULL, 0xF1);
                    break;
                    
                case __SCR_SET_INFOR:
                    UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                    Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_INFORMATION, 0,
                                       __SCR_INFOR_FW_VERSION_2, __SCR_INFOR_FW_VERSION_2, __SCR_INFOR_FW_VERSION_2,
                                       NULL, 0x00);
                    break;
            }
            break;
            
        case _LCD_SCR_SET_MODBUS:
            switch (sLCD.sScreenNow.Para_u8)
            {
                case __SET_MODBUS_ID:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_MODBUS, (sLCD.sScreenNow.SubIndex_u8+1),
                                               __SET_MODBUS_ID, __SET_MODBUS_ID, __SET_MODBUS_BR,
                                               &sButton.Old_value, 0xF2);
                             sButton.Old_value = sParaDisplay.ID_u8;
                            break;
                            
                        case 1:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_CHECK_SETTING, 0,
                                               __CHECK_STATE_SETTING, __CHECK_STATE_SETTING, __CHECK_STATE_SETTING,
                                               NULL, 0xF0);
                            sParaDisplay.State_Setting = _STATE_SETTING_ENTER;
                            break;
                        
                        default:
                            break;
                    }
                    break;
                    
                case __SET_MODBUS_BR:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_MODBUS, (sLCD.sScreenNow.SubIndex_u8+1),
                                               __SET_MODBUS_BR, __SET_MODBUS_BR, __SET_MODBUS_BR,
                                               &sButton.Old_value, 0xF2);
                             sButton.Old_value = sParaDisplay.Baudrate_u32;
                            break;
                            
                        case 1:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_CHECK_SETTING, 0,
                                               __CHECK_STATE_SETTING, __CHECK_STATE_SETTING, __CHECK_STATE_SETTING,
                                               NULL, 0xF0);
                            sParaDisplay.State_Setting = _STATE_SETTING_ENTER;
                            break;
                        
                        default:
                            break;
                    }
                    break;
                   
            }
            break;
            
        case _LCD_SCR_SET_MODE_CALIB:
            switch (sLCD.sScreenNow.Para_u8)
            {
                case __SET_MODE_CALIB_NH4:
                    UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                    Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_CALIB_NH4, 0,
                                       __SET_CALIB_NH4_RESET, __SET_CALIB_NH4_RESET, __SET_CALIB_NH4_CONFIRM,
                                       NULL, 0xF1);
                    break;
                    
                case __SET_MODE_CALIB_PH:
                    UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                    Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_CALIB_PH, 0,
                                       __SET_CALIB_NH4_RESET, __SET_CALIB_NH4_RESET, __SET_CALIB_NH4_CONFIRM,
                                       NULL, 0xF1);
                    break;
                    
                case __SET_MODE_CALIB_K:
                    UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                    Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_CALIB_K, 0,
                                       __SET_CALIB_K_RESET, __SET_CALIB_K_RESET, __SET_CALIB_K_CONFIRM,
                                       NULL, 0xF1);
                    break;
                    
                default:
                  break;
            }
            break;
            
        case _LCD_SCR_CALIB_NH4:
            switch (sLCD.sScreenNow.Para_u8)
            {
                case __SET_CALIB_NH4_RESET:
                    UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                    Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_CHECK_SETTING, 0,
                                       __CHECK_STATE_SETTING, __CHECK_STATE_SETTING, __CHECK_STATE_SETTING,
                                       NULL, 0xF0);

                    Old_Value_i16 = 0x0000;
                    RS485_LogData_Calib(_RS485_SS_NH4_CALIB_686, &Old_Value_i16, sizeof(int16_t));
                    break;
              
                case __SET_CALIB_NH4_P1:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_CALIB_NH4, (sLCD.sScreenNow.SubIndex_u8+1),
                                               __SET_CALIB_NH4_P1, __SET_CALIB_NH4_RESET, __SET_CALIB_NH4_CONFIRM,
                                               &sButton.Old_value, 0xF2);
                             sButton.Old_value = sParaDisplay.NH4_P1_i32;
                            break;
                            
                        case 1:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_CHECK_SETTING, 0,
                                               __CHECK_STATE_SETTING, __CHECK_STATE_SETTING, __CHECK_STATE_SETTING,
                                               NULL, 0xF0);
                            sParaDisplay.State_Setting = _STATE_SETTING_ENTER;
                            break;
                        
                        default:
                            break;
                    }
                    break;
                    
                case __SET_CALIB_NH4_P2:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_CALIB_NH4, (sLCD.sScreenNow.SubIndex_u8+1),
                                               __SET_CALIB_NH4_P2, __SET_CALIB_NH4_RESET, __SET_CALIB_NH4_CONFIRM,
                                               &sButton.Old_value, 0xF2);
                             sButton.Old_value = sParaDisplay.NH4_P2_i32;
                            break;
                            
                        case 1:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_CHECK_SETTING, 0,
                                               __CHECK_STATE_SETTING, __CHECK_STATE_SETTING, __CHECK_STATE_SETTING,
                                               NULL, 0xF0);
                            sParaDisplay.State_Setting = _STATE_SETTING_ENTER;
                            break;
                        
                        default:
                            break;
                    }
                    break;
                    
                case __SET_CALIB_NH4_CONFIRM:
                    UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                    Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_CHECK_SETTING, 0,
                                       __CHECK_STATE_SETTING, __CHECK_STATE_SETTING, __CHECK_STATE_SETTING,
                                       NULL, 0xF0);

                    Old_Value_i16 = 0x0000;
                    RS485_LogData_Calib(_RS485_SS_NH4_CALIB_686, &Old_Value_i16, sizeof(int16_t));
                    break;
                    
            }
            break;
            
        case _LCD_SCR_CALIB_PH:
            switch (sLCD.sScreenNow.Para_u8)
            {
                case __SET_CALIB_NH4_RESET:
                    UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                    Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_CHECK_SETTING, 0,
                                       __CHECK_STATE_SETTING, __CHECK_STATE_SETTING, __CHECK_STATE_SETTING,
                                       NULL, 0xF0);

                    Old_Value_i16 = 0x0000;
                    RS485_LogData_Calib(_RS485_SS_NH4_CALIB_686, &Old_Value_i16, sizeof(int16_t));
                    break;
              
                case __SET_CALIB_NH4_P1:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_CALIB_PH, (sLCD.sScreenNow.SubIndex_u8+1),
                                               __SET_CALIB_NH4_P1, __SET_CALIB_NH4_RESET, __SET_CALIB_NH4_CONFIRM,
                                               &sButton.Old_value, 0xF2);
                             sButton.Old_value = sParaDisplay.PH_P1_i32;
                            break;
                            
                        case 1:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_CHECK_SETTING, 0,
                                               __CHECK_STATE_SETTING, __CHECK_STATE_SETTING, __CHECK_STATE_SETTING,
                                               NULL, 0xF0);
                            sParaDisplay.State_Setting = _STATE_SETTING_ENTER;
                            break;
                        
                        default:
                            break;
                    }
                    break;
                    
                case __SET_CALIB_NH4_P2:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_CALIB_PH, (sLCD.sScreenNow.SubIndex_u8+1),
                                               __SET_CALIB_NH4_P2, __SET_CALIB_NH4_RESET, __SET_CALIB_NH4_CONFIRM,
                                               &sButton.Old_value, 0xF2);
                             sButton.Old_value = sParaDisplay.PH_P2_i32;
                            break;
                            
                        case 1:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_CHECK_SETTING, 0,
                                               __CHECK_STATE_SETTING, __CHECK_STATE_SETTING, __CHECK_STATE_SETTING,
                                               NULL, 0xF0);
                            sParaDisplay.State_Setting = _STATE_SETTING_ENTER;
                            break;
                        
                        default:
                            break;
                    }
                    break;
                    
                case __SET_CALIB_NH4_CONFIRM:
                    UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                    Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_CHECK_SETTING, 0,
                                       __CHECK_STATE_SETTING, __CHECK_STATE_SETTING, __CHECK_STATE_SETTING,
                                       NULL, 0xF0);

                    Old_Value_i16 = 0x0000;
                    RS485_LogData_Calib(_RS485_SS_NH4_CALIB_686, &Old_Value_i16, sizeof(int16_t));
                    break;
                    
            }
            break;
            
        case _LCD_SCR_CALIB_K:
            switch (sLCD.sScreenNow.Para_u8)
            {
                case __SET_CALIB_K_RESET:
                    UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                    Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_CHECK_SETTING, 0,
                                       __CHECK_STATE_SETTING, __CHECK_STATE_SETTING, __CHECK_STATE_SETTING,
                                       NULL, 0xF0);

                    Old_Value_i16 = 0x0000;
                    RS485_LogData_Calib(_RS485_SS_NH4_CALIB_686, &Old_Value_i16, sizeof(int16_t));
                    break;
              
                case __SET_CALIB_K_P1:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_CALIB_K, (sLCD.sScreenNow.SubIndex_u8+1),
                                               __SET_CALIB_K_P1, __SET_CALIB_K_RESET, __SET_CALIB_K_CONFIRM,
                                               &sButton.Old_value, 0xF2);
                             sButton.Old_value = sParaDisplay.K_P1_i32;
                            break;
                            
                        case 1:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_CHECK_SETTING, 0,
                                               __CHECK_STATE_SETTING, __CHECK_STATE_SETTING, __CHECK_STATE_SETTING,
                                               NULL, 0xF0);
                            sParaDisplay.State_Setting = _STATE_SETTING_ENTER;
                            break;
                        
                        default:
                            break;
                    }
                    break;
                    
                case __SET_CALIB_K_P2:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_CALIB_K, (sLCD.sScreenNow.SubIndex_u8+1),
                                               __SET_CALIB_K_P2, __SET_CALIB_K_RESET, __SET_CALIB_K_CONFIRM,
                                               &sButton.Old_value, 0xF2);
                             sButton.Old_value = sParaDisplay.K_P2_i32;
                            break;
                            
                        case 1:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_CHECK_SETTING, 0,
                                               __CHECK_STATE_SETTING, __CHECK_STATE_SETTING, __CHECK_STATE_SETTING,
                                               NULL, 0xF0);
                            sParaDisplay.State_Setting = _STATE_SETTING_ENTER;
                            break;
                        
                        default:
                            break;
                    }
                    break;
                    
                case __SET_CALIB_K_CONFIRM:
                    UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                    Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_CHECK_SETTING, 0,
                                       __CHECK_STATE_SETTING, __CHECK_STATE_SETTING, __CHECK_STATE_SETTING,
                                       NULL, 0xF0);

                    Old_Value_i16 = 0x0000;
                    RS485_LogData_Calib(_RS485_SS_NH4_CALIB_686, &Old_Value_i16, sizeof(int16_t));
                    break;
                    
            }
            break;
   
        case _LCD_SCR_SET_OFFSET:
            switch(sLCD.sScreenNow.Para_u8)
            {
                case __SET_OFFSET_PH:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_OFFSET, (sLCD.sScreenNow.SubIndex_u8+1),
                                               __SET_OFFSET_PH, __SET_OFFSET_PH, __SET_OFFSET_TEMP,
                                               &sButton.Old_value, 0xF2);
                            sButton.Old_value = sParaDisplay.pH_Offset_i32;
                            break;
                            
                        case 1:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_CHECK_SETTING, 0,
                                               __CHECK_STATE_SETTING, __CHECK_STATE_SETTING, __CHECK_STATE_SETTING,
                                               NULL, 0xF0);
                            sParaDisplay.State_Setting = _STATE_SETTING_ENTER;
                            break;
                            
                        default:
                            break;
                    }
                    break;
                    
                case __SET_OFFSET_TEMP:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_OFFSET, (sLCD.sScreenNow.SubIndex_u8+1),
                                               __SET_OFFSET_TEMP, __SET_OFFSET_PH, __SET_OFFSET_TEMP,
                                               &sButton.Old_value,  0xF2);
                            sButton.Old_value = sParaDisplay.temp_Offset_i32;
                            break;
                            
                        case 1:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_CHECK_SETTING, 0,
                                               __CHECK_STATE_SETTING, __CHECK_STATE_SETTING, __CHECK_STATE_SETTING,
                                               NULL, 0xF0);
                            sParaDisplay.State_Setting = _STATE_SETTING_ENTER;
                            break;
                            
                        default:
                            break;
                    }
                    break;
                    
                default:
                    break;
            }
            break;
            
        case _LCD_SCR_SET_ALARM:
            switch(sLCD.sScreenNow.Para_u8)
            {
                case __SET_ALARM_STATE:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_ALARM, (sLCD.sScreenNow.SubIndex_u8+1),
                                               __SET_ALARM_STATE, __SET_ALARM_STATE, __SET_ALARM_LOWER,
                                               &sButton.Old_value, 0xF2);
                            sButton.Old_value = sTempAlarm.State;
                            sParaDisplay.aAlarm_State_u8 = (uint8_t *)&sButton.Old_value;
                            break;
                            
                        case 1:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_CHECK_SETTING, 0,
                                               __CHECK_STATE_SETTING, __CHECK_STATE_SETTING, __CHECK_STATE_SETTING,
                                               NULL, 0xF0);
                            sParaDisplay.State_Setting = _STATE_SETTING_ENTER;
                            break;
                            
                        default:
                            break;
                    }
                    break;
                    
                case __SET_ALARM_UPPER:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_ALARM, (sLCD.sScreenNow.SubIndex_u8+1),
                                               __SET_ALARM_UPPER, __SET_ALARM_STATE, __SET_ALARM_LOWER,
                                               &sButton.Old_value, 0xF2);
                            sButton.Old_value = sParaDisplay.Alarm_Upper_i32;
                            break;
                            
                        case 1:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_CHECK_SETTING, 0,
                                               __CHECK_STATE_SETTING, __CHECK_STATE_SETTING, __CHECK_STATE_SETTING,
                                               NULL, 0xF0);
                            sParaDisplay.State_Setting = _STATE_SETTING_ENTER;
                            break;
                            
                        default:
                            break;
                    }
                    break;
                    
                case __SET_ALARM_LOWER:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_ALARM, (sLCD.sScreenNow.SubIndex_u8+1),
                                               __SET_ALARM_LOWER, __SET_ALARM_STATE, __SET_ALARM_LOWER,
                                               &sButton.Old_value, 0xF2);
                            sButton.Old_value = sParaDisplay.Alarm_Lower_i32;
                            break;
                            
                        case 1:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_CHECK_SETTING, 0,
                                               __CHECK_STATE_SETTING, __CHECK_STATE_SETTING, __CHECK_STATE_SETTING,
                                               NULL, 0xF0);
                            sParaDisplay.State_Setting = _STATE_SETTING_ENTER;
                            break;
                            
                        default:
                            break;
                    }
                    break;

                default:
                    break;
            }
            break;
            
        case _LCD_SCR_SET_RANGE:
            switch(sLCD.sScreenNow.Para_u8)
            {
                case __SET_RANGE_U_KEY:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_RANGE, (sLCD.sScreenNow.SubIndex_u8+1),
                                               __SET_RANGE_U_KEY, __SET_RANGE_U_KEY, __SET_RANGE_L_TEMP,
                                               &sButton.Old_value, 0xF2);
                            sButton.Old_value = sParaDisplay.Upper_Key_i32;
                            break;
                            
                        case 1:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_CHECK_SETTING, 0,
                                               __CHECK_STATE_SETTING, __CHECK_STATE_SETTING, __CHECK_STATE_SETTING,
                                               NULL, 0xF0);
                            sParaDisplay.State_Setting = _STATE_SETTING_ENTER;
                            break;
                            
                        default:
                            break;
                    }
                    break;
                    
                case __SET_RANGE_L_KEY:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_RANGE, (sLCD.sScreenNow.SubIndex_u8+1),
                                               __SET_RANGE_L_KEY, __SET_RANGE_U_KEY, __SET_RANGE_L_TEMP,
                                               &sButton.Old_value, 0xF2);
                            sButton.Old_value = sParaDisplay.Lower_Key_i32;
                            break;
                            
                        case 1:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_CHECK_SETTING, 0,
                                               __CHECK_STATE_SETTING, __CHECK_STATE_SETTING, __CHECK_STATE_SETTING,
                                               NULL, 0xF0);
                            sParaDisplay.State_Setting = _STATE_SETTING_ENTER;
                            break;
                            
                        default:
                            break;
                    }
                    break;
                    
                case __SET_RANGE_U_TEMP:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_RANGE, (sLCD.sScreenNow.SubIndex_u8+1),
                                               __SET_RANGE_U_TEMP, __SET_RANGE_U_KEY, __SET_RANGE_L_TEMP,
                                               &sButton.Old_value, 0xF2);
                            sButton.Old_value = sParaDisplay.Upper_Temp_i32;
                            break;
                            
                        case 1:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_CHECK_SETTING, 0,
                                               __CHECK_STATE_SETTING, __CHECK_STATE_SETTING, __CHECK_STATE_SETTING,
                                               NULL, 0xF0);
                            sParaDisplay.State_Setting = _STATE_SETTING_ENTER;
                            break;
                            
                        default:
                            break;
                    }
                    break;
                    
                case __SET_RANGE_L_TEMP:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_RANGE, (sLCD.sScreenNow.SubIndex_u8+1),
                                               __SET_RANGE_L_TEMP, __SET_RANGE_U_KEY, __SET_RANGE_L_TEMP,
                                               &sButton.Old_value, 0xF2);
                            sButton.Old_value = sParaDisplay.Lower_Temp_i32;
                            break;
                            
                        case 1:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_CHECK_SETTING, 0,
                                               __CHECK_STATE_SETTING, __CHECK_STATE_SETTING, __CHECK_STATE_SETTING,
                                               NULL, 0xF0);
                            sParaDisplay.State_Setting = _STATE_SETTING_ENTER;
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
          switch(sLCD.sScreenBack.Index_u8)
          {
              case _LCD_SCR_SET_MODBUS:
                On_Speaker(50);
                sParaDisplay.State_Setting = _STATE_SETTING_DONE;
                switch (sLCD.sScreenBack.Para_u8)
                {
                    case __SET_MODBUS_ID:
                      Save_InforSlaveModbusRTU((uint8_t)sButton.Old_value, sSlave_ModbusRTU.Baudrate);
                      break;
                      
                    case __SET_MODBUS_BR:
                      for(uint8_t i = 0; i < 11; i++)
                      {
                        if(aBaudrate_value[i] == sButton.Old_value)
                        {
                            sSlave_ModbusRTU.Baudrate = i;
                            break;
                        }
                      }
                      Save_InforSlaveModbusRTU(sSlave_ModbusRTU.ID, sSlave_ModbusRTU.Baudrate);
                      break;
                      
                    default:
                      break;
                }
                break;
                
              case _LCD_SCR_SET_OFFSET:
                sParaDisplay.State_Setting = _STATE_SETTING_DONE;
                On_Speaker(50);
                switch (sLCD.sScreenBack.Para_u8)
                {
                    case __SET_OFFSET_PH:
                      Save_ParamCalib(((float)sButton.Old_value/Calculator_Scale(sParaDisplay.Scale_NH4)), sSensor_NH4.temp_Offset_f);
                      break;
                      
                    case __SET_OFFSET_TEMP:
                      Save_ParamCalib(sSensor_NH4.pH_Offset_f, ((float)sButton.Old_value/Calculator_Scale(sParaDisplay.Scale_Temp)));
                      break;
                      
                    default:
                      break;
                }
                break;
                
              case _LCD_SCR_SET_ALARM:
                sParaDisplay.State_Setting = _STATE_SETTING_DONE;
                On_Speaker(50);
                switch (sLCD.sScreenBack.Para_u8)
                {
                    case __SET_ALARM_STATE:
                      Save_TempAlarm((uint8_t)(sButton.Old_value), sTempAlarm.Alarm_Lower, sTempAlarm.Alarm_Upper);
                      break;
                      
                    case __SET_ALARM_UPPER:
                      Save_TempAlarm(sTempAlarm.State, sTempAlarm.Alarm_Lower, ((float)sButton.Old_value/Calculator_Scale(sParaDisplay.Scale_Alarm)));
                      break;
                      
                    case __SET_ALARM_LOWER:
                      Save_TempAlarm(sTempAlarm.State, ((float)sButton.Old_value/Calculator_Scale(sParaDisplay.Scale_Alarm)), sTempAlarm.Alarm_Upper);
                      break;
                      
                    default:
                      break;
                }
                break;
                
              case _LCD_SCR_SET_RANGE:
                sParaDisplay.State_Setting = _STATE_SETTING_DONE;
                On_Speaker(50);
                switch (sLCD.sScreenBack.Para_u8)
                {
                    case __SET_RANGE_U_KEY:
                      Save_MeasureRange(((float)sButton.Old_value/Calculator_Scale(sParaDisplay.Scale_Range)), sMeasureRange.Lower_Key, sMeasureRange.Upper_Temp, sMeasureRange.Lower_Temp);
                      break;
                      
                    case __SET_RANGE_L_KEY:
                      Save_MeasureRange(sMeasureRange.Upper_Key,((float)sButton.Old_value/Calculator_Scale(sParaDisplay.Scale_Range)) , sMeasureRange.Upper_Temp, sMeasureRange.Lower_Temp);
                      break;
                      
                    case __SET_RANGE_U_TEMP:
                      Save_MeasureRange(sMeasureRange.Upper_Key, sMeasureRange.Lower_Key,((float)sButton.Old_value/Calculator_Scale(sParaDisplay.Scale_Range)) , sMeasureRange.Lower_Temp);
                      break;
                      
                    case __SET_RANGE_L_TEMP:
                      Save_MeasureRange(sMeasureRange.Upper_Key, sMeasureRange.Lower_Key, sMeasureRange.Upper_Temp, ((float)sButton.Old_value/Calculator_Scale(sParaDisplay.Scale_Range)));
                      break;
                      
                    default:
                      break;
                }
                break;
                
              case _LCD_SCR_CALIB_NH4:
                switch (sLCD.sScreenBack.Para_u8)
                {
                    case __SET_CALIB_NH4_RESET:
                      RS485_Enter_Calib();
                      break;
                      
                    case __SET_CALIB_NH4_P1:
                      sParaDisplay.State_Setting = _STATE_SETTING_DONE;
                      On_Speaker(50);
                      Save_CalibNH4(_E_STD_NH4_P1, ((float)sButton.Old_value/Calculator_Scale(sParaDisplay.Scale_NH4))); 
                      Save_CalibNH4(_E_MEA_NH4_P1, sSensor_NH4.NH4_Value_f); 
                      break;
                      
                    case __SET_CALIB_NH4_P2:
                      sParaDisplay.State_Setting = _STATE_SETTING_DONE;
                      On_Speaker(50);
                      Save_CalibNH4(_E_STD_NH4_P2, ((float)sButton.Old_value/Calculator_Scale(sParaDisplay.Scale_NH4))); 
                      Save_CalibNH4(_E_MEA_NH4_P2, sSensor_NH4.NH4_Value_f); 
                      break;
                      
                    case __SET_CALIB_NH4_CONFIRM:
                      RS485_Enter_Calib();
                      break;
                      
                    default:
                      break;
                }
                break;
                
              case _LCD_SCR_CALIB_PH:
                switch (sLCD.sScreenBack.Para_u8)
                {
                    case __SET_CALIB_NH4_RESET:
                      RS485_Enter_Calib();
                      break;
                      
                    case __SET_CALIB_NH4_P1:
                      sParaDisplay.State_Setting = _STATE_SETTING_DONE;
                      On_Speaker(50);
                      Save_CalibNH4(_E_STD_NH4_P1, ((float)sButton.Old_value/Calculator_Scale(sParaDisplay.Scale_NH4))); 
                      Save_CalibNH4(_E_MEA_NH4_P1, sSensor_NH4.pH_Value_f); 
                      break;
                      
                    case __SET_CALIB_NH4_P2:
                      sParaDisplay.State_Setting = _STATE_SETTING_DONE;
                      On_Speaker(50);
                      Save_CalibNH4(_E_STD_NH4_P2, ((float)sButton.Old_value/Calculator_Scale(sParaDisplay.Scale_NH4))); 
                      Save_CalibNH4(_E_MEA_NH4_P2, sSensor_NH4.pH_Value_f); 
                      break;
                      
                    case __SET_CALIB_NH4_CONFIRM:
                      RS485_Enter_Calib();
                      break;
                      
                    default:
                      break;
                }
                break;
                
              case _LCD_SCR_CALIB_K:
                switch (sLCD.sScreenBack.Para_u8)
                {
                    case __SET_CALIB_K_RESET:
                      RS485_Enter_Calib();
                      break;
                      
                    case __SET_CALIB_K_P1:
                      sParaDisplay.State_Setting = _STATE_SETTING_DONE;
                      On_Speaker(50);
                      Save_CalibNH4(_E_STD_K_P1, ((float)sButton.Old_value/Calculator_Scale(sParaDisplay.Scale_NH4))); 
                      Save_CalibNH4(_E_MEA_K_P1, sSensor_NH4.K_Value_f); 
                      break;
                      
                    case __SET_CALIB_K_P2:
                      sParaDisplay.State_Setting = _STATE_SETTING_DONE;
                      On_Speaker(50);
                      Save_CalibNH4(_E_STD_K_P2, ((float)sButton.Old_value/Calculator_Scale(sParaDisplay.Scale_NH4))); 
                      Save_CalibNH4(_E_MEA_K_P2, sSensor_NH4.K_Value_f); 
                      break;
                      
                    case __SET_CALIB_K_CONFIRM:
                      RS485_Enter_Calib();
                      break;
                      
                    default:
                      break;
                }
                break;

              default:
                break;
          }
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
          break;
          
        case _LCD_SCR_PASS:
            aTEST[sButton.Old_value] ++;
            if (aTEST[sButton.Old_value] > '9')
                aTEST[sButton.Old_value] = '0';
            break;
            
        case _LCD_SCR_SETTING:
            if (sLCD.sScreenNow.Para_u8 > sLCD.sScreenNow.ParaMin_u8 ) {
                sLCD.sScreenNow.Para_u8--;
            }
            Display_Set_Screen_Flag(&sLCD.sScreenNow, NULL, 0xF1);
            break;
            
        case _LCD_SCR_SET_MODBUS:
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
                    case __SET_MODBUS_ID:
                        switch(sLCD.sScreenNow.SubIndex_u8)
                        {
                            case 0:
                                break;
                                
                            case 1:
                                if(sButton.Old_value < 256)
                                    sButton.Old_value++;
                                break;
                            
                            default:
                                break;
                        }
                        break;
                        
                    case __SET_MODBUS_BR:
                        switch(sLCD.sScreenNow.SubIndex_u8)
                        {
                            case 0:
                                break;
                                
                            case 1:
                                for(uint8_t i = 0; i < 11; i++)
                                {
                                    if(aBaudrate_value[i] == sButton.Old_value)
                                    {
                                        if(i<10)
                                        {
                                            sButton.Old_value = aBaudrate_value[i+1];
                                            break;
                                        }
                                    }
                                }
                                break;
                            
                            default:
                                break;
                        }
                        break;
                }
            }
            break;
            
        case _LCD_SCR_SET_MODE_CALIB:
            if (sLCD.sScreenNow.Para_u8 > sLCD.sScreenNow.ParaMin_u8 ) {
                sLCD.sScreenNow.Para_u8--;
            }
            Display_Set_Screen_Flag(&sLCD.sScreenNow, NULL, 0xF1);
            break;
            
        case _LCD_SCR_SET_OFFSET:
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
                case __SET_OFFSET_PH:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            break;
                            
                        case 1:
                            sButton.Old_value++;
                            break;
                        
                        default:
                            break;
                    }
                    break;
                    
                case __SET_OFFSET_TEMP:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            break;
                            
                        case 1:
                            sButton.Old_value++;
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
          
        case _LCD_SCR_CALIB_NH4:
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
                    case __SET_CALIB_NH4_RESET:
                        break;
                        
                    case __SET_CALIB_NH4_P1:
                        switch(sLCD.sScreenNow.SubIndex_u8)
                        {
                            case 0:
                                break;
                                
                            case 1:
                                sButton.Old_value += 1;
                                break;
                            
                            default:
                                break;
                        }
                        break;
                        
                    case __SET_CALIB_NH4_P2:
                        switch(sLCD.sScreenNow.SubIndex_u8)
                        {
                            case 0:
                                break;
                                
                            case 1:
                                sButton.Old_value += 1;
                                break;
                            
                            default:
                                break;
                        }
                        break;
                        
                    case __SET_CALIB_NH4_CONFIRM:
                        break;
                }
            }
            break;
            
        case _LCD_SCR_CALIB_PH:
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
                    case __SET_CALIB_NH4_RESET:
                        break;
                        
                    case __SET_CALIB_NH4_P1:
                        switch(sLCD.sScreenNow.SubIndex_u8)
                        {
                            case 0:
                                break;
                                
                            case 1:
                                sButton.Old_value += 1;
                                break;
                            
                            default:
                                break;
                        }
                        break;
                        
                    case __SET_CALIB_NH4_P2:
                        switch(sLCD.sScreenNow.SubIndex_u8)
                        {
                            case 0:
                                break;
                                
                            case 1:
                                sButton.Old_value += 1;
                                break;
                            
                            default:
                                break;
                        }
                        break;
                        
                    case __SET_CALIB_NH4_CONFIRM:
                        break;
                }
            }
            break;
            
        case _LCD_SCR_CALIB_K:
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
                    case __SET_CALIB_K_RESET:
                        break;
                        
                    case __SET_CALIB_K_P1:
                        switch(sLCD.sScreenNow.SubIndex_u8)
                        {
                            case 0:
                                break;
                                
                            case 1:
                                sButton.Old_value += 1;
                                break;
                            
                            default:
                                break;
                        }
                        break;
                        
                    case __SET_CALIB_K_P2:
                        switch(sLCD.sScreenNow.SubIndex_u8)
                        {
                            case 0:
                                break;
                                
                            case 1:
                                sButton.Old_value += 1;
                                break;
                            
                            default:
                                break;
                        }
                        break;
                        
                    case __SET_CALIB_K_CONFIRM:
                        break;
                }
            }
            break;
          
        case _LCD_SCR_SET_ALARM:
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
                case __SET_ALARM_STATE:
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
                    
                case __SET_ALARM_UPPER:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            break;
                            
                        case 1:
                            if(sButton.Old_value < ALARM_MAX*Calculator_Scale(sParaDisplay.Scale_Alarm))
                                sButton.Old_value++;
                            break;
                        
                        default:
                            break;
                    }
                    break;
                    
                case __SET_ALARM_LOWER:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            break;
                            
                        case 1:
                            if(sButton.Old_value < sParaDisplay.Alarm_Upper_i32)
                                sButton.Old_value++;
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
          
        case _LCD_SCR_SET_RANGE:
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
                case __SET_RANGE_U_KEY:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            break;
                            
                        case 1:
                            if(sButton.Old_value < RANGE_KEY_MAX*Calculator_Scale(sParaDisplay.Scale_Range))
                                sButton.Old_value++;
                            break;
                        
                        default:
                            break;
                    }
                    break;
                    
                case __SET_RANGE_L_KEY:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            break;
                            
                        case 1:
                            if(sButton.Old_value < sParaDisplay.Upper_Key_i32)
                                sButton.Old_value++;
                            break;
                        
                        default:
                            break;
                    }
                    break;
                    
                case __SET_RANGE_U_TEMP:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            break;
                            
                        case 1:
                            if(sButton.Old_value < RANGE_TEMP_MAX*Calculator_Scale(sParaDisplay.Scale_Range))
                                sButton.Old_value++;
                            break;
                        
                        default:
                            break;
                    }
                    break;
                    
                case __SET_RANGE_L_TEMP:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            break;
                            
                        case 1:
                            if(sButton.Old_value < sParaDisplay.Upper_Temp_i32)
                                sButton.Old_value++;
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
            
        default:
          break;
    }
}


void BUTTON_Down_Process (void)
{
    switch (sLCD.sScreenNow.Index_u8)
    {
        case _LCD_SCREEN_1:
          break;
          
        case _LCD_SCR_PASS:
            aTEST[sButton.Old_value] --;
            if (aTEST[sButton.Old_value] < '0') {
                aTEST[sButton.Old_value] = '9';
            }
            break;
            
        case _LCD_SCR_SETTING:
            if (sLCD.sScreenNow.Para_u8 < sLCD.sScreenNow.ParaMax_u8) {
                sLCD.sScreenNow.Para_u8++;
            }
            Display_Set_Screen_Flag(&sLCD.sScreenNow, NULL, 0xF1);
            break;
            
        case _LCD_SCR_SET_MODBUS:
              if(sLCD.sScreenNow.SubIndex_u8 == 0)
              {
                    if (sLCD.sScreenNow.Para_u8 < sLCD.sScreenNow.ParaMax_u8) {
                        sLCD.sScreenNow.Para_u8++;
                    }
                    Display_Set_Screen_Flag(&sLCD.sScreenNow, NULL, 0xF1);
              }
              else
              {
                switch (sLCD.sScreenNow.Para_u8)
                {
                    case __SET_MODBUS_ID:
                        switch(sLCD.sScreenNow.SubIndex_u8)
                        {
                            case 0:
                                break;
                                
                            case 1:
                                if(sButton.Old_value > 0)
                                    sButton.Old_value--;
                                break;
                            
                            default:
                                break;
                        }
                        break;
                        
                    case __SET_MODBUS_BR:
                        switch(sLCD.sScreenNow.SubIndex_u8)
                        {
                            case 0:
                                break;
                                
                            case 1:
                                for(uint8_t i = 10; i>0; i--)
                                {
                                    if(aBaudrate_value[i] == sButton.Old_value)
                                    {
                                        if(i>0)
                                        {
                                            sButton.Old_value = aBaudrate_value[i-1];
                                            break;
                                        }
                                    }
                                }
                                break;
                            
                            default:
                                break;
                        }
                        break;
                }
            }
            break;
            
        case _LCD_SCR_SET_MODE_CALIB:
            if (sLCD.sScreenNow.Para_u8 < sLCD.sScreenNow.ParaMax_u8) {
                sLCD.sScreenNow.Para_u8++;
            }
            Display_Set_Screen_Flag(&sLCD.sScreenNow, NULL, 0xF1);
            break;
            
        case _LCD_SCR_SET_OFFSET:
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
                case __SET_OFFSET_PH:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            break;
                            
                        case 1:
                            sButton.Old_value--;
                            break;
                        
                        default:
                            break;
                    }
                    break;
                    
                case __SET_OFFSET_TEMP:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            break;
                            
                        case 1:
                            sButton.Old_value--;
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
          
        case _LCD_SCR_CALIB_NH4:
              if(sLCD.sScreenNow.SubIndex_u8 == 0)
              {
                    if (sLCD.sScreenNow.Para_u8 < sLCD.sScreenNow.ParaMax_u8) {
                        sLCD.sScreenNow.Para_u8++;
                    }
                    Display_Set_Screen_Flag(&sLCD.sScreenNow, NULL, 0xF1);
              }
              else
              {
                switch (sLCD.sScreenNow.Para_u8)
                {
                    case __SET_CALIB_NH4_RESET:
                        break;
                        
                    case __SET_CALIB_NH4_P1:
                        switch(sLCD.sScreenNow.SubIndex_u8)
                        {
                            case 0:
                                break;
                                
                            case 1:
                                if(sButton.Old_value > 0)
                                    sButton.Old_value -= 1;
                                break;
                            
                            default:
                                break;
                        }
                        break;
                        
                    case __SET_CALIB_NH4_P2:
                        switch(sLCD.sScreenNow.SubIndex_u8)
                        {
                            case 0:
                                break;
                                
                            case 1:
                                if(sButton.Old_value > 0)
                                    sButton.Old_value -= 1;
                                break;
                            
                            default:
                                break;
                        }
                        break;
                        
                    case __SET_CALIB_NH4_CONFIRM:
                        break;
                }
            }
            break;
            
        case _LCD_SCR_CALIB_PH:
              if(sLCD.sScreenNow.SubIndex_u8 == 0)
              {
                    if (sLCD.sScreenNow.Para_u8 < sLCD.sScreenNow.ParaMax_u8) {
                        sLCD.sScreenNow.Para_u8++;
                    }
                    Display_Set_Screen_Flag(&sLCD.sScreenNow, NULL, 0xF1);
              }
              else
              {
                switch (sLCD.sScreenNow.Para_u8)
                {
                    case __SET_CALIB_NH4_RESET:
                        break;
                        
                    case __SET_CALIB_NH4_P1:
                        switch(sLCD.sScreenNow.SubIndex_u8)
                        {
                            case 0:
                                break;
                                
                            case 1:
                                if(sButton.Old_value > 0)
                                    sButton.Old_value -= 1;
                                break;
                            
                            default:
                                break;
                        }
                        break;
                        
                    case __SET_CALIB_NH4_P2:
                        switch(sLCD.sScreenNow.SubIndex_u8)
                        {
                            case 0:
                                break;
                                
                            case 1:
                                if(sButton.Old_value > 0)
                                    sButton.Old_value -= 1;
                                break;
                            
                            default:
                                break;
                        }
                        break;
                        
                    case __SET_CALIB_NH4_CONFIRM:
                        break;
                }
            }
            break;

        case _LCD_SCR_CALIB_K:
              if(sLCD.sScreenNow.SubIndex_u8 == 0)
              {
                    if (sLCD.sScreenNow.Para_u8 < sLCD.sScreenNow.ParaMax_u8) {
                        sLCD.sScreenNow.Para_u8++;
                    }
                    Display_Set_Screen_Flag(&sLCD.sScreenNow, NULL, 0xF1);
              }
              else
              {
                switch (sLCD.sScreenNow.Para_u8)
                {
                    case __SET_CALIB_K_RESET:
                        break;
                        
                    case __SET_CALIB_K_P1:
                        switch(sLCD.sScreenNow.SubIndex_u8)
                        {
                            case 0:
                                break;
                                
                            case 1:
                                if(sButton.Old_value > 0)
                                    sButton.Old_value -= 1;
                                break;
                            
                            default:
                                break;
                        }
                        break;
                        
                    case __SET_CALIB_K_P2:
                        switch(sLCD.sScreenNow.SubIndex_u8)
                        {
                            case 0:
                                break;
                                
                            case 1:
                                if(sButton.Old_value > 0)
                                    sButton.Old_value -= 1;
                                break;
                            
                            default:
                                break;
                        }
                        break;
                        
                    case __SET_CALIB_K_CONFIRM:
                        break;
                }
            }
            break;
          
        case _LCD_SCR_SET_ALARM:
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
                case __SET_ALARM_STATE:
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
                    
                case __SET_ALARM_UPPER:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            break;
                            
                        case 1:
                            if(sButton.Old_value > sParaDisplay.Alarm_Lower_i32)
                                sButton.Old_value--;
                            break;
                        
                        default:
                            break;
                    }
                    break;
                    
                case __SET_ALARM_LOWER:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            break;
                            
                        case 1:
                            if(sButton.Old_value > ALARM_MIN*Calculator_Scale(sParaDisplay.Scale_Alarm))
                                sButton.Old_value--;
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
          
        case _LCD_SCR_SET_RANGE:
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
                case __SET_RANGE_U_KEY:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            break;
                            
                        case 1:
                            if(sButton.Old_value > sParaDisplay.Lower_Key_i32)
                                sButton.Old_value--;
                            break;
                        
                        default:
                            break;
                    }
                    break;
                    
                case __SET_RANGE_L_KEY:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            break;
                            
                        case 1:
                            if(sButton.Old_value > RANGE_KEY_MIN*Calculator_Scale(sParaDisplay.Scale_Range))
                                sButton.Old_value--;
                            break;
                        
                        default:
                            break;
                    }
                    break;
                    
                case __SET_RANGE_U_TEMP:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            break;
                            
                        case 1:
                            if(sButton.Old_value > sParaDisplay.Lower_Temp_i32)
                                sButton.Old_value--;
                            break;
                        
                        default:
                            break;
                    }
                    break;
                    
                case __SET_RANGE_L_TEMP:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            break;
                            
                        case 1:
                            if(sButton.Old_value > RANGE_TEMP_MIN*Calculator_Scale(sParaDisplay.Scale_Range))
                                sButton.Old_value--;
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
            
        default:
          break;
    }
}

void BUTTON_ESC_Process (void)
{
    switch (sLCD.sScreenNow.Index_u8)
    {
        case _LCD_SCREEN_1:
          break;
          
        case _LCD_SCR_PASS:
            sLCD.sScreenNow.Index_u8 = _LCD_SCREEN_1;
            break;
            
        case _LCD_SCR_SETTING:
            sLCD.sScreenNow.Index_u8 = _LCD_SCREEN_1;
            break;
          
        case _LCD_SCR_SET_MODBUS:   
          switch(sLCD.sScreenNow.Para_u8)
          {
                case __SET_MODBUS_ID:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SETTING, 0,
                                                __SCR_SET_MODBUS, __SCR_SET_MODBUS, __SCR_SET_INFOR,
                                                NULL, 0xF1);
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            break;             
                          
                        case 1:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_MODBUS, (sLCD.sScreenNow.SubIndex_u8-1),
                                               __SET_MODBUS_ID, __SET_MODBUS_ID, __SET_MODBUS_BR,
                                               &sParaDisplay.ID_u8, 0xF1);
                            break;
                            
                        default:
                            break;
                    }
                    break;
                
                case __SET_MODBUS_BR:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SETTING, 0,
                                                __SCR_SET_MODBUS, __SCR_SET_MODBUS, __SCR_SET_INFOR,
                                                NULL, 0xF1);
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            break;         
                          
                        case 1:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_MODBUS, (sLCD.sScreenNow.SubIndex_u8-1),
                                               __SET_MODBUS_BR, __SET_MODBUS_ID, __SET_MODBUS_BR,
                                               &sParaDisplay.Baudrate_u32, 0xF1);
                            break;
                            
                        default:
                            break;
                    }
                    break;
                
              default:
                break;
          }
          break;
          
        case _LCD_SCR_SET_MODE_CALIB: 
          switch(sLCD.sScreenNow.Para_u8)
          {
            case __SET_MODE_CALIB_NH4:
            case __SET_MODE_CALIB_PH:
            case __SET_MODE_CALIB_K:
                Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SETTING, 0,
                                    __SCR_SET_CALIB, __SCR_SET_MODBUS, __SCR_SET_INFOR,
                                    NULL, 0xF1);
                UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                break;   
            
            default:
                break;
          }
          break;
          
        case _LCD_SCR_CALIB_NH4:   
          switch(sLCD.sScreenNow.Para_u8)
          {
              case __SET_CALIB_NH4_RESET:
                Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_MODE_CALIB, 0,
                                    __SET_MODE_CALIB_NH4, __SET_MODE_CALIB_NH4, __SET_MODE_CALIB_K,
                                    NULL, 0xF1);
                UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                break;
                
            case __SET_CALIB_NH4_P1:
                switch(sLCD.sScreenNow.SubIndex_u8)
                {
                    case 0:
                        Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_MODE_CALIB, 0,
                                            __SET_MODE_CALIB_NH4, __SET_MODE_CALIB_NH4, __SET_MODE_CALIB_K,
                                            NULL, 0xF1);
                        UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                        break;             
                      
                    case 1:
                        UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                        Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_CALIB_NH4, (sLCD.sScreenNow.SubIndex_u8-1),
                                           __SET_CALIB_NH4_P1, __SET_CALIB_NH4_RESET, __SET_CALIB_NH4_CONFIRM,
                                           &sParaDisplay.NH4_P1_i32, 0xF1);
                        break;
                        
                    default:
                        break;
                }
                break;
                
            case __SET_CALIB_NH4_P2:
                switch(sLCD.sScreenNow.SubIndex_u8)
                {
                    case 0:
                        Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_MODE_CALIB, 0,
                                            __SET_MODE_CALIB_NH4, __SET_MODE_CALIB_NH4, __SET_MODE_CALIB_K,
                                            NULL, 0xF1);
                        UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                        break;             
                      
                    case 1:
                        UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                        Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_CALIB_NH4, (sLCD.sScreenNow.SubIndex_u8-1),
                                           __SET_CALIB_NH4_P2, __SET_CALIB_NH4_RESET, __SET_CALIB_NH4_CONFIRM,
                                           &sParaDisplay.NH4_P2_i32, 0xF1);
                        break;
                        
                    default:
                        break;
                }
                break;
                
              case __SET_CALIB_NH4_CONFIRM:
                Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_MODE_CALIB, 0,
                                    __SET_MODE_CALIB_NH4, __SET_MODE_CALIB_NH4, __SET_MODE_CALIB_K,
                                    NULL, 0xF1);
                UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                break;
                
              default:
                break;
          }
          break;
          
        case _LCD_SCR_CALIB_PH:   
          switch(sLCD.sScreenNow.Para_u8)
          {
              case __SET_CALIB_NH4_RESET:
                Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_MODE_CALIB, 0,
                                    __SET_MODE_CALIB_PH, __SET_MODE_CALIB_NH4, __SET_MODE_CALIB_K,
                                    NULL, 0xF1);
                UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                break;
                
            case __SET_CALIB_NH4_P1:
                switch(sLCD.sScreenNow.SubIndex_u8)
                {
                    case 0:
                        Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_MODE_CALIB, 0,
                                            __SET_MODE_CALIB_PH, __SET_MODE_CALIB_NH4, __SET_MODE_CALIB_K,
                                            NULL, 0xF1);
                        UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                        break;             
                      
                    case 1:
                        UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                        Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_CALIB_PH, (sLCD.sScreenNow.SubIndex_u8-1),
                                           __SET_CALIB_NH4_P1, __SET_CALIB_NH4_RESET, __SET_CALIB_NH4_CONFIRM,
                                           &sParaDisplay.PH_P1_i32, 0xF1);
                        break;
                        
                    default:
                        break;
                }
                break;
                
            case __SET_CALIB_NH4_P2:
                switch(sLCD.sScreenNow.SubIndex_u8)
                {
                    case 0:
                        Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_MODE_CALIB, 0,
                                            __SET_MODE_CALIB_PH, __SET_MODE_CALIB_NH4, __SET_MODE_CALIB_K,
                                            NULL, 0xF1);
                        UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                        break;             
                      
                    case 1:
                        UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                        Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_CALIB_PH, (sLCD.sScreenNow.SubIndex_u8-1),
                                           __SET_CALIB_NH4_P2, __SET_CALIB_NH4_RESET, __SET_CALIB_NH4_CONFIRM,
                                           &sParaDisplay.PH_P2_i32, 0xF1);
                        break;
                        
                    default:
                        break;
                }
                break;
                
              case __SET_CALIB_NH4_CONFIRM:
                Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_MODE_CALIB, 0,
                                    __SET_MODE_CALIB_PH, __SET_MODE_CALIB_NH4, __SET_MODE_CALIB_K,
                                    NULL, 0xF1);
                UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                break;
                
              default:
                break;
          }
          break;
          
        case _LCD_SCR_CALIB_K:   
          switch(sLCD.sScreenNow.Para_u8)
          {
              case __SET_CALIB_K_RESET:
                Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_MODE_CALIB, 0,
                                    __SET_MODE_CALIB_K, __SET_MODE_CALIB_NH4, __SET_MODE_CALIB_K,
                                    NULL, 0xF1);
                UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                break;
                
            case __SET_CALIB_K_P1:
                switch(sLCD.sScreenNow.SubIndex_u8)
                {
                    case 0:
                        Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_MODE_CALIB, 0,
                                            __SET_MODE_CALIB_K, __SET_MODE_CALIB_NH4, __SET_MODE_CALIB_K,
                                            NULL, 0xF1);
                        UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                        break;             
                      
                    case 1:
                        UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                        Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_CALIB_K, (sLCD.sScreenNow.SubIndex_u8-1),
                                           __SET_CALIB_K_P1, __SET_CALIB_K_RESET, __SET_CALIB_K_CONFIRM,
                                           &sParaDisplay.K_P1_i32, 0xF1);
                        break;
                        
                    default:
                        break;
                }
                break;
                
            case __SET_CALIB_K_P2:
                switch(sLCD.sScreenNow.SubIndex_u8)
                {
                    case 0:
                        Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_MODE_CALIB, 0,
                                            __SET_MODE_CALIB_K, __SET_MODE_CALIB_NH4, __SET_MODE_CALIB_K,
                                            NULL, 0xF1);
                        UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                        break;             
                      
                    case 1:
                        UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                        Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_CALIB_K, (sLCD.sScreenNow.SubIndex_u8-1),
                                           __SET_CALIB_K_P2, __SET_CALIB_K_RESET, __SET_CALIB_K_CONFIRM,
                                           &sParaDisplay.K_P2_i32, 0xF1);
                        break;
                        
                    default:
                        break;
                }
                break;
                
              case __SET_CALIB_K_CONFIRM:
                Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_MODE_CALIB, 0,
                                    __SET_MODE_CALIB_K, __SET_MODE_CALIB_NH4, __SET_MODE_CALIB_K,
                                    NULL, 0xF1);
                UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                break;
                
              default:
                break;
          }
          break;
          
        case _LCD_SCR_SET_OFFSET:
          switch(sLCD.sScreenNow.Para_u8)
          {
                case __SET_OFFSET_PH:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SETTING, 0,
                                                __SCR_SET_OFFSET, __SCR_SET_MODBUS, __SCR_SET_INFOR,
                                                NULL, 0xF1);
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            break;             
                          
                        case 1:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_OFFSET, (sLCD.sScreenNow.SubIndex_u8-1),
                                               __SET_OFFSET_PH, __SET_OFFSET_PH, __SET_OFFSET_TEMP,
                                               &sParaDisplay.pH_Offset_i32, 0xF1);
                            break;
                            
                        default:
                            break;
                    }
                    break;
                
                case __SET_OFFSET_TEMP:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SETTING, 0,
                                                __SCR_SET_OFFSET, __SCR_SET_MODBUS, __SCR_SET_INFOR,
                                                NULL, 0xF1);
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            break;         
                          
                        case 1:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_OFFSET, (sLCD.sScreenNow.SubIndex_u8-1),
                                               __SET_OFFSET_TEMP, __SET_OFFSET_PH, __SET_OFFSET_TEMP,
                                               &sParaDisplay.temp_Offset_i32, 0xF1);
                            break;
                            
                        default:
                            break;
                    }
                    break;
                
              default:
                break;
          }
          break;
          
        case _LCD_SCR_SET_ALARM:
          switch(sLCD.sScreenNow.Para_u8)
          {
                case __SET_ALARM_STATE:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SETTING, 0,
                                                __SCR_SET_ALARM, __SCR_SET_MODBUS, __SCR_SET_INFOR,
                                                NULL, 0xF1);
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            break;             
                          
                        case 1:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_ALARM, (sLCD.sScreenNow.SubIndex_u8-1),
                                               __SET_ALARM_STATE, __SET_ALARM_STATE, __SET_ALARM_LOWER,
                                               &sTempAlarm.State, 0xF1);
                            sParaDisplay.aAlarm_State_u8 = &sTempAlarm.State;
                            break;
                            
                        default:
                            break;
                    }
                    break;
                    
                case __SET_ALARM_UPPER:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SETTING, 0,
                                                __SCR_SET_ALARM, __SCR_SET_MODBUS, __SCR_SET_INFOR,
                                                NULL, 0xF1);
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            break;             
                          
                        case 1:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_ALARM, (sLCD.sScreenNow.SubIndex_u8-1),
                                               __SET_ALARM_UPPER, __SET_ALARM_STATE, __SET_ALARM_LOWER,
                                               &sParaDisplay.Alarm_Upper_i32, 0xF1);
                            break;
                            
                        default:
                            break;
                    }
                    break;
                    
                case __SET_ALARM_LOWER:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SETTING, 0,
                                                __SCR_SET_ALARM, __SCR_SET_MODBUS, __SCR_SET_INFOR,
                                                NULL, 0xF1);
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            break;             
                          
                        case 1:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_ALARM, (sLCD.sScreenNow.SubIndex_u8-1),
                                               __SET_ALARM_LOWER, __SET_ALARM_STATE, __SET_ALARM_LOWER,
                                               &sParaDisplay.Alarm_Lower_i32, 0xF1);
                            break;
                            
                        default:
                            break;
                    }
                    break;

              default:
                break;
          }
          break;
          
        case _LCD_SCR_SET_RANGE:
          switch(sLCD.sScreenNow.Para_u8)
          {
                case __SET_RANGE_U_KEY:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SETTING, 0,
                                                __SCR_SET_RANGE, __SCR_SET_MODBUS, __SCR_SET_INFOR,
                                                NULL, 0xF1);
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            break;             
                          
                        case 1:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_RANGE, (sLCD.sScreenNow.SubIndex_u8-1),
                                               __SET_RANGE_U_KEY, __SET_RANGE_U_KEY, __SET_RANGE_L_TEMP,
                                               &sParaDisplay.Upper_Key_i32, 0xF1);
                            break;
                            
                        default:
                            break;
                    }
                    break;
                    
                case __SET_RANGE_L_KEY:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SETTING, 0,
                                                __SCR_SET_RANGE, __SCR_SET_MODBUS, __SCR_SET_INFOR,
                                                NULL, 0xF1);
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            break;             
                          
                        case 1:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_RANGE, (sLCD.sScreenNow.SubIndex_u8-1),
                                               __SET_RANGE_L_KEY, __SET_RANGE_U_KEY, __SET_RANGE_L_TEMP,
                                               &sParaDisplay.Lower_Key_i32, 0xF1);
                            break;
                            
                        default:
                            break;
                    }
                    break;
                    
                case __SET_RANGE_U_TEMP:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SETTING, 0,
                                                __SCR_SET_RANGE, __SCR_SET_MODBUS, __SCR_SET_INFOR,
                                                NULL, 0xF1);
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            break;             
                          
                        case 1:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_RANGE, (sLCD.sScreenNow.SubIndex_u8-1),
                                               __SET_RANGE_U_TEMP, __SET_RANGE_U_KEY, __SET_RANGE_L_TEMP,
                                               &sParaDisplay.Upper_Temp_i32, 0xF1);
                            break;
                            
                        default:
                            break;
                    }
                    break;
                    
                case __SET_RANGE_L_TEMP:
                    switch(sLCD.sScreenNow.SubIndex_u8)
                    {
                        case 0:
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SETTING, 0,
                                                __SCR_SET_RANGE, __SCR_SET_MODBUS, __SCR_SET_INFOR,
                                                NULL, 0xF1);
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            break;             
                          
                        case 1:
                            UTIL_MEM_cpy(&sLCD.sScreenBack, &sLCD.sScreenNow, sizeof(sScreenInformation));
                            Display_Set_Screen(&sLCD.sScreenNow, _LCD_SCR_SET_RANGE, (sLCD.sScreenNow.SubIndex_u8-1),
                                               __SET_RANGE_L_TEMP, __SET_RANGE_U_KEY, __SET_RANGE_L_TEMP,
                                               &sParaDisplay.Lower_Temp_i32, 0xF1);
                            break;
                            
                        default:
                            break;
                    }
                    break;

              default:
                break;
          }
          break;
          
        case _LCD_SCR_SET_INFORMATION:
            UTIL_MEM_cpy(&sLCD.sScreenNow, &sLCD.sScreenBack, sizeof(sScreenInformation));
            break;
          
        case _LCD_SCR_CHECK_SETTING:
          if(sParaDisplay.State_Setting != _STATE_SETTING_WAIT)
          {
            UTIL_MEM_cpy(&sLCD.sScreenNow, &sLCD.sScreenBack, sizeof(sScreenInformation));
            sParaDisplay.State_Setting = _STATE_SETTING_FREE;
          }
          break;
          
        default:
          break;
    }
}



