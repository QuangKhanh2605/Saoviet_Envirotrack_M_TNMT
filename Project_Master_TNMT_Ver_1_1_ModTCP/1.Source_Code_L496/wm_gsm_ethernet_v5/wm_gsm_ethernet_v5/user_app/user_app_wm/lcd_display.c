

#include "lcd_display.h"
#include "string.h"



static uint8_t _Cb_Display_Init (uint8_t event);
static uint8_t _Cb_Display_Show (uint8_t event);
static uint8_t _Cb_button_scan (uint8_t event);
static uint8_t _Cb_button_detect (uint8_t event);
static uint8_t _Cb_Display_Auto_SW (uint8_t event);
static uint8_t _Cb_Display_Logo (uint8_t event);
extern char sFirmVersion[];


sEvent_struct sEventDisplay [] =
{
    { _EVENT_DISP_INIT, 		    0, 0, 500, 	    _Cb_Display_Init }, 
    { _EVENT_DISP_LOGO, 		    0, 0, 2000, 	_Cb_Display_Logo}, 
    { _EVENT_DISP_SHOW, 		    0, 0, 100,      _Cb_Display_Show }, 
    { _EVENT_DISP_AUTO_SW, 		    0, 0, 5000,     _Cb_Display_Auto_SW }, 
    
    { _EVENT_BUTTON_SCAN, 		    0, 0, 2,    	_Cb_button_scan  },
    { _EVENT_BUTTON_DETECTTED, 	    0, 0, 10, 		_Cb_button_detect },
};
         

sLCDinformation      sLCD;

uint8_t aPASSWORD[4] = {"1000"};

Struct_PageSensor       sLCDPageSensor = {0};

sOjectInformation   sLCDObject[] = 
{
        //  para          name                  value      dtype         scale   unit      row  col      screen
    {   __PARAM_COMM,       "Pcomm.",           NULL,   _DTYPE_STRING,   0,      NULL,      0,  0,  0x00,    _LCD_SCREEN_1    },
    {   __SERIAL_1,         NULL,               NULL,   _DTYPE_STRING,   0,      NULL,      2,  18, 0x00,    _LCD_SCREEN_1    },
    {   __BATTERY_VOL,      "V_bat: ",          NULL,   _DTYPE_U32,      0xFD,   " (V)",    3,  0,  0x00,    _LCD_SCREEN_1    },
    {   __POWWER_12V,       "V_pow: ",          NULL,   _DTYPE_U32,      0xFD,   " (V)",    4,  0,  0x00,    _LCD_SCREEN_1    },
    {   __FREQ,             "Tsend: ",          NULL,   _DTYPE_U16,      0,      " (min)",  5,  0,  0x00,    _LCD_SCREEN_1    },
    {   __RSSI,             "Rssi : -",         NULL,   _DTYPE_U8,       0,      " (dbm)",  6,  0,  0x00,    _LCD_SCREEN_1    },
    {   __SD_FREE,          "SD_FR: ",          NULL,   _DTYPE_U32,      0,      " (MB)",   7,  0,  0x00,    _LCD_SCREEN_1    },
    
    {   __PARAM_CM44,       "Sensor.",            NULL,   _DTYPE_STRING,   0,      NULL,      0,  0,  0x00,    _LCD_SCREEN_CM44    },
//    {   __SC1_ID_DCU,       NULL,               NULL,   _DTYPE_STRING,   0,      NULL,      2,  20, 0x00,    _LCD_SCREEN_CM44    },
    {   __SC1_CLO_DU,       "CLO  : ",         NULL,   _DTYPE_I32,        0,   "  mg/L",   2,  0,  0x00,    _LCD_SCREEN_CM44    },
    {   __SC1_PH_WATER,     "PH   : ",         NULL,   _DTYPE_I32,        0,   "  pH",     2,  0,  0x00,    _LCD_SCREEN_CM44    },
    {   __SC1_TURB,         "NTU  : ",         NULL,   _DTYPE_I32,        0,   "  NTU",    2,  0,  0x00,    _LCD_SCREEN_CM44    },
    {   __SC1_EC,           "EC   : ",         NULL,   _DTYPE_I32,        0,   "  uS/cm",  2,  0,  0x00,    _LCD_SCREEN_CM44    },
    {   __SC1_SALINITY,     "Salt : ",         NULL,   _DTYPE_I32,        0,   "  %",      2,  0,  0x00,    _LCD_SCREEN_CM44    },
    {   __SC1_COD,          "COD  : ",         NULL,   _DTYPE_I32,        0,   "  mg/L",   2,  0,  0x00,    _LCD_SCREEN_CM44    },
    {   __SC1_TSS,          "TSS  : ",         NULL,   _DTYPE_I32,        0,   "  mg/L",   2,  0,  0x00,    _LCD_SCREEN_CM44    },
    {   __SC1_NH4,          "NH4  : ",         NULL,   _DTYPE_I32,        0,   "  mg/L",   2,  0,  0x00,    _LCD_SCREEN_CM44    },
    {   __SC1_DO,           "DO   : ",         NULL,   _DTYPE_I32,        0,   "  mg/L",   2,  0,  0x00,    _LCD_SCREEN_CM44    },
    {   __SC1_TDS,          "TDS  : ",         NULL,   _DTYPE_I32,        0,   "  mg/L",   2,  0,  0x00,    _LCD_SCREEN_CM44    },
    {   __SC1_NO3,          "NO3  : ",         NULL,   _DTYPE_I32,        0,   "  mg/L",   2,  0,  0x00,    _LCD_SCREEN_CM44    },
    {   __SC1_TEMP,         "Temp : ",         NULL,   _DTYPE_I32,        0,   "  �C",     2,  0,  0x00,    _LCD_SCREEN_CM44    },
    
    {   __PARAM_CM44_2,     "Sensor.",         NULL,   _DTYPE_STRING,   0,      NULL,      0,  0,  0x00,    _LCD_SCREEN_CM44_2  },
    {   __PARAM_CM44_3,     "Sensor.",         NULL,   _DTYPE_STRING,   0,      NULL,      0,  0,  0x00,    _LCD_SCREEN_CM44_3  },
    
    //screen channel 1

    {   __CHANEL_1,         "CH.1  ",           NULL,   _DTYPE_STRING,   0,      NULL,      0,  0,  0x00,    _LCD_SCREEN_2    },
    {   __PULSE_1,          "To: ",             NULL,   _DTYPE_DOUBLE,   0,      " (m3)",   2,  0,  0x00,    _LCD_SCREEN_2    },
    {   __PRESS_1,          "P : ",             NULL,   _DTYPE_I32,      0xFD,   " (bar)",  3,  0,  0x00,    _LCD_SCREEN_2    },
    {   __FLOW_1,           "F : ",             NULL,   _DTYPE_FLOAT,    0xFE,   " (m3/h)", 4,  0,  0x00,    _LCD_SCREEN_2    },
    
    {   __CHANEL_2,         "CH.2  ",           NULL,   _DTYPE_STRING,   0,      NULL,      0,  0,  0xF0,    _LCD_SCREEN_3    },
    {   __PULSE_2,          "To: ",             NULL,   _DTYPE_DOUBLE,   0,      " (m3)",   2,  0,  0x00,    _LCD_SCREEN_3    },
    {   __PRESS_2,          "P : ",             NULL,   _DTYPE_I32,      0xFD,   " (bar)",  3,  0,  0x00,    _LCD_SCREEN_3    },
    {   __FLOW_2,           "F : ",             NULL,   _DTYPE_FLOAT,    0xFE,   " (m3/h)", 4,  0,  0x00,    _LCD_SCREEN_3    },
    
    {   __CHANEL_3,         "CH.3  ",           NULL,   _DTYPE_STRING,   0,      NULL,      0,  0,  0x00,    _LCD_SCREEN_4    },
    {   __PULSE_3,          "To: ",             NULL,   _DTYPE_DOUBLE,   0,      " (m3)",   2,  0,  0x00,    _LCD_SCREEN_4    },
    {   __PRESS_3,          "P : ",             NULL,   _DTYPE_I32,      0xFD,   " (bar)",  3,  0,  0x00,    _LCD_SCREEN_4    },
    {   __FLOW_3,           "F : ",             NULL,   _DTYPE_FLOAT,    0xFE,   " (m3/h)", 4,  0,  0x00,    _LCD_SCREEN_4    },
    
    {   __CHANEL_4,         "CH.4  ",           NULL,   _DTYPE_STRING,   0,      NULL,      0,  0,  0x00,    _LCD_SCREEN_5    },
    {   __PULSE_4,          "To: ",             NULL,   _DTYPE_DOUBLE,   0,      " (m3)",   2,  0,  0x00,    _LCD_SCREEN_5    },
    {   __PRESS_4,          "P : ",             NULL,   _DTYPE_I32,      0xFD,   " (bar)",  3,  0,  0x00,    _LCD_SCREEN_5    },
    {   __FLOW_4,           "F : ",             NULL,   _DTYPE_FLOAT,    0xFE,   " (m3/h)", 4,  0,  0x00,    _LCD_SCREEN_5    },
    
    {   __CHANEL_5,         "CH.5  ",           NULL,   _DTYPE_STRING,   0,      NULL,      0,  0,  0x00,    _LCD_SCREEN_6    },
    {   __PULSE_5,          "To: ",             NULL,   _DTYPE_DOUBLE,   0,      " (m3)",   2,  0,  0x00,    _LCD_SCREEN_6    },
    {   __PRESS_5,          "P : ",             NULL,   _DTYPE_I32,      0xFD,   " (bar)",  3,  0,  0x00,    _LCD_SCREEN_6    },
    {   __FLOW_5,           "F : ",             NULL,   _DTYPE_FLOAT,    0xFE,   " (m3/h)", 4,  0,  0x00,    _LCD_SCREEN_6    },
    
    {   __CHANEL_6,         "CH.6  ",           NULL,   _DTYPE_STRING,   0,      NULL,      0,  0,  0x00,    _LCD_SCREEN_7    },
    {   __PULSE_6,          "To: ",             NULL,   _DTYPE_DOUBLE,   0,      " (m3)",   2,  0,  0x00,    _LCD_SCREEN_7    },
    {   __PRESS_6,          "P : ",             NULL,   _DTYPE_I32,      0xFD,   " (bar)",  3,  0,  0x00,    _LCD_SCREEN_7    },
    {   __FLOW_6,           "F : ",             NULL,   _DTYPE_FLOAT,    0xFE,   " (m3/h)", 4,  0,  0x00,    _LCD_SCREEN_7    },
    
    {   __CHANEL_7,         "CH.7  ",           NULL,   _DTYPE_STRING,   0,      NULL,      0,  0,  0x00,    _LCD_SCREEN_7b    },
    {   __PULSE_7,          "To : ",            NULL,   _DTYPE_I64,      0,      " (m3)",   1,  0,  0x00,    _LCD_SCREEN_7b    },
    {   __PULSE_7_2,        "For: ",            NULL,   _DTYPE_U64,      0,      " (m3)",   2,  0,  0x00,    _LCD_SCREEN_7b    },
    {   __PULSE_7_3,        "Rev: ",            NULL,   _DTYPE_U64,      0,      " (m3)",   3,  0,  0x00,    _LCD_SCREEN_7b    },
    {   __FLOW_7,           "F : ",             NULL,   _DTYPE_I32,      0xFE,   " (m3/h)", 4,  0,  0x00,    _LCD_SCREEN_7b    },
    {   __PRESS_7,          "P : ",             NULL,   _DTYPE_I32,      0xFD,   " (bar)",  5,  0,  0x00,    _LCD_SCREEN_7b    },
    
    {   __INFORMATION,      "Infor.",           NULL,   _DTYPE_STRING,   0,      NULL,      0,  0,  0x00,    _LCD_SCREEN_8    },
    {   __FW_VERSION_1,     "Ver:  ",           NULL,   _DTYPE_STRING,   0,      NULL,      2,  0,  0x00,    _LCD_SCREEN_8    },
    
    {   __PASS_WORD_1,      "Enter Password",   NULL,   _DTYPE_STRING,   0,      NULL,      2,  24, 0x00,    _LCD_SCR_PASS    },
    {   __PASS_WORD_2,      NULL,               NULL,   _DTYPE_STRING,   0,      NULL,      3,  48, 0x00,    _LCD_SCR_PASS    },
    
    {   __SET_REQ_1,        "1.Tsend Data",     NULL,   _DTYPE_STRING,   0,      NULL,      2,  24, 0x00,    _LCD_SCR_SETTING },
    {   __SET_PULSE_SETT,   "2.Pulse Setting",  NULL,   _DTYPE_STRING,   0,      NULL,      3,  24, 0x00,    _LCD_SCR_SETTING },
    {   __SET_PULSE_RESET,  "3.Pulse Reset",    NULL,   _DTYPE_STRING,   0,      NULL,      4,  24, 0x00,    _LCD_SCR_SETTING },
    {   __SET_PRESSURE,     "4.Pressure Set",   NULL,   _DTYPE_STRING,   0,      NULL,      5,  24, 0x00,    _LCD_SCR_SETTING },
    {   __SET_PRESS_CALIB,  "5.Pressure Calib", NULL,   _DTYPE_STRING,   0,      NULL,      6,  24, 0x00,    _LCD_SCR_SETTING },
    
    {   __SET_LEVEL_SETT,   "6.Level Set",      NULL,   _DTYPE_STRING,   0,      NULL,      2,  24, 0x00,    _LCD_SCR_SETTING_2 },
    {   __SET_RESTORE_DEV,  "7.Restore device", NULL,   _DTYPE_STRING,   0,      NULL,      3,  24, 0x00,    _LCD_SCR_SETTING_2 },
    {   __SET_OPTION_SENSOR,"8.Option Sensor",  NULL,   _DTYPE_STRING,   0,      NULL,      4,  24, 0x00,    _LCD_SCR_SETTING_2 },
    {   __SET_STATE_CALIB,  "9.State Calib",    NULL,   _DTYPE_STRING,   0,      NULL,      5,  24, 0x00,    _LCD_SCR_SETTING_2 },
    {   __SET_MANUFACTOR,   "10.Information",   NULL,   _DTYPE_STRING,   0,      NULL,      6,  24, 0x00,    _LCD_SCR_SETTING_2 },
    
    {   __SET_REQ_2_1,      "*Tsend Data:",     NULL,   _DTYPE_STRING,   0,      NULL,      2,  24, 0x00,    _LCD_SCR_SET_FREQ },
    {   __SET_REQ_2_2,      NULL,               NULL,   _DTYPE_U32,      0,      " (min)",  3,  36, 0x02,    _LCD_SCR_SET_FREQ },
    
    {   __SET_OPTION_SS_TITLE,  "OPTION SENSOR",  NULL,   _DTYPE_STRING,   0x00, NULL,        2,  26,  0x00,    _LCD_SCR_SET_OPTION_SENSOR  },
    {   __SET_OPTION_SS_PH,     "1. pH     : ",   NULL,   _DTYPE_U8,       0x00, NULL,        3,  14,  0x00,    _LCD_SCR_SET_OPTION_SENSOR  },
    {   __SET_OPTION_SS_CLO,    "2. Clo    : ",   NULL,   _DTYPE_U8,       0x00, NULL,        4,  14,  0x00,    _LCD_SCR_SET_OPTION_SENSOR  },
    {   __SET_OPTION_SS_EC,     "3. EC     : ",   NULL,   _DTYPE_U8,       0x00, NULL,        5,  14,  0x00,    _LCD_SCR_SET_OPTION_SENSOR  },
    {   __SET_OPTION_SS_TURB,   "4. TUR    : ",   NULL,   _DTYPE_U8,       0x00, NULL,        6,  14,  0x00,    _LCD_SCR_SET_OPTION_SENSOR  },
    {   __SET_OPTION_SS_COD,    "5. COD    : ",   NULL,   _DTYPE_U8,       0x00, NULL,        7,  14,  0x00,    _LCD_SCR_SET_OPTION_SENSOR  },

    {   __SET_OPTION_SS_TITLE_2,"OPTION SENSOR",  NULL,   _DTYPE_STRING,   0x00, NULL,        2,  26,  0x00,    _LCD_SCR_SET_OPTION_SENSOR_TAB_2 },
    {   __SET_OPTION_SS_TSS,    "6. TSS    : ",   NULL,   _DTYPE_U8,       0x00, NULL,        3,  14,  0x00,    _LCD_SCR_SET_OPTION_SENSOR_TAB_2 },
    {   __SET_OPTION_SS_NH4,    "7. NH4    : ",   NULL,   _DTYPE_U8,       0x00, NULL,        4,  14,  0x00,    _LCD_SCR_SET_OPTION_SENSOR_TAB_2 },
    {   __SET_OPTION_SS_DO,     "8. DO     : ",   NULL,   _DTYPE_U8,       0x00, NULL,        5,  14,  0x00,    _LCD_SCR_SET_OPTION_SENSOR_TAB_2 },
    {   __SET_OPTION_SS_SALT,   "9. SALT   : ",   NULL,   _DTYPE_U8,       0x00, NULL,        6,  14,  0x00,    _LCD_SCR_SET_OPTION_SENSOR_TAB_2 },
    {   __SET_OPTION_SS_TDS,    "10.TDS    : ",   NULL,   _DTYPE_U8,       0x00, NULL,        7,  14,  0x00,    _LCD_SCR_SET_OPTION_SENSOR_TAB_2 },
    
    {   __SET_OPTION_SS_TITLE_3,"OPTION SENSOR",  NULL,   _DTYPE_STRING,   0x00, NULL,        2,  26,  0x00,    _LCD_SCR_SET_OPTION_SENSOR_TAB_3 },
    {   __SET_OPTION_SS_NO3,    "11.NO3    : ",   NULL,   _DTYPE_U8,       0x00, NULL,        3,  14,  0x00,    _LCD_SCR_SET_OPTION_SENSOR_TAB_3 },
    {   __SET_OPTION_SS_TEMP,   "12.TEMP   : ",   NULL,   _DTYPE_U8,       0x00, NULL,        4,  14,  0x00,    _LCD_SCR_SET_OPTION_SENSOR_TAB_3 },
    
    {   __CHECK_STATE_SETTING,        NULL,             NULL,   _DTYPE_STRING,   0,      NULL,      1,  24, 0x00,     _LCD_SCR_CHECK_SETTING},
    
    {   __SET_PRESSURE_1_1, "1.Pressure CH.1",  NULL,   _DTYPE_STRING,   0,      NULL,      1,  24, 0x00,    _LCD_SCR_SET_PRESS_1 },
    {   __SET_PRESSURE_1_2, "2.Pressure CH.2",  NULL,   _DTYPE_STRING,   0,      NULL,      2,  24, 0x00,    _LCD_SCR_SET_PRESS_1 },
    {   __SET_PRESSURE_1_3, "3.Pressure CH.3",  NULL,   _DTYPE_STRING,   0,      NULL,      3,  24, 0x00,    _LCD_SCR_SET_PRESS_1 },
    {   __SET_PRESSURE_1_4, "4.Pressure CH.4",  NULL,   _DTYPE_STRING,   0,      NULL,      4,  24, 0x00,    _LCD_SCR_SET_PRESS_1 },
    {   __SET_PRESSURE_1_5, "5.Pressure CH.5",  NULL,   _DTYPE_STRING,   0,      NULL,      5,  24, 0x00,    _LCD_SCR_SET_PRESS_1 },
    
    {   __SET_PRESSURE_1_6, "6.Pressure CH.6",  NULL,   _DTYPE_STRING,   0,      NULL,      1,  24, 0x00,    _LCD_SCR_SET_PRESS_1_2 },
     
    {   __SET_PRESSURE_2,   "*Linear Inter:",   NULL,   _DTYPE_STRING,   0,      NULL,      1,  24, 0x00,    _LCD_SCR_SET_PRESS_2 },
    {   __SET_PRESSURE_2_0, "Type: ",           NULL,   _DTYPE_STRING,   0,      NULL,      2,  0,  0x02,    _LCD_SCR_SET_PRESS_2 },
    {   __SET_PRESSURE_2_1, "Factor: ",         NULL,   _DTYPE_U16,      0,      NULL,      3,  0,  0x02,    _LCD_SCR_SET_PRESS_2 },
    {   __SET_PRESSURE_2_2, "In : ",            NULL,   _DTYPE_U16,      0,      NULL,      4,  0,  0x02,    _LCD_SCR_SET_PRESS_2 },
    {   __SET_PRESSURE_2_3, " - ",              NULL,   _DTYPE_U16,      0,      NULL,      4,  54, 0x02,    _LCD_SCR_SET_PRESS_2 },
    {   __SET_PRESSURE_2_4, NULL,               NULL,   _DTYPE_STRING,   0,      NULL,      4,  108,0x02,    _LCD_SCR_SET_PRESS_2 },
    
    {   __SET_PRESSURE_2_5, "Out: ",            NULL,   _DTYPE_U16,      0,      NULL,      5,  0,  0x02,    _LCD_SCR_SET_PRESS_2 },
    {   __SET_PRESSURE_2_6, " - ",              NULL,   _DTYPE_U16,      0,      NULL,      5,  54, 0x02,    _LCD_SCR_SET_PRESS_2 },
    {   __SET_PRESSURE_2_7, NULL,               NULL,   _DTYPE_STRING,   0,      NULL,      5,  108,0x02,    _LCD_SCR_SET_PRESS_2 },
    
    //cai dat he so xung
    {   __SET_PULSE_SET1_1, "1.Pulse CH.1",     NULL,   _DTYPE_STRING,   0,      NULL,      1,  24, 0x00,    _LCD_SCR_SET_PULSE_1 },
    {   __SET_PULSE_SET1_2, "2.Pulse CH.2",     NULL,   _DTYPE_STRING,   0,      NULL,      2,  24, 0x00,    _LCD_SCR_SET_PULSE_1 },
    {   __SET_PULSE_SET1_3, "3.Pulse CH.3",     NULL,   _DTYPE_STRING,   0,      NULL,      3,  24, 0x00,    _LCD_SCR_SET_PULSE_1 },
    {   __SET_PULSE_SET1_4, "4.Pulse CH.4",     NULL,   _DTYPE_STRING,   0,      NULL,      4,  24, 0x00,    _LCD_SCR_SET_PULSE_1 },
    
    //
    {   __SET_PULSE_SET2_1, "Factor: ",         NULL,   _DTYPE_U8,       0,      NULL,      2,  0,  0x00,    _LCD_SCR_SET_PULSE_2 },
    {   __SET_PULSE_SET2_2, "Start: ",          NULL,   _DTYPE_U64,      0,      NULL,      3,  0,  0x02,    _LCD_SCR_SET_PULSE_2 },
    
    {   __SET_PULSE_RS_1,   "1.Reset CH.1",     NULL,   _DTYPE_STRING,   0,      NULL,      1,  24, 0x00,    _LCD_SCR_PULSE_RESET },
    {   __SET_PULSE_RS_2,   "2.Reset CH.2",     NULL,   _DTYPE_STRING,   0,      NULL,      2,  24, 0x00,    _LCD_SCR_PULSE_RESET },
    {   __SET_PULSE_RS_3,   "3.Reset CH.3",     NULL,   _DTYPE_STRING,   0,      NULL,      3,  24, 0x00,    _LCD_SCR_PULSE_RESET },
    {   __SET_PULSE_RS_4,   "4.Reset CH.4",     NULL,   _DTYPE_STRING,   0,      NULL,      4,  24, 0x00,    _LCD_SCR_PULSE_RESET },
    
    {   __SET_RESET_CONF_1, "Pulse Reset?",     NULL,   _DTYPE_STRING,   0,      NULL,      2,  24, 0x00,    _LCD_SCR_RS_CONFIRM },
    {   __SET_RESET_CONF_2, NULL,               NULL,   _DTYPE_STRING,   0,      NULL,      3,  54, 0x00,    _LCD_SCR_RS_CONFIRM },
    
    {   __SET_RES_CONF_1,   "Restore Device?",  NULL,   _DTYPE_STRING,   0,      NULL,      2,  24, 0x00,    _LCD_SCR_RESTORE_DEV },
    {   __SET_RES_CONF_2,   NULL,               NULL,   _DTYPE_STRING,   0,      NULL,      3,  54, 0x00,    _LCD_SCR_RESTORE_DEV },
    
    {   __SET_CALIB_SS_1,   "Sensor calibrating?", NULL, _DTYPE_STRING,  0,      NULL,      2,  10, 0x00,    _LCD_SCR_CALIB_SS },
    {   __SET_CALIB_SS_2,   NULL,               NULL,    _DTYPE_STRING,  0,      NULL,      3,  54, 0x00,    _LCD_SCR_CALIB_SS },
    
    {   __SET_SUCCESS,      "Successfully!",    NULL,   _DTYPE_STRING,   0,      NULL,      3,  24, 0x02,    _LCD_SCR_SET_SUCCESS},
    
    {   __CAL_PRESS_CH_1,   "1.Calib P_CH.1",   NULL,   _DTYPE_STRING,   0,      NULL,      1,  24, 0x00,    _LCD_SCR_CAL_CHANN_1 },
    {   __CAL_PRESS_CH_2,   "2.Calib P_CH.2",   NULL,   _DTYPE_STRING,   0,      NULL,      2,  24, 0x00,    _LCD_SCR_CAL_CHANN_1 },
    {   __CAL_PRESS_CH_3,   "3.Calib P_CH.3",   NULL,   _DTYPE_STRING,   0,      NULL,      3,  24, 0x00,    _LCD_SCR_CAL_CHANN_1 },
    {   __CAL_PRESS_CH_4,   "4.Calib P_CH.4",   NULL,   _DTYPE_STRING,   0,      NULL,      4,  24, 0x00,    _LCD_SCR_CAL_CHANN_1 },
    {   __CAL_PRESS_CH_5,   "5.Calib P_CH.5",   NULL,   _DTYPE_STRING,   0,      NULL,      5,  24, 0x00,    _LCD_SCR_CAL_CHANN_1 },
    
    {   __CAL_PRESS_CH_6,   "6.Calib P_CH.6",   NULL,   _DTYPE_STRING,   0,      NULL,      1,  24, 0x00,    _LCD_SCR_CAL_CHANN_2 },
    
    {   __CAL_PRESSURE_1,   "Cur Calib: ",      NULL,   _DTYPE_I16,      0,      " (mV)",   1,  6,  0x00,    _LCD_SCR_CAL_PRESS }, 
    {   __CAL_PRESSURE_2,   "Vadc: ",           NULL,   _DTYPE_I16,      0,      " (mV)",   2,  6,  0x02,    _LCD_SCR_CAL_PRESS },
    
    {   __CAL_PRESSURE_3,   "*Take Input 0 (V)",NULL,   _DTYPE_STRING,   0,      NULL,      4,  0,  0x00,    _LCD_SCR_CAL_PRESS },
    {   __CAL_PRESSURE_4, "Press Enter To Save",NULL,   _DTYPE_STRING,   0,      NULL,      5,  0,  0x00,    _LCD_SCR_CAL_PRESS },
    
    {   __SET_LEV_SET1_1,   "1.Level CH.1",     NULL,   _DTYPE_STRING,   0,      NULL,      1,  24, 0x00,    _LCD_SCR_LEV_SET_1 },
    {   __SET_LEV_SET1_2,   "2.Level CH.2",     NULL,   _DTYPE_STRING,   0,      NULL,      2,  24, 0x00,    _LCD_SCR_LEV_SET_1 },
    {   __SET_LEV_SET1_3,   "3.Level CH.3",     NULL,   _DTYPE_STRING,   0,      NULL,      3,  24, 0x00,    _LCD_SCR_LEV_SET_1 },
    {   __SET_LEV_SET1_4,   "4.Level CH.4",     NULL,   _DTYPE_STRING,   0,      NULL,      4,  24, 0x00,    _LCD_SCR_LEV_SET_1 },
    
    {   __SET_LEV_SET2_1,   "Lwir : ",          NULL,   _DTYPE_U16,      0,      " (m)",    1,  24, 0x00,    _LCD_SCR_LEV_SET_2 },
    {   __SET_LEV_SET2_2,   "Lsta : ",          NULL,   _DTYPE_U16,      0,      " (m)",    2,  24, 0x00,    _LCD_SCR_LEV_SET_2 },
    {   __SET_LEV_SET2_3,   "Ldym : ",          NULL,   _DTYPE_U16,      0,      " (m)",    3,  24, 0x00,    _LCD_SCR_LEV_SET_2 },
    {   __SET_LEV_SET2_4,   NULL,               NULL,   _DTYPE_STRING,   0,      NULL,      5,  0,  0x00,    _LCD_SCR_LEV_SET_2 },
};

static char charNotDetectPress = '-';

static const SENSOR_CFG SensorTable[] =
{
    { __SC1_CLO_DU,     &s485Measure[_SS_CLO].sUser,    &sDataSensorMeasure.sClo.State_u8,    &sAverageMeasure[_SS_CLO].State},
    { __SC1_PH_WATER,   &s485Measure[_SS_PH].sUser,     &sDataSensorMeasure.spH.State_u8,     &sAverageMeasure[_SS_PH].State},
    { __SC1_TURB,       &s485Measure[_SS_TURB].sUser,   &sDataSensorMeasure.sTurb.State_u8,   &sAverageMeasure[_SS_TURB].State},
    { __SC1_EC,         &s485Measure[_SS_EC].sUser,     &sDataSensorMeasure.sEC.State_u8,     &sAverageMeasure[_SS_EC].State},
    { __SC1_SALINITY,   &s485Measure[_SS_SALT].sUser,   &sDataSensorMeasure.sSal.State_u8,    &sAverageMeasure[_SS_SALT].State},
    { __SC1_COD,        &s485Measure[_SS_COD].sUser,    &sDataSensorMeasure.sCOD.State_u8,    &sAverageMeasure[_SS_COD].State},
    { __SC1_TSS,        &s485Measure[_SS_TSS].sUser,    &sDataSensorMeasure.sTSS.State_u8,    &sAverageMeasure[_SS_TSS].State},
    { __SC1_NH4,        &s485Measure[_SS_NH4].sUser,    &sDataSensorMeasure.sNH4.State_u8,    &sAverageMeasure[_SS_NH4].State},
    { __SC1_DO,         &s485Measure[_SS_DO].sUser,     &sDataSensorMeasure.sDO.State_u8,     &sAverageMeasure[_SS_DO].State},
    { __SC1_TDS,        &s485Measure[_SS_TDS].sUser,    &sDataSensorMeasure.sTDS.State_u8,    &sAverageMeasure[_SS_TDS].State},
    { __SC1_NO3,        &s485Measure[_SS_NO3].sUser,    &sDataSensorMeasure.sNO3.State_u8,    &sAverageMeasure[_SS_NO3].State},
    
    { __SC1_TEMP,       &s485Measure[_SS_TEMP].sUser,   &sDataSensorMeasure.sTemp.State_u8,   &sAverageMeasure[_SS_TEMP].State},
};

static uint8_t IsSensorDisabled(uint8_t id)
{
    for (uint8_t i = 0; i < (sizeof(SensorTable) / sizeof(SensorTable[0])); i++)
    {
        if (SensorTable[i].E_Kind == id)
        {
            if (SensorTable[i].Enable == NULL)
                return 0;   

            return (*(SensorTable[i].Enable) == _INACTIVE_SENSOR);
        }
    }
    return 0;  
}
/*===================Function=========================*/
void Display_Init (void)
{
    static uint64_t TempPulse_u64 = 0;
    static uint32_t TempPulse_u32 = 0;
    //Init
    fevent_enable(sEventDisplay, _EVENT_DISP_INIT); 
//    fevent_enable(sEventDisplay, _EVENT_DISP_LOGO); 
    HAL_GPIO_WritePin (LCD_ON_OFF_GPIO_Port, LCD_ON_OFF_Pin, GPIO_PIN_RESET);   
//
//    glcd_init();
    
    //Gan cac bien data cho truong thong tin
    sLCDObject[__BATTERY_VOL].pData    = &sBattery.mVol_u32; 
    sLCDObject[__POWWER_12V].pData    = &sVout.mVol_u32;   
    sLCDObject[__RSSI].pData    = &sSimCommInfor.RSSI_u8;   
    sLCDObject[__SD_FREE].pData = &SD_Free_u32;  
    sLCDObject[__FREQ].pData    = &sModemInfor.sFrequence.DurOnline_u32;    
    
//    sLCDObject[__SC1_ID_DCU].pData      = sModemInfor.sId.Data_a8;
    sLCDObject[__SC1_CLO_DU].pData      = &sDataSensorMeasure.sClo.Value_i32 ; 
    sLCDObject[__SC1_CLO_DU].Scale_u8   = sDataSensorMeasure.sClo.Scale_u8; 
    sLCDObject[__SC1_PH_WATER].pData    = &sDataSensorMeasure.spH.Value_i32;    
    sLCDObject[__SC1_PH_WATER].Scale_u8 = sDataSensorMeasure.spH.Scale_u8;    
    sLCDObject[__SC1_TURB].pData         = &sDataSensorMeasure.sTurb.Value_i32;   
    sLCDObject[__SC1_TURB].Scale_u8      = sDataSensorMeasure.sTurb.Scale_u8;   
    sLCDObject[__SC1_SALINITY].pData    = &sDataSensorMeasure.sSal.Value_i32;    
    sLCDObject[__SC1_SALINITY].Scale_u8 = sDataSensorMeasure.sSal.Scale_u8; 
    sLCDObject[__SC1_TEMP].pData        = &sDataSensorMeasure.sTemp.Value_i32;    
    sLCDObject[__SC1_TEMP].Scale_u8     = sDataSensorMeasure.sTemp.Scale_u8;   
    sLCDObject[__SC1_EC].pData          = &sDataSensorMeasure.sEC.Value_i32;   
    sLCDObject[__SC1_EC].Scale_u8       = sDataSensorMeasure.sEC.Scale_u8;    
    
    sLCDObject[__SC1_COD].pData         = &sDataSensorMeasure.sCOD.Value_i32;   
    sLCDObject[__SC1_COD].Scale_u8      = sDataSensorMeasure.sCOD.Scale_u8;   
    sLCDObject[__SC1_TSS].pData         = &sDataSensorMeasure.sTSS.Value_i32;   
    sLCDObject[__SC1_TSS].Scale_u8      = sDataSensorMeasure.sTSS.Scale_u8; 
    sLCDObject[__SC1_NH4].pData         = &sDataSensorMeasure.sNH4.Value_i32;   
    sLCDObject[__SC1_NH4].Scale_u8      = sDataSensorMeasure.sNH4.Scale_u8; 
    sLCDObject[__SC1_DO].pData          = &sDataSensorMeasure.sDO.Value_i32;   
    sLCDObject[__SC1_DO].Scale_u8       = sDataSensorMeasure.sDO.Scale_u8; 
    sLCDObject[__SC1_TDS].pData         = &sDataSensorMeasure.sTDS.Value_i32;   
    sLCDObject[__SC1_TDS].Scale_u8      = sDataSensorMeasure.sTDS.Scale_u8; 
    sLCDObject[__SC1_NO3].pData         = &sDataSensorMeasure.sNO3.Value_i32;   
    sLCDObject[__SC1_NO3].Scale_u8      = sDataSensorMeasure.sNO3.Scale_u8; 
    
    sLCDObject[__SET_OPTION_SS_PH].pData      = &s485Measure[_SS_PH].sUser;
    sLCDObject[__SET_OPTION_SS_CLO].pData     = &s485Measure[_SS_CLO].sUser;
    sLCDObject[__SET_OPTION_SS_EC].pData      = &s485Measure[_SS_EC].sUser;
    sLCDObject[__SET_OPTION_SS_TURB].pData   = &s485Measure[_SS_TURB].sUser;
    sLCDObject[__SET_OPTION_SS_COD].pData   = &s485Measure[_SS_COD].sUser;
    
    sLCDObject[__SET_OPTION_SS_TSS].pData    = &s485Measure[_SS_TSS].sUser;
    sLCDObject[__SET_OPTION_SS_NH4].pData    = &s485Measure[_SS_NH4].sUser;
    sLCDObject[__SET_OPTION_SS_DO].pData     = &s485Measure[_SS_DO].sUser;
    sLCDObject[__SET_OPTION_SS_SALT].pData   = &s485Measure[_SS_SALT].sUser;
    sLCDObject[__SET_OPTION_SS_TDS].pData    = &s485Measure[_SS_TDS].sUser;
    sLCDObject[__SET_OPTION_SS_NO3].pData    = &s485Measure[_SS_NO3].sUser;
    sLCDObject[__SET_OPTION_SS_TEMP].pData    = &s485Measure[_SS_TEMP].sUser;
    
    sLCDObject[__PULSE_1].pData =  &sPulse[0].Total_lf;
    sLCDObject[__PULSE_1].Scale_u8 = sPulse[0].FactorDec_u8;
    sLCDObject[__PRESS_1].pData =  &sWmVar.aPRESSURE[0].Val_i32;
    sLCDObject[__FLOW_1].pData =  &sPulse[0].Flow_f;
    sLCDObject[__FLOW_1].Scale_u8 = sPulse[0].FactorDec_u8;
    
    sLCDObject[__PULSE_2].pData =  &sPulse[1].Total_lf;
    sLCDObject[__PRESS_2].pData =  &sWmVar.aPRESSURE[1].Val_i32;
    sLCDObject[__FLOW_2].pData =  &sPulse[1].Flow_f;
    
    sLCDObject[__PULSE_3].pData =  &sPulse[2].Total_lf;
    sLCDObject[__PRESS_3].pData =  &sWmVar.aPRESSURE[2].Val_i32;
    sLCDObject[__FLOW_3].pData =  &sPulse[2].Flow_f;
    
    sLCDObject[__PULSE_4].pData =  &sPulse[3].Total_lf;
    sLCDObject[__PRESS_4].pData =  &sWmVar.aPRESSURE[3].Val_i32;
    sLCDObject[__FLOW_4].pData =  &sPulse[3].Flow_f;
    
    sLCDObject[__PULSE_5].pData =  &TempPulse_u64;
    sLCDObject[__PRESS_5].pData =  &sWmVar.aPRESSURE[4].Val_i32;
    sLCDObject[__FLOW_5].pData =  &TempPulse_u32;
    
    sLCDObject[__PULSE_6].pData =  &TempPulse_u64;
    sLCDObject[__PRESS_6].pData =  &sWmVar.aPRESSURE[5].Val_i32;
    sLCDObject[__FLOW_6].pData =  &TempPulse_u32;
    
    sLCDObject[__SERIAL_1].pData        = sModemInfor.aID;
    sLCDObject[__FW_VERSION_1].pData    = sFirmVersion + 5;
    //
    sLCD.sScreenNow.Index_u8 = _LCD_SCREEN_1; 
    sLCD.sScreenNow.Para_u8 = 0xFF;
    sLCD.sScreenNow.ParaMin_u8 = 0;
    sLCD.sScreenNow.ParaMax_u8 = 0;
}



uint8_t Display_Task(void)
{
	uint8_t i = 0;

	for (i = 0; i < _EVENT_END_DISPLAY; i++)
	{
		if (sEventDisplay[i].e_status == 1)
		{
			if ((sEventDisplay[i].e_systick == 0) ||
					((HAL_GetTick() - sEventDisplay[i].e_systick)  >=  sEventDisplay[i].e_period))
			{
                sEventDisplay[i].e_status = 0;
				sEventDisplay[i].e_systick = HAL_GetTick();
				sEventDisplay[i].e_function_handler(i);
			}
		}
	}
    
	return 0;
}



/*----------- Func callback ------------*/
static uint8_t _Cb_Display_Init (uint8_t event)
{
    if (sLCD.Ready_u8 == false)
    {
        UTIL_Printf_Str(DBLEVEL_M, "u_lcd: init...\r\n");
        
        HAL_GPIO_WritePin (LCD_ON_OFF_GPIO_Port, LCD_ON_OFF_Pin, GPIO_PIN_RESET);   

        glcd_init();
        glcd_tiny_set_font(Font5x7, 5, 7, 32, 127 + 9);
        
        sLCD.Ready_u8 = true;
            
        fevent_enable(sEventDisplay, _EVENT_DISP_LOGO); 
    }
    
	return 1;
}


static uint8_t _Cb_Display_Logo (uint8_t event)
{
    static uint8_t step_u8 = 0;
         
    switch (step_u8)
    {
        case 0:
            glcd_test_bitmap_128x64();
            break;
        case 1:
            glcd_clear_buffer();
            glcd_set_font(Liberation_Sans17x17_Alpha, 17, 17, 65, 90);
            
            glcd_draw_string_xy(30, 13, "SV");
            
            glcd_set_pixel(60, 25, BLACK);
            glcd_set_pixel(60, 26, BLACK);
            glcd_set_pixel(61, 26, BLACK);
            glcd_set_pixel(61, 25, BLACK);
            
            glcd_draw_string_xy(65, 13, "JSC");
            
            glcd_tiny_set_font(Font5x7, 5, 7, 32, 127 + 10);
            glcd_tiny_draw_string(9, 4, "CHAT LUONG LA NIEM");
            glcd_tiny_draw_string(54, 5, "TIN");
            
            glcd_draw_rect(0, 0, 128, 64,BLACK);
            
            glcd_write();	
            break;
        default:
            sLCD.Ready_u8 = true;
            glcd_tiny_set_font(Font5x7, 5, 7, 32, 127 + 10);
            fevent_active(sEventDisplay, _EVENT_DISP_SHOW); 
            fevent_active(sEventDisplay, _EVENT_DISP_AUTO_SW); 
            fevent_active(sEventDisplay, _EVENT_BUTTON_SCAN); 
            return 1;
    }
    step_u8++;
    fevent_enable(sEventDisplay, event); 
    
    return 1;
}



static uint8_t _Cb_Display_Show (uint8_t event)
{
    static uint8_t ScreenLast = 0;
    static uint32_t LandMarkChange_u32 = 0;
    
    //update cac bien moi cai dat
    Update_ParaDisplay();
    Display_Update();
    //hien thi man hinh: index
    if (sLCD.Ready_u8 == true) {
        //neu index 9: modbus disp: gan lai cac bien va subindex
        if (sLCD.sScreenNow.Index_u8 == _LCD_SCREEN_7b) {
            Display_Setup_SCREEN_Modb();
        }
        
        //hien thị
        Display_Show_Screen(sLCD.sScreenNow.Index_u8);
    } else {
        fevent_active(sEventDisplay, _EVENT_DISP_INIT); 
    }
    
    //ghi moc thoi gian man hinh dc chuyen: cho su kien auto next
    if (sLCD.sScreenNow.Index_u8 != ScreenLast) {
        ScreenLast = sLCD.sScreenNow.Index_u8;
        LandMarkChange_u32 = RtCountSystick_u32;
    }
    
    //hien thi man hinh cho "OK" sau do quay lai man hinh truoc do
    if ( (sLCD.sScreenNow.Index_u8 == _LCD_SCR_SET_SUCCESS)
        && (Check_Time_Out(LandMarkChange_u32, 2000) == true ) ) {
        LandMarkChange_u32 = RtCountSystick_u32;
        //doi ve man hinh truoc do
        UTIL_MEM_cpy(&sLCD.sScreenNow, &sLCD.sScreenBack, sizeof(sScreenInformation));
    }
    
    fevent_enable(sEventDisplay, event); 
    
	return 1;
}



static uint8_t _Cb_Display_Auto_SW (uint8_t event)
{
    static uint16_t cNext = 0;
    static uint8_t MarkButtPressed = false;
    
    if (Check_Time_Out(sButton.LandMarkPressButton_u32, TIMEOUT_SETTING) == true) {
        if (MarkButtPressed == true) {
            MarkButtPressed = false;
            sLCD.sScreenNow.Index_u8 = _LCD_SCREEN_1;
        }
        
        cNext++;
        if (cNext >= (DURATION_DISPLAY/sEventDisplay[event].e_period)) {
            cNext = 0;
            
            if (sLCD.sScreenNow.Index_u8 == _LCD_SCREEN_7b) {                
                sLCD.sScreenNow.SubIndex_u8++;
                if (sLCD.sScreenNow.SubIndex_u8 >= sWmDigVar.nModbus_u8) {
                    sLCD.sScreenNow.Index_u8++;
                }
            } else {
                if (sLCD.sScreenNow.Index_u8 == _LCD_SCREEN_7) {
                    sLCD.sScreenNow.SubIndex_u8 = 0;
                }
                
                sLCD.sScreenNow.Index_u8++;
            }
            
//            if (sLCD.sScreenNow.Index_u8 > _LCD_SCREEN_7b) {
//                sLCD.sScreenNow.Index_u8 = _LCD_SCREEN_1;
//            }
            if (sLCD.sScreenNow.Index_u8 > _LCD_SCREEN_CM44) {
                sLCD.sScreenNow.Index_u8 = _LCD_SCREEN_1;
            }
            
        }
    } else {
        MarkButtPressed = true;
    }
    
    fevent_enable(sEventDisplay, event); 
    
	return 1;
}




/*------------ Func Handle -------------*/
/*
    Func: show all oject of screen

*/


void Display_Update (void)
{
    //relate struct global aUnitWm
    static char aUnitWm[5][10] =  
    {
        {" (mV)"},
        {" (mA)"},
        {" (V)"},
        {" (met)"},
        {" (bar)"},
    };
    static char sAnalogName[2][10] = {"P : ", "L : "};
    
    uint8_t StepChann = __CHANEL_2 - __CHANEL_1;
    
    
    for (uint8_t i = 0; i < MAX_CHANNEL; i++) {
        //update unit pressure and scale pulse
        sLCDObject[__PRESS_1 + i * StepChann].Unit = aUnitWm[sWmVar.aPRESSURE[i].sLinearInter.OutUnit_u8];
        sLCDObject[__PULSE_1 + i * StepChann].Scale_u8 =  sPulse[i].FactorDec_u8;
        //update val press = -1 to convert string '-'
        sLCDObject[__PRESS_1 + i * StepChann].sName =  sAnalogName[sWmVar.aPRESSURE[i].sLinearInter.Type_u8];
        if (sWmVar.aPRESSURE[i].Val_i32 == -1000) {
            sLCDObject[__PRESS_1 + i * StepChann].pData = &charNotDetectPress;
            sLCDObject[__PRESS_1 + i * StepChann].dType_u8 = _DTYPE_CHAR;
        } else {
            sLCDObject[__PRESS_1 + i * StepChann].pData = &sWmVar.aPRESSURE[i].Val_i32;
            sLCDObject[__PRESS_1 + i * StepChann].dType_u8 = _DTYPE_I32;
        }
    }
}


void Display_Show_Screen (uint8_t screen)
{
    uint16_t  i = 0; 
    
    //Clear buff lcd data
    glcd_clear_buffer();
    //Show static param: stime, icon internet,...
    Display_Show_Static_Param();
    
    //Show state connect sensor
    Display_Update_ScrSensor(screen);
    Display_Show_State_Sensor_Network(screen);
    Display_Show_State_Calib_Sensor(screen);
    
    //Show name of screen
    if (screen >= _LCD_SCR_CAL_CHANN_1) {
        glcd_tiny_draw_string(0, 0, "Cal.");
    } else if (screen >= _LCD_SCR_SETTING) {
        glcd_tiny_draw_string(0, 0, "Set.");
    }
    
//    for (i = 0; i < __OJECT_END; i++) {
//        if (sLCDObject[i].Screen_u8 == screen) {
//            Display_Show_Oject(i);
//        }
//    }
    
    for (i = 0; i < __OJECT_END; i++)
    {
        if (sLCDObject[i].Screen_u8 != screen)
            continue;

        if (IsSensorDisabled(i))
            continue;

        Display_Show_Oject(i);
    }
    
    glcd_write();
}

void Display_Show_Oject (uint8_t object)
{
    char aTEMP[32] = {0}; 
    int64_t TempVal = 0;
    uint8_t type = 0, temp = 0;
    uint16_t PosX = sLCDObject[object].Col_u8;
    
    //show name
    if (sLCDObject[object].sName != NULL) {
        if (Display_Check_Toggle(object, 0x01) == false) {
            glcd_tiny_draw_string(PosX, sLCDObject[object].Row_u8, sLCDObject[object].sName);
        } else {
            for (uint8_t i = 0; i < strlen(sLCDObject[object].sName); i++) {
                aTEMP[i] = ' ';
            }
            
            glcd_tiny_draw_string(PosX, sLCDObject[object].Row_u8, aTEMP);
        }
   
        PosX += strlen(sLCDObject[object].sName) * (font_current.width + 1);
    }
        
    //show value
    if (sLCDObject[object].pData != NULL) {
        switch (sLCDObject[object].dType_u8) 
        {
            case _DTYPE_U8:
                TempVal = *( (uint8_t *) sLCDObject[object].pData );
                break;
            case _DTYPE_I8:
                TempVal = *( (int8_t *) sLCDObject[object].pData );
                break;
            case _DTYPE_U16:
                TempVal = *( (uint16_t *) sLCDObject[object].pData );
                break;
            case _DTYPE_I16:
                TempVal = *( (int16_t *) sLCDObject[object].pData );
                break;
            case _DTYPE_U32:
                TempVal = *( (uint32_t *) sLCDObject[object].pData );
                break;
            case _DTYPE_I32:
                TempVal = *( (int32_t *) sLCDObject[object].pData );
                break;
            case _DTYPE_U64:
                TempVal = *( (uint64_t *) sLCDObject[object].pData );
                break;
            case _DTYPE_I64:
                TempVal = *( (int64_t *) sLCDObject[object].pData );
                break;
            case _DTYPE_STRING:
                type = 1;
                break;
            case _DTYPE_CHAR:
                type = 2;
                break;
            case _DTYPE_FLOAT:
                TempVal = (uint64_t) (*(float *) sLCDObject[object].pData * Convert_Scale(0xFF - sLCDObject[object].Scale_u8 + 1));
                type = 3;
                break; 
            case _DTYPE_DOUBLE:
                TempVal = (uint64_t) (*(double *) sLCDObject[object].pData * Convert_Scale(0xFF - sLCDObject[object].Scale_u8 + 1));
                type = 3;
                break; 
        }
        
        switch (type)
        {
            case 0:
                UtilIntToStringWithScale (TempVal, aTEMP, 0xFF - sLCDObject[object].Scale_u8 + 1);
                break;
            case 1:
                if (strlen ((char *) sLCDObject[object].pData) < sizeof (aTEMP))    
                    UTIL_MEM_cpy( aTEMP, (char *) sLCDObject[object].pData, strlen ((char *) sLCDObject[object].pData) );
                break;
            case 2:
                aTEMP[0] = * ( (char *)sLCDObject[object].pData );
                break;
            case 3:
                UtilIntToStringWithScale (TempVal, aTEMP, 0xFF - sLCDObject[object].Scale_u8 + 1);
                break;
        }

        if (Display_Check_Toggle(object, 0x02) == true) {
            //check xem nhay all hay nhay 1 vi tri
            temp = (sLCDObject[object].Mode_u8 >> 4) & 0x0F; 
            if ( temp != 0x0F) {
                aTEMP[temp] = ' ';
            } else {
                for (uint8_t i = 0; i < strlen(aTEMP); i++)
                    aTEMP[i] = ' ';
            }
        }
        
        glcd_tiny_draw_string(PosX, sLCDObject[object].Row_u8, aTEMP);
        PosX += strlen(aTEMP)* (font_current.width + 1);
    }
    
    //Show Unit

    if ( (sLCDObject[object].Unit != NULL)
        && ( (sLCDObject[object].pData != NULL) && (sLCDObject[object].pData != &charNotDetectPress) ) ) {
        glcd_tiny_draw_string(PosX, sLCDObject[object].Row_u8, (char *) sLCDObject[object].Unit );
    }
}


static uint8_t _Cb_button_scan (uint8_t event)
{
    BUTTON_scan();
    
    if (sButton.Status == 1) {
        fevent_active(sEventDisplay, _EVENT_BUTTON_DETECTTED);
        sButton.LandMarkPressButton_u32 = RtCountSystick_u32;
    }

    fevent_enable(sEventDisplay, event);
    
	return 1;
}


static uint8_t _Cb_button_detect (uint8_t event)
{
    BUTTON_Process();
    
    sButton.Value = 0;
    sButton.Status = 0;
    
	return 1;
}


/*
    Func: check toggle object
        + creat toggle effect
*/

uint8_t Display_Check_Toggle (uint8_t object, uint8_t Flag)
{
    static uint32_t LandMarkToggle_u32[3][__OJECT_END] = {0};
    static uint8_t Hide[3][__OJECT_END] = {0};

    if ((sLCDObject[object].Mode_u8 & Flag) == Flag) {
        if (Check_Time_Out(LandMarkToggle_u32[Flag][object], TIME_TOGGLE) == true) {
            LandMarkToggle_u32[Flag][object] = RtCountSystick_u32;
            Hide[Flag][object] =  1- Hide[Flag][object];
        }
    } else {
        Hide[Flag][object] = false;
    }
    
    return Hide[Flag][object];
}

/*
    Func: setup screen modbus: many slave and many type data
        + change data modbus: name, pdata, scale....
*/

void Display_Setup_SCREEN_Modb (void)
{
    static char aCHANNEL[] = "Modb.1  ";

    if (sLCD.sScreenNow.SubIndex_u8 >= sWmDigVar.nModbus_u8) {
        return;
    }
    
    //check status data cua meter
    if (sWmDigVar.sModbDevData[sLCD.sScreenNow.SubIndex_u8].Status_u8 == true) {
        aCHANNEL[5] = sLCD.sScreenNow.SubIndex_u8 + 0x31;
        
        sLCDObject[__CHANEL_7].sName = aCHANNEL;
        
        switch (sWmDigVar.sModbDevData[sLCD.sScreenNow.SubIndex_u8].Type_u8)
        {
            case __MET_WOTECK:  //wm
            case __MET_MT100:  
            case __MET_WM_SENCE: 
            case __MET_WOTECK_ULTRA: 
                sLCDObject[__PULSE_7].sName = "To : ";
                sLCDObject[__PULSE_7].pData = &sWmDigVar.sModbDevData[sLCD.sScreenNow.SubIndex_u8].nTotal_i64;
                sLCDObject[__PULSE_7].dType_u8 = _DTYPE_I64;
                sLCDObject[__PULSE_7].Scale_u8 = sWmDigVar.sModbDevData[sLCD.sScreenNow.SubIndex_u8].Factor;
                sLCDObject[__PULSE_7].Unit = sWmDigVar.sModbDevData[sLCD.sScreenNow.SubIndex_u8].sTotalUnit;

                sLCDObject[__PULSE_7_2].sName = "For: ";
                sLCDObject[__PULSE_7_2].pData = &sWmDigVar.sModbDevData[sLCD.sScreenNow.SubIndex_u8].nForw_i64;
                sLCDObject[__PULSE_7_2].dType_u8 = _DTYPE_U64;
                sLCDObject[__PULSE_7_2].Scale_u8 = sWmDigVar.sModbDevData[sLCD.sScreenNow.SubIndex_u8].Factor;
                sLCDObject[__PULSE_7_2].Unit = sWmDigVar.sModbDevData[sLCD.sScreenNow.SubIndex_u8].sTotalUnit;
                
                sLCDObject[__PULSE_7_3].sName = "Rev: ";
                sLCDObject[__PULSE_7_3].pData = &sWmDigVar.sModbDevData[sLCD.sScreenNow.SubIndex_u8].nRev_i64;
                sLCDObject[__PULSE_7_3].dType_u8 = _DTYPE_U64;
                sLCDObject[__PULSE_7_3].Scale_u8 = sWmDigVar.sModbDevData[sLCD.sScreenNow.SubIndex_u8].Factor;
                sLCDObject[__PULSE_7_3].Unit = sWmDigVar.sModbDevData[sLCD.sScreenNow.SubIndex_u8].sTotalUnit;
                
                sLCDObject[__FLOW_7].sName = "F  : ";
                sLCDObject[__FLOW_7].pData = &sWmDigVar.sModbDevData[sLCD.sScreenNow.SubIndex_u8].Flow_i32;
                sLCDObject[__FLOW_7].dType_u8 = _DTYPE_I32;
                sLCDObject[__FLOW_7].Scale_u8 = sWmDigVar.sModbDevData[sLCD.sScreenNow.SubIndex_u8].Factor;
                sLCDObject[__FLOW_7].Unit = sWmDigVar.sModbDevData[sLCD.sScreenNow.SubIndex_u8].sFlowUnit; 
                
                sLCDObject[__PRESS_7].sName = "P  : ";
                sLCDObject[__PRESS_7].pData = &charNotDetectPress;
                sLCDObject[__PRESS_7].dType_u8 = _DTYPE_CHAR; 
                sLCDObject[__PRESS_7].Unit = NULL; 
                sLCDObject[__PRESS_7].Scale_u8 = 0x00;
                break;
            case __MET_LEVEL_LIQ: //level
            case __MET_LEVEL_ULTRA: 
            case __MET_LEVEL_LIQ_SUP: 
                sLCDObject[__PULSE_7].pData = NULL;
                sLCDObject[__PULSE_7].sName = NULL;
                
                sLCDObject[__PULSE_7_2].sName = "Lsen : ";
                sLCDObject[__PULSE_7_2].pData = &sWmDigVar.sModbDevData[sLCD.sScreenNow.SubIndex_u8].LVal_i16;
                sLCDObject[__PULSE_7_2].dType_u8 = _DTYPE_I16;
                sLCDObject[__PULSE_7_2].Scale_u8 = 0xFF - sWmDigVar.sModbDevData[sLCD.sScreenNow.SubIndex_u8].LDecimal_u16 + 1; 
                if (sWmDigVar.sModbDevData[sLCD.sScreenNow.SubIndex_u8].LUnit_u16 == 1) {
                    sLCDObject[__PULSE_7_2].Unit = " (cm)";
                } else {
                    sLCDObject[__PULSE_7_2].Unit = " (m)";
                }
            
                sLCDObject[__PULSE_7_3].sName = "Lsta : ";
                sLCDObject[__PULSE_7_3].pData = &sWmDigVar.sModbDevData[sLCD.sScreenNow.SubIndex_u8].Lstatic_u16;
                sLCDObject[__PULSE_7_3].dType_u8 = _DTYPE_U16;
                sLCDObject[__PULSE_7_3].Scale_u8 = 0xFF - sWmDigVar.sModbDevData[sLCD.sScreenNow.SubIndex_u8].LDecimal_u16 + 1;  
                sLCDObject[__PULSE_7_3].Unit = sLCDObject[__PULSE_7_2].Unit;
                
                sLCDObject[__FLOW_7].sName = "Ldym : ";
                sLCDObject[__FLOW_7].pData = &sWmDigVar.sModbDevData[sLCD.sScreenNow.SubIndex_u8].Ldynamic_u16;
                sLCDObject[__FLOW_7].dType_u8 = _DTYPE_U16;
                sLCDObject[__FLOW_7].Scale_u8 = 0xFF - sWmDigVar.sModbDevData[sLCD.sScreenNow.SubIndex_u8].LDecimal_u16 + 1;  
                sLCDObject[__FLOW_7].Unit = sLCDObject[__PULSE_7_2].Unit;
                
                sLCDObject[__PRESS_7].sName = "Ldel : ";
                sLCDObject[__PRESS_7].pData = &sWmDigVar.sModbDevData[sLCD.sScreenNow.SubIndex_u8].Ldelta_i16;
                sLCDObject[__PRESS_7].dType_u8 = _DTYPE_I16;
                sLCDObject[__PRESS_7].Scale_u8 = 0xFF - sWmDigVar.sModbDevData[sLCD.sScreenNow.SubIndex_u8].LDecimal_u16 + 1; 
                sLCDObject[__PRESS_7].Unit = sLCDObject[__PULSE_7_2].Unit;  
                
                break;
        }              
    } else {
        if (sButton.Old_value == _LCD_SCREEN_1) {
            //dang giam
            if (sLCD.sScreenNow.SubIndex_u8 > 0) {
                sLCD.sScreenNow.SubIndex_u8--;
            } else {
                sLCD.sScreenNow.Index_u8--;
            }
        } else {
            if ( (sLCD.sScreenNow.SubIndex_u8 + 1) >= sWmDigVar.nModbus_u8 ) {
                sLCD.sScreenNow.SubIndex_u8 = 0;
                sLCD.sScreenNow.Index_u8 = _LCD_SCREEN_1;
            } else {
                sLCD.sScreenNow.SubIndex_u8++;
            }
        }
    }
}



void Display_Set_Screen_Flag (sScreenInformation *screen, void *pData, uint8_t flag)
{
    screen->Flag_u8 = flag;
    //setting mode    
    for (uint8_t i = screen->ParaMin_u8; i <= screen->ParaMax_u8; i++) {
        sLCDObject[i].Mode_u8 = 0xF0;
    }
    
    if (screen->Para_u8 <= screen->ParaMax_u8) {
        sLCDObject[screen->Para_u8].Mode_u8 = screen->Flag_u8;
        if (pData != NULL) {
            sLCDObject[screen->Para_u8].pData = pData;
        }  
    }
}

/*
    Func: set screen next
        + index:
        + param: curr, min, max
        + pdata: option
        + flag: toggle: name | pdata | index of pdata
*/

void Display_Set_Screen (sScreenInformation *screen, uint8_t index, uint8_t subindex,
                         uint8_t para, uint8_t paramin, uint8_t paramax,
                         void *pData, uint8_t flag)
{
    screen->Index_u8 = index;
    screen->SubIndex_u8 = subindex;
    screen->Para_u8 = para;
    screen->ParaMin_u8 = paramin;
    screen->ParaMax_u8 = paramax;

    Display_Set_Screen_Flag(screen, pData, flag);
}

/*
    Func: check pass to setting
*/

uint8_t Display_Check_Password (uint8_t pPass[])
{
    for (uint8_t i = 0; i < sizeof(aPASSWORD); i++) {
        if (pPass[i] != aPASSWORD[i])
            return false;
    }
    
    return true;
}


/*
    Func: sub process up button: control config pressure
*/
extern char aTEST[20];
void Display_Process_Up_Pressure_2 (sPressureLinearInter *pPress)
{
    if (sLCD.rSett_u8 == false) {
        if (sLCD.sScreenNow.Para_u8 > sLCD.sScreenNow.ParaMin_u8) {
            sLCD.sScreenNow.Para_u8--;
        }
        
        Display_Set_Screen_Flag(&sLCD.sScreenNow, NULL, 0xF3);
        return;
    }
    
    switch (sLCD.sScreenNow.Para_u8)
    {
        case __SET_PRESSURE_2_0:
            pPress->Type_u8++;
            if (pPress->Type_u8 > 1)
                pPress->Type_u8 = 0;
            sLCDObject[sLCD.sScreenNow.Para_u8].pData = AnalogType[pPress->Type_u8];  
            break;
        case __SET_PRESSURE_2_1:
        case __SET_PRESSURE_2_2:
        case __SET_PRESSURE_2_3:
        case __SET_PRESSURE_2_5:
        case __SET_PRESSURE_2_6:
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
        case __SET_PRESSURE_2_4:
            if (pPress->InUnit_u8 < _UNIT_BAR) {
                pPress->InUnit_u8++;
                sLCDObject[sLCD.sScreenNow.Para_u8].pData = aUnitWm[pPress->InUnit_u8];
            } else {
                pPress->InUnit_u8 = _UNIT_MILIVOL;
                sLCDObject[sLCD.sScreenNow.Para_u8].pData = aUnitWm[pPress->InUnit_u8];
            }
            break;
        case __SET_PRESSURE_2_7:
            if (pPress->OutUnit_u8 < _UNIT_BAR) {
                pPress->OutUnit_u8++;
                sLCDObject[sLCD.sScreenNow.Para_u8].pData = aUnitWm[pPress->OutUnit_u8];
            } else {
                pPress->OutUnit_u8 = _UNIT_MILIVOL;
                sLCDObject[sLCD.sScreenNow.Para_u8].pData = aUnitWm[pPress->OutUnit_u8];
            }
            break;
    }  
}

/*
    Func: sub process down button: control config pressure
*/

void Display_Process_Down_Pressure_2 (sPressureLinearInter *pPress)
{
    if (sLCD.rSett_u8 == false) {
        if (sLCD.sScreenNow.Para_u8 <= sLCD.sScreenNow.ParaMax_u8) {
            sLCD.sScreenNow.Para_u8++;
            Display_Set_Screen_Flag(&sLCD.sScreenNow, NULL, 0xF3);
        } else {
            Display_Set_Screen_Flag(&sLCD.sScreenNow, NULL, 0xF0);
        }

        return;
    }
    
    switch (sLCD.sScreenNow.Para_u8)
    {
        case __SET_PRESSURE_2_0:
            if (pPress->Type_u8 > 1) {
                pPress->Type_u8--;
            } else {
                pPress->Type_u8 = 1;
            }
            sLCDObject[sLCD.sScreenNow.Para_u8].pData = AnalogType[pPress->Type_u8];  
            break;
        case __SET_PRESSURE_2_1:
        case __SET_PRESSURE_2_2:
        case __SET_PRESSURE_2_3:
        case __SET_PRESSURE_2_5:
        case __SET_PRESSURE_2_6:
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
        case __SET_PRESSURE_2_4:
            if (pPress->InUnit_u8 > _UNIT_MILIVOL) {
                pPress->InUnit_u8--;
            } else {
                pPress->InUnit_u8 = _UNIT_BAR;
            }
            sLCDObject[sLCD.sScreenNow.Para_u8].pData = aUnitWm[pPress->InUnit_u8];
            break;
        case __SET_PRESSURE_2_7:
            if (pPress->OutUnit_u8 > _UNIT_MILIVOL) {
                pPress->OutUnit_u8--;
            } else {
                pPress->OutUnit_u8 = _UNIT_BAR; 
            }
            sLCDObject[sLCD.sScreenNow.Para_u8].pData = aUnitWm[pPress->OutUnit_u8];
            break;
    }  
}

/*
    Func: show static param
        + stime
        + icon: internet, baterry, "____"
*/
void Display_Show_Static_Param (void)
{
    static uint8_t cCharge = 0, batlevel = 0;
    uint8_t TempPos = 0;
    char aData[32] = {0};
    static uint32_t LandMarkNextScreen = 0;
    
    //Hien thi sac pin
    if (Display_Show_Charge_Bat(&batlevel) == 1) {
        if (Check_Time_Out(LandMarkNextScreen, 500) == true) {
            LandMarkNextScreen = RtCountSystick_u32;
            cCharge++;
        } 
        
        if(cCharge > 3) {
            cCharge = batlevel;
        }
    } else {
        LandMarkNextScreen = RtCountSystick_u32;
        cCharge = batlevel;
    }
        
    glcd_tiny_draw_char(120, 0, PIN_ZERO + cCharge);
        
    //Hien thi cot song sim
    if (sSimCommVar.State_u8 == _SIM_CONN_MQTT) {
        glcd_tiny_draw_char(114, 0, CONNECT_DISPLAY);
        TempPos = 6;
    } else {
        glcd_tiny_draw_char(114, 0, 0x20U);    
    }
    
    //hien thi cot ethernet
    if (sAppEthVar.Status_u8 == _ETH_MQTT_CONNECTED) {
        glcd_tiny_draw_char(114 - TempPos, 0, FONT_ETHERNET);    
    } else {
        glcd_tiny_draw_char(114 - TempPos, 0, 0x20U);    
    }
    
    //Hien thi stime
    sprintf(aData, "%02d:%02d:%02d", sRTC.hour, sRTC.min, sRTC.sec);
    glcd_tiny_draw_string(48, 0, (char *) aData);
    
    //Hien thi gạch duoi
    glcd_draw_line(0, 8, 127, 8, BLACK); 
}

/*
    Func: show static param
        + stime
        + icon: internet, baterry, "____"
*/
void Display_Show_State_Sensor_Network(uint8_t screen)
{
    for (uint8_t i = 0; i < (sizeof(SensorTable) / sizeof((SensorTable)[0])); i++)
    {
        // TEMP khong can Enable
        if (SensorTable[i].Enable != NULL)
        {
            if (*(SensorTable[i].Enable) == 0) 
                continue;
        }

        uint8_t id = SensorTable[i].E_Kind;

        if (sLCDObject[id].Screen_u8 != screen)
            continue;

        if (*(SensorTable[i].StateConnet) == 0)
            glcd_tiny_draw_string(120, sLCDObject[id].Row_u8, " ");
        else
        {
            if (*(SensorTable[i].StateSensor) == _E_DANG_DO)
                glcd_tiny_draw_string(120, sLCDObject[id].Row_u8, "N");
            else if (*(SensorTable[i].StateSensor) == _E_HIEU_CHUAN)
                glcd_tiny_draw_string(120, sLCDObject[id].Row_u8, "C");
            else
                glcd_tiny_draw_string(120, sLCDObject[id].Row_u8, "E");
        }
    }
}

/*
    Func: show static param
        + stime
        + icon: internet, baterry, "____"
*/
void Display_Show_State_Calib_Sensor (uint8_t screen)
{
//    if(screen == _LCD_SCR_CHECK_SETTING)
//    {
//        switch(sHandleRs485.State_Wait_Calib)
//        {
//            case _STATE_CALIB_FREE:
//              sLCDObject[__CHECK_STATE_SETTING].pData = aSTT_SETTING_FREE;
//              break;
//              
//            case _STATE_CALIB_ENTER:
//              sLCDObject[__CHECK_STATE_SETTING].pData = aSTT_SETTING_ENTER;
//              break;
//
//            case _STATE_CALIB_WAIT:
//              sLCDObject[__CHECK_STATE_SETTING].pData = aSTT_SETTING_WAIT;
//              break;
//              
//            case _STATE_CALIB_DONE:
//              sLCDObject[__CHECK_STATE_SETTING].pData = aSTT_SETTING_DONE;
//              break;
//              
//            case _STATE_CALIB_ERROR:
//              sLCDObject[__CHECK_STATE_SETTING].pData = aSTT_SETTING_ERROR;
//              break;
//              
//            default:
//              break;
//        }
//    }
//    else
//    {
//        sLCDObject[__CHECK_STATE_SETTING].pData = aSTT_SETTING_FREE;
//    }
}

void Display_Update_ScrSensor(uint8_t screen)
{
//    uint8_t row  = 2;
//    uint8_t page = _LCD_SCREEN_CM44;
//
//    uint8_t total = 1; 
//
//    for(int i = 0; i < (sizeof(SensorTable) / sizeof((SensorTable)[0])); i++)
//        if(SensorTable[i].Enable && *SensorTable[i].Enable)
//            total++;
//
//    sLCDPageSensor.Number = total;
//    sLCDPageSensor.Page   = (total + 5) / 6;
//
//    for(int i = 0; i < (sizeof(SensorTable) / sizeof((SensorTable)[0])); i++)
//    {
//        const SENSOR_CFG *cfg = &SensorTable[i];
//
//        if(cfg->Enable && *cfg->Enable == 0)
//            continue;
//
//        sLCDObject[cfg->E_Kind].Row_u8    = row;
//        sLCDObject[cfg->E_Kind].Screen_u8 = page;
//
//        if(++row == 8) { row = 2; page++; }
//    }
    uint8_t row  = 2;
    uint8_t page = _LCD_SCREEN_CM44;

    uint8_t total = 0;

    uint8_t numSensor = sizeof(SensorTable) / sizeof(SensorTable[0]);

    // ===== Dem sensor hien thi =====
    for (uint8_t i = 0; i < numSensor; i++)
    {
        uint8_t isEnable = 1; 

        if (SensorTable[i].Enable != NULL)
        {
            isEnable = *(SensorTable[i].Enable);
        }

        if (isEnable)
        {
            total++;
        }
    }

    sLCDPageSensor.Number = total;
    sLCDPageSensor.Page   = (total + 5) / 6;

    // ===== gan row va page =====
    for (uint8_t i = 0; i < numSensor; i++)
    {
        const SENSOR_CFG *cfg = &SensorTable[i];

        uint8_t isEnable = 1;

        if (cfg->Enable != NULL)
        {
            isEnable = *(cfg->Enable);
        }

        if (!isEnable)
        {
            continue;
        }

        sLCDObject[cfg->E_Kind].Row_u8    = row;
        sLCDObject[cfg->E_Kind].Screen_u8 = page;

        row++;

        if (row >= 8)
        {
            row = 2;
            page++;
        }
    }
}

void Update_ParaDisplay(void)
{
//    sParaDisplay.pH_Offset_i32 = (int32_t)(s485Measure[_SS_PH].Offset_f * 100);
//    sParaDisplay.Clo_Offset_i32 = (int32_t)(s485Measure[_SS_CLO].Offset_f * 100);
//    sParaDisplay.Turb_Offset_i32 = (int32_t)(s485Measure[_SS_TURB].Offset_f * 100);
//    sParaDisplay.EC_Offset_i32 = (int32_t)(s485Measure[_SS_EC].Offset_f * 100);
//    sParaDisplay.Salinity_Offset_i32 = (int32_t)(s485Measure[_SS_SALT].Offset_f * 100);
//    sParaDisplay.Temp_Offset_i32 = (int32_t)(sOffsetMeasure.Temp_f * 100);
    
    sLCDObject[__SC1_CLO_DU].Scale_u8   = sDataSensorMeasure.sClo.Scale_u8;   
    sLCDObject[__SC1_PH_WATER].Scale_u8 = sDataSensorMeasure.spH.Scale_u8;    
    sLCDObject[__SC1_TURB].Scale_u8      = sDataSensorMeasure.sTurb.Scale_u8;    
    sLCDObject[__SC1_SALINITY].Scale_u8 = sDataSensorMeasure.sSal.Scale_u8;  
    sLCDObject[__SC1_TEMP].Scale_u8     = sDataSensorMeasure.sTemp.Scale_u8;    
    sLCDObject[__SC1_EC].Scale_u8       = sDataSensorMeasure.sEC.Scale_u8; 
    sLCDObject[__SC1_COD].Scale_u8     = sDataSensorMeasure.sCOD.Scale_u8;  
    sLCDObject[__SC1_TSS].Scale_u8     = sDataSensorMeasure.sTSS.Scale_u8;  
    sLCDObject[__SC1_NH4].Scale_u8     = sDataSensorMeasure.sNH4.Scale_u8;  
    sLCDObject[__SC1_DO].Scale_u8      = sDataSensorMeasure.sDO.Scale_u8;  
    sLCDObject[__SC1_TDS].Scale_u8      = sDataSensorMeasure.sTDS.Scale_u8;
    sLCDObject[__SC1_NO3].Scale_u8      = sDataSensorMeasure.sNO3.Scale_u8;
}

/*
    Func: caculator level Vbat : 
        + co 4 muc: < 2.9v : 0
                    < 3.5v : 1
                    < 4.1  : 2
                    > 4.1  : 3
        + return: 1: co nguon ngoai va dang sac: chay hieu ung sac
                  0: mat nguon ngoai: hien thi cot pin
*/



/*
    Func: caculator level Vbat : 
        + co 4 muc: < 2.9v : 0
                    < 3.5v : 1
                    < 4.1  : 2
                    > 4.1  : 3
        + return: 1: co nguon ngoai va dang sac: chay hieu ung sac
                  0: mat nguon ngoai: hien thi cot pin
*/

uint8_t Display_Show_Charge_Bat (uint8_t *level)
{
    if (sBattery.mVol_u32 < 2900) {
        *level = 0;
    } else if (sBattery.mVol_u32 < 3500) {
        *level = 1;
    } else if (sBattery.mVol_u32 < 4100) {
        *level = 2;
    } else {
        *level = 3;
    }
   
    //neu co nguon ngoai return 1
    if (sVout.mVol_u32 > 5000) {
        if (*level < 3) {
            return 1;
        }
    }
    
    return 0;
}

/*
    Get data vào 1 con tro: biet kieu du lieu o do
*/
void Display_Set_Data (void *pdata, uint64_t val, uint8_t type)
{    
    switch (type) 
    {
        case _DTYPE_U8:
            *((uint8_t *) pdata) = val;
            break;
        case _DTYPE_I8:
            *((int8_t *) pdata) = val;
            break;
        case _DTYPE_U16:
            *((uint16_t *) pdata) = val;
            break;
        case _DTYPE_I16:
            *((int16_t *) pdata) = val;
            break;
        case _DTYPE_U32:
            *((uint32_t *) pdata) = val;
            break;
        case _DTYPE_I32:
            *((int32_t *) pdata) = val;
            break;
        case _DTYPE_U64:
            *((uint64_t * )pdata) = val;
            break;
        case _DTYPE_I64:
            *((int64_t *) pdata) = val;
            break;
        case _DTYPE_FLOAT:
            *((float * )pdata) = val;
            break;
        case _DTYPE_DOUBLE:
            *((double * )pdata) = val;
            break;
    }
}









