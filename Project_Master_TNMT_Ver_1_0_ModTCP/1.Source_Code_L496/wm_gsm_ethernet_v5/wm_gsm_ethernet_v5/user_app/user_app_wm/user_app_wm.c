

#include "user_app_wm.h"

#include "user_string.h"
#include "string.h"
#include "user_json.h"

#include "user_adc.h"
#include "user_timer.h"
#include "user_time.h"

#include "user_internal_mem.h"
#include "user_uart.h"

#include "user_app_comm.h"
#include "user_modbus_rtu.h"

#include "user_sd.h"
#include "sd_functions.h"
#include "stdio.h"
#include "sd_benchmark.h"


/*================ Define =================*/
static uint8_t     _Cb_Entry_Wm (uint8_t event);
static uint8_t     _Cb_Log_TSVH (uint8_t event);
static uint8_t     _Cb_ext_irq (uint8_t event);
static uint8_t     _Cb_Dir_Change (uint8_t event);

static uint8_t     _Cb_Check_Mode (uint8_t event);
static uint8_t     _Cb_Meas_Pressure (uint8_t event);
static uint8_t     _Cb_Scan_Alarm (uint8_t event);
static uint8_t     _Cb_Control_Led1 (uint8_t event);
static uint8_t     _Cb_Read_RS485_Modbus(uint8_t event);
static uint8_t     _Cb_Read_RS485_2_Modbus(uint8_t event); 

static uint8_t     _Cb_Power_Up_12V(uint8_t event);

static uint8_t     _Cb_Get_Contact_Input(uint8_t event); 

static uint8_t     _Cb_Rs485_1_Recv (uint8_t event);
static uint8_t     _Cb_Rs485_2_Recv (uint8_t event);


/*================ Struct =================*/
sEvent_struct sEventAppWM [] =
{
    { _EVENT_ENTRY_WM, 		    0, 0, 500, 	    _Cb_Entry_Wm }, 
        
    { _EVENT_LOG_TSVH, 		    0, 0, 500, 	    _Cb_Log_TSVH },
    { _EVENT_EXT_IRQ,           0, 0, 100, 	    _Cb_ext_irq },               //Ngat de 100ms chong nhieu
    { _EVENT_DIR_CHANGE,		0, 0, 0, 	    _Cb_Dir_Change },            //Chong nhieu 100ms -> Ton PIN    
    
    { _EVENT_CHECK_MODE,		1, 0, 1000,     _Cb_Check_Mode },           
    { _EVENT_MEAS_PRESSURE,		0, 0, 3000,     _Cb_Meas_Pressure }, 
    { _EVENT_SCAN_ALARM,	    1, 0, 50,       _Cb_Scan_Alarm }, 
    { _EVENT_CONTROL_LED1,		1, 0, 200,      _Cb_Control_Led1 }, 
    
    { _EVENT_RS485_MODBUS,		0, 0, 500,      _Cb_Read_RS485_Modbus }, 
    { _EVENT_RS485_2_MODBUS,    0, 0, 500,      _Cb_Read_RS485_2_Modbus }, 
    
    { _EVENT_POWER_UP_12V,      0, 0, 100,      _Cb_Power_Up_12V },
    
    { _EVENT_GET_CONTACT_IN,    0, 0, 1000,     _Cb_Get_Contact_Input },
    
    { _EVENT_RS485_1_RECV,		0, 0, 20,       _Cb_Rs485_1_Recv   }, 
    { _EVENT_RS485_2_RECV,		0, 0, 20,       _Cb_Rs485_2_Recv }, 
};
            
    
Struct_Battery_Status    sBattery;
Struct_Battery_Status    sVout;
Struct_Battery_Status    sPressure;
Struct_Pulse			 sPulse[MAX_CHANNEL];

sThresholdVariable sMeterThreshold[MAX_CHANNEL] = 
{ 
    [0] = {
            .Quan = 
                {true, 0, 1439, 0, 65535},
                {true, 0, 1439, 0, 65535},
                
            .Flow = 
                {true, 0, 1439, 0, 65535},
                {true, 0, 1439, 0, 65535},
                
            .LowBatery = 10,
          },
};

sLastRecordInfor    sLastPackInfor = { {0}, 0};

static uint8_t aMARK_ALARM[10];

sAppWmVariable sWmVar = 
{
    .nChannel_u8 = MAX_CHANNEL,
    .rPressCalib_u8 = false,
};


static UTIL_TIMER_Object_t TimerLevel;
static void OnTimerLevelPowerOn(void *context);

static UTIL_TIMER_Object_t TimerRs485_2;
static void OnTimerRs485_2(void *context);


static GPIO_TypeDef*  LED_PORT[1] = {LED_GPIO_Port};
static const uint16_t LED_PIN[1] = {LED_Pin};

static UTIL_TIMER_Object_t Timer12VPowerUp;
static void OnTimer12VPowerUp(void *context);

static UTIL_TIMER_Object_t TimerPress;
static void OnTimerPressure (void *context);

char aUnitWm[5][10] =  
{
    {"mV"},
    {"mA"},
    {"V"},
    {"met"},
    {"bar"},
};

char AnalogType[2][10] = { "pressure", "level"};

static uint8_t iMarkTSVH = 0;

char AppWm_TN_PARAM[2][10] = 
{ 
    "LUU_LUONG", 
    "MUC_NUOC"
};


/*================ Struct =================*/


void AppWm_Init (void)
{
    //Init xung trong flash
    AppWm_Init_Pulse_Infor();
    AppWm_Button_Default_Device();    
    AppWm_Init_Default_Pressure();
    AppWm_Init_Thresh_Measure();
    AppWm_Init_WM_Dig_Infor();
    AppWm_Init_TNMT_Infor();
    
    //Set timer power on level power
    UTIL_TIMER_Create(&TimerLevel, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, OnTimerLevelPowerOn, NULL);
    UTIL_TIMER_SetPeriod(&TimerLevel, 10000);
    
    //Set timer power on level power
    UTIL_TIMER_Create(&TimerRs485_2, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, OnTimerRs485_2, NULL);
    
    //Set timer power on level power
    UTIL_TIMER_Create(&Timer12VPowerUp, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, OnTimer12VPowerUp, NULL);
    UTIL_TIMER_SetPeriod(&Timer12VPowerUp, 10000);
            
#ifdef USING_AT_CONFIG
    sATCmdList[_SET_DEFAULT_PULSE].CallBack = AppWm_SER_Set_Default_Pulse;
    
    sATCmdList[_SET_FLOW_THRESH].CallBack = AppWm_SER_Set_Thresh_Flow;
    sATCmdList[_QUERY_FLOW_THRESH].CallBack = AppWm_SER_Get_Thresh_Flow;
    
    sATCmdList[_SET_QUAN_THRESH].CallBack = AppWm_SER_Set_Thresh_Quan;
    sATCmdList[_QUERY_QUAN_THRESH].CallBack = AppWm_SER_Get_Thresh_Quan;
    
    sATCmdList[_SET_BAT_THRESH].CallBack = AppWm_SER_Set_Thresh_Bat;
    sATCmdList[_QUERY_BAT_THRESH].CallBack = AppWm_SER_Get_Thresh_Bat;
    
    sATCmdList[_SET_PULSE_VALUE].CallBack = AppWm_SER_Set_Pulse;
    sATCmdList[_QUERY_PULSE_VALUE].CallBack = AppWm_SER_Get_Pulse;
    
    sATCmdList[_SET_PULSE_FACTOR].CallBack = AppWm_SER_Set_Pulse_Factor;
    sATCmdList[_QUERY_PULSE_FACTOR].CallBack = AppWm_SER_Get_Pulse_Factor;
    
    sATCmdList[_SET_PULSE_START].CallBack = AppWm_SER_Set_Pulse_Start;
    sATCmdList[_QUERY_PULSE_START].CallBack = AppWm_SER_Get_Pulse_Start;
    
    sATCmdList[_SET_LEVEL_THRESH].CallBack = AppWm_SER_Set_Level_Thresh;
    sATCmdList[_QUERY_LEVEL_THRESH].CallBack = AppWm_SER_Get_Level_Thresh;
    
    sATCmdList[_SET_NUM_CHANNEL].CallBack = AppWm_SER_Set_Num_Channel;
    sATCmdList[_QUERY_NUM_CHANNEL].CallBack = AppWm_SER_Get_Num_Channel;
    
    sATCmdList[_SET_MODBUS_INFOR].CallBack = AppWm_SER_Set_Modbus_Infor;
    sATCmdList[_QUERY_MODBUS_INFOR].CallBack = AppWm_SER_Get_Modbus_Infor;
    
    sATCmdList[_SET_PRESS_INFOR].CallBack = AppWm_SER_Set_Pressure_Infor;
    sATCmdList[_QUERY_PRESS_INFOR].CallBack = AppWm_SER_Get_Pressure_Infor;
    
    sATCmdList[_SET_LEVEL_INFOR].CallBack = AppWm_SER_Set_Level_Infor;
    sATCmdList[_QUERY_LEVEL_INFOR].CallBack = AppWm_SER_Get_Level_Infor;

    sATCmdList[_QUERY_PRESS_VAL].CallBack = AppWm_SER_Get_Pressure_Val; 
    sATCmdList[_QUERY_PRESS_CALIB].CallBack = AppWm_SER_Set_Pressure_Calib;  
    sATCmdList[_QUERY_PRESS_CAL_VAL].CallBack = AppWm_SER_Get_Pressure_Calib; 
    
    sATCmdList[_SET_TNMT_INFOR].CallBack = AppWm_SER_Set_TNMT_Infor;
    sATCmdList[_QUERY_TNMT_INFOR].CallBack = AppWm_SER_Get_TNMT_Infor;
    
    sATCmdList[_SET_TNMT_PACK_M].CallBack = AppWm_SER_Set_TNMT_Pack_M;
    sATCmdList[_QUERY_TNMT_PACK_M].CallBack = AppWm_SER_Get_TNMT_Pack_M;
     
      
#endif
    
//    UTIL_TIMER_Create(&TimerAlarm, 0xFFFFFFFFU, UTIL_TIMER_PERIODIC, OnTimerAlarm, NULL);
//    UTIL_TIMER_SetPeriod(&TimerAlarm, sModemInfor.sFrequence.DurAlarm_u32 * 1000);
//    UTIL_TIMER_Start(&TimerAlarm);
//    
    UTIL_TIMER_Create(&TimerPress, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, OnTimerPressure, NULL);
    UTIL_TIMER_SetPeriod(&TimerPress, 1000);
    
    RS485_Init_Uart();
    RS485_Init_RX_Mode();
    
    pRS485_Rx_Done = AppWm_RS485_Rx_Done;
    pRS485_2_Rx_Done = AppWm_RS485_2_Rx_Done;
}


uint8_t AppWm_Task(void)
{
	uint8_t i = 0;
	uint8_t Result = 0;

	for (i = 0; i < _EVENT_END_WM; i++)
	{
		if (sEventAppWM[i].e_status == 1)
		{
            if ( (i != _EVENT_CHECK_MODE) 
                  && (i != _EVENT_SCAN_ALARM) 
                  && (i != _EVENT_CONTROL_LED1)
                  && (i != _EVENT_GET_CONTACT_IN) )
                Result = 1;

			if ((sEventAppWM[i].e_systick == 0) ||
					((HAL_GetTick() - sEventAppWM[i].e_systick)  >=  sEventAppWM[i].e_period))
			{
                sEventAppWM[i].e_status = 0;  //Disable event
				sEventAppWM[i].e_systick = HAL_GetTick();
				sEventAppWM[i].e_function_handler(i);
			}
		}
	}
    
	return Result;
}



static uint8_t _Cb_Entry_Wm (uint8_t event)
{
    fevent_active(sEventAppWM, _EVENT_MEAS_PRESSURE); 
    fevent_active(sEventAppWM, _EVENT_RS485_MODBUS);
    fevent_active(sEventAppWM, _EVENT_RS485_2_MODBUS);
    fevent_active(sEventAppWM, _EVENT_GET_CONTACT_IN); 
    
    iMarkTSVH = 0x01;
    
	return 1;
}


static uint8_t _Cb_Log_TSVH (uint8_t event)
{
	//Log Data meter to flash
    UTIL_Printf_Str(DBLEVEL_M, "u_app_wm:log tsvh data\r\n");
    //
    Get_RTC();

	AppWm_Get_VBat();
    AppWm_Get_VOut();
    
    APP_LOG(TS_OFF, DBLEVEL_M, "u_app_wm: Vpin in: %d mV \r\n", sBattery.mVol_u32);
    APP_LOG(TS_OFF, DBLEVEL_M, "u_app_wm: Vpin out: %d mV \r\n", sVout.mVol_u32);
    
    AppWm_Save_Pulse();
#ifdef ENABLE_TEST_HARDWARE
    APP_LOG(TS_OFF, DBLEVEL_M, "u_app_wm: pulse: %d, %d, %d, %d \r\n", 
                                    sPulse.Number_i64, sPulse[1].Number_i64, sPulse[2].Number_i64, sPulse[3].Number_i64 );
#endif
    
//    if (sWmVar.rIntan_u8 == true) {
//        sWmVar.rIntan_u8 = false;
//    #ifdef USING_APP_SIM
//        sMessage.aMESS_PENDING[DATA_INTAN] = TRUE;
//    #endif
//    } else {
//        //Packet TSVH and push to mem
//        AppWm_Packet_TSVH();  
//    }
//    
//    AppWm_Packet_Modbus();
//    AppWm_Packet_TNMT();
    
    

	return 1;
}


static uint8_t _Cb_ext_irq(uint8_t event)
{
    static uint32_t Landmark = 0;
    uint32_t CurrentTime = HW_RTC_GetCalendarValue_Second(sRTC, 0);
    
    if (Cal_Time(Landmark, CurrentTime) >= 5) {
        Landmark = CurrentTime;
        
        UTIL_Printf_Str(DBLEVEL_M, "u_app_wm: irq send intan\r\n");

        sWmVar.rIntan_u8 = true;
        //Active event Measure Press
        fevent_active(sEventAppWM, _EVENT_ENTRY_WM);   // chan do pin can delay: neu co ap suat thi active luon
        LED_On (_LED_STATUS);
        sEventAppWM[_EVENT_CONTROL_LED1].e_period = 1000;
        fevent_enable(sEventAppWM, _EVENT_CONTROL_LED1); //led sang
        // uu tien khoi dong ethernet truoc
    #ifdef USING_APP_ETHERNET    
        AppEth_Restart_If_PSM();
    #endif
    
    #ifdef USING_APP_SIM
        AppSim_Restart_If_PSM(); 
        
        if (sWmVar.pRq_Instant != NULL)
            sWmVar.pRq_Instant();
    #endif
    }
    
    return 1;
}


static uint8_t _Cb_Dir_Change(uint8_t event)
{
    char aData[48] = {0}; 
    
    LPTIM_Counter_Get_Pulse(&hlptim1, &sPulse[0]);
    LPTIM_Counter_Get_Pulse(&hlptim2, &sPulse[1]);
    
    sprintf(aData, "u_app_wm: dir change: %d, %d\r\n", sPulse[0].Direct_u8, sPulse[1].Direct_u8);

    if (sWmVar.pPack_Alarm_Str != NULL)
        sWmVar.pPack_Alarm_Str(aData);
    
    //Neu nhu bi nhieu vao day nhieu qua thi nen bo qua?
    aMARK_ALARM[_ALARM_DIR_CHANGE] = TRUE;
    //Dong goi ban tin Event
    AppWm_Log_Data_Event(0);
   
    return 1;
}


static uint8_t _Cb_Check_Mode(uint8_t event)
{
//    //Check PIN VBAT_Detect to decide _MODE_ONLINE or _MODE_RUNNING
//#ifdef USING_APP_SIM
//    if ((UTIL_var.ModeConnNow_u8 == _CONNECT_DATA_MAIN) 
//        || (UTIL_var.ModeConnNow_u8 == _CONNECT_DATA_BACKUP)
//        || (UTIL_var.ModeConnNow_u8 == _CONNECT_FTP_UPLOAD) )
//    {
//        if (sAppSimVar.StepPowerOn_u8 == 0)
//            AppWm_Check_Mode_Power();    
//    }
//#endif
//
//    fevent_enable(sEventAppWM, event);

    return 1;
}


static uint8_t _Cb_Meas_Pressure(uint8_t event)
{       
    uint8_t Status = AppWm_Meas_Pressure_Process(&sEventAppWM[event].e_period);
               
    if (Status == true)
    {       
        if (iMarkTSVH & 0x01) {
            for (uint8_t i = 0; i < MAX_CHANNEL; i++)
            {
              
            #ifdef PRESSURE_DECODE
                APP_LOG(TS_OFF, DBLEVEL_M, "u_app_wm: pressure chan.%d: %d mbar \r\n", i, sWmVar.aPRESSURE[i].Val_i32);
            #else
                APP_LOG(TS_OFF, DBLEVEL_M, "u_app_wm: pressure chan.%d: %d mV \r\n", i, sWmVar.aPRESSURE[i].Val_i32);
            #endif
            }    
        }
             
        iMarkTSVH |= 0x02;
        
        if ( (UTIL_var.ModePower_u8 == _POWER_MODE_SAVE) && (iMarkTSVH == 0x0F) ){
            V_PIN_OFF;  
        } else {
            sEventAppWM[event].e_period = PERIOD_READ_MODBUS;
            fevent_enable(sEventAppWM, event);
        }
        
        if (iMarkTSVH == 0x0F) {
            iMarkTSVH = 0;
            fevent_active(sEventAppWM, _EVENT_LOG_TSVH);
        }

    #ifdef USING_APP_SIM
        if (sWmVar.pCtrl_Sim != NULL)
            sWmVar.pCtrl_Sim(false);
    #endif

    } else
    {     
    #ifdef USING_APP_SIM
        if (sWmVar.pCtrl_Sim != NULL)
            sWmVar.pCtrl_Sim(true);
    #endif
        
        fevent_enable(sEventAppWM, event);
    }

    return 1;
}




static uint8_t _Cb_Scan_Alarm(uint8_t event)
{   
    static uint16_t cCheck = 0;

    if ( cCheck++ >= (TIME_CHECK_ALARM / sEventAppWM[event].e_period) )
    {
        cCheck = 0;
        
        if (sRTC.year > 20) {
            Get_RTC();
        
            AppWm_Get_VBat();
            AppWm_Get_VOut();
            AppWm_Get_Pulse();
            
            AppWm_Cacul_Param();
            if (AppWm_Scan_Alarm() == true) {
            #ifdef USING_APP_SIM
                AppSim_Restart_If_PSM();
            #endif
            }
        }
    }
            
    fevent_enable(sEventAppWM, event);
        
	return 1;
}



void AppWm_Cacul_Param (void)
{
    for (uint8_t i = 0; i < MAX_CHANNEL; i++) {
        if (UTIL_var.ModePower_u8 == _POWER_MODE_ONLINE) {
            sWmVar.iCaculOK[i] = AppWm_Cacul_Flow_1(i);
        } else {
            sWmVar.iCaculOK[i] = AppWm_Cacul_Flow_2(i);
        }
        
    }
}

uint8_t AppWm_Cacul_Flow_2 (uint8_t chann)
{
    static uint8_t mCacal[MAX_CHANNEL] = {false};
    static uint32_t LastTime[MAX_CHANNEL] = {0};
    static int64_t LastNum[MAX_CHANNEL] = {0};
    
    uint32_t CurrTime = HW_RTC_GetCalendarValue_Second(sRTC, 0);
    uint32_t DelTime = 0;
    uint8_t result = false;
        
    if ( (mCacal[chann] == false ) || (sRTC.year == 0) ) {
        LastNum[chann] = sPulse[chann].Number_i64;
        LastTime[chann] = CurrTime;
        mCacal[chann] = true;
    } else {
        sPulse[chann].Quantity_f = (sPulse[chann].Number_i64 - LastNum[chann]);
        sPulse[chann].Quantity_f = sPulse[chann].Quantity_f * sPulse[chann].FactorInt_i16 \
                                                * Convert_Scale(sPulse[chann].FactorDec_u8);
        
        DelTime = CurrTime - LastTime[chann];
        sPulse[chann].Flow_f = (sPulse[chann].Quantity_f * 3600) / DelTime;    //Pulse per hour
        
        LastNum[chann] = sPulse[chann].Number_i64;
        LastTime[chann] = CurrTime;
        result = true;
    }
     
    return result;
}

/*
    Func: tinh luu luong theo xung

        + Tinh cách 1: theo 1 chu kì (5p) tính tổng bao nhieu xung: sau do chia cho 5 phut
            -- neu nhu dong ho chay cham (1 - 2) xung như vậy sẽ làm cho luu luong bị gấp đôi
        
        + Tinh cách 2: theo khoảng thời gian giữa các xung (tính gối đầu 25 xung)
            -- khi xung không tăng nữa: lưu lượng giảm dần rất lâu về 0.
                tương tụ khi tăng dần cũng rất lâu để lên được chính xác.

            -- neu như them dieu kien: khoang thoi gian giua 2 xung ma > chu ki gui:
                    san luong gui len = 0, nen minh se gui len luu luong la 0

            trien khai
                + lay count[0] lam diem so sanh bat dau

*/
uint8_t AppWm_Cacul_Flow_1 (uint8_t chann)
{
    static sFlowCacul sFlowVar[MAX_CHANNEL] = {0};
    static uint8_t count[MAX_CHANNEL] = {0};
    double TempDouble = 0;
    static uint8_t LastYear = 0;
    
    int32_t duratime = 0; 
    uint32_t CurrTime = 0;
    uint8_t rCacul = false, mNewPulse = false, i = 0;
    int32_t pulse = 0;
    uint32_t FreqSend = 0;
    uint32_t SecondToTick = 1 << RTC_N_PREDIV_S;
    
    if (chann >= MAX_CHANNEL) {
        return false;   
    }
    
    if (UTIL_var.ModePower_u8 == _POWER_MODE_ONLINE) {
        FreqSend = sModemInfor.sFrequence.DurOnline_u32 * 60 * SecondToTick;
    } else {
        FreqSend = sModemInfor.sFrequence.Duration_u32 * 60 * SecondToTick;
    }
    
    //neu lay lai thoi gian thuc. tinh lai tu dau
    if ((sRTC.year - LastYear) >= 2) {
        count[chann] = 0;
    }
    LastYear = sRTC.year;
    
    CurrTime = HW_RTC_GetCalendarValue_ms(sRTC);
    
    if (count[chann] == 0) {  
        sFlowVar[chann].LastTime_u32 = CurrTime;
        sFlowVar[chann].sSamplePulse[count[chann]] = sPulse[chann].Number_i64;
        UTIL_MEM_cpy(&sFlowVar[chann].sSampleTime[count[chann]], &sRTC, sizeof(ST_TIME_FORMAT));
        
        count[chann]++;
        return false;
    }
    
    if (sPulse[chann].Number_i64 != sFlowVar[chann].sSamplePulse[count[chann] - 1]) {
        sFlowVar[chann].sSamplePulse[count[chann]] = sPulse[chann].Number_i64;
        UTIL_MEM_cpy(&sFlowVar[chann].sSampleTime[count[chann]], &sRTC, sizeof(ST_TIME_FORMAT));
        
        sFlowVar[chann].DelTime_u32 = CurrTime - sFlowVar[chann].LastTime_u32;
        sFlowVar[chann].LastTime_u32 = CurrTime;
        
        if (count[chann] >= 2) {
            rCacul = true;
        }
        sFlowVar[chann].PlusTime_u32 = 0;
        mNewPulse = true;
        
        count[chann]++;  
    } else {
        //khoang cach giua 2 xung > 1h. coi nhu lay lai moc thoi gian
        if ( CurrTime > (sFlowVar[chann].LastTime_u32 + 3600 * SecondToTick ) ) {
            count[chann] = 0;
        }
               
        if ( CurrTime > (sFlowVar[chann].LastTime_u32 + FreqSend) ) {
            sPulse[chann].Flow_f = 0;
        } else {
            //qua thoi gian duration xung truoc moi cho giam dan
            if ( (count[chann] >= 3)
                   && ( CurrTime > (sFlowVar[chann].LastTime_u32 + sFlowVar[chann].DelTime_u32 + sFlowVar[chann].PlusTime_u32) ) ) {
                rCacul = true;
                sFlowVar[chann].PlusTime_u32 += 10000;   //them 10s sau moi check
            } 
        }
    }
    
    if (rCacul == true) {
        //vao day count[chann] luon >= 3
        pulse = AppWm_Cacul_Quantity(sFlowVar[chann].sSamplePulse[count[chann] - 1], sFlowVar[chann].sSamplePulse[1]);
        duratime = CurrTime - HW_RTC_GetCalendarValue_ms(sFlowVar[chann].sSampleTime[1]);
        
        if (duratime > 0) {
            //tinh ra luu luong
            TempDouble = pulse * sPulse[chann].FactorInt_i16 * 3600 * Convert_Scale(sPulse[chann].FactorDec_u8 + 2) * SecondToTick / duratime;  
            sPulse[chann].Flow_f = (float) (TempDouble) ;
        }  
    }
    
    if (mNewPulse == true) {
        //neu count > MAX -1: cuon sample
        if (count[chann] >= (MAX_SAMPLE_FLOW - 1) ) {
            for (i = 0; i < count[chann]; i++) {
                UTIL_MEM_cpy(&sFlowVar[chann].sSampleTime[i], &sFlowVar[chann].sSampleTime[i+1], sizeof(ST_TIME_FORMAT));
                UTIL_MEM_cpy(&sFlowVar[chann].sSamplePulse[i], &sFlowVar[chann].sSamplePulse[i+1], sizeof(uint64_t));
            }
            
            count[chann]--;
        }
    }
    
    return true;
}



/*
    Func: Set Mode Control Led
        - Mode: + _LED_MODE_ONLINE_INIT     : Nhay deu 1s
                + _LED_MODE_CONNECT_SERVER  : Nhay Duty: 430ms off, 70ms on
                + _LED_MODE_UPDATE_FW       : Nhay deu 100ms
                + _LED_MODE_POWER_SAVE      : Off led
                + _LED_MODE_TEST_PULSE      : Nhay theo xung doc vao
*/
uint8_t AppWm_Get_Mode_Led (void)
{
    uint8_t Result = 0;
    
    #ifdef USING_APP_SIM
        if ((UTIL_var.ModeConnNow_u8 != _CONNECT_DATA_MAIN) 
            && (UTIL_var.ModeConnNow_u8 != _CONNECT_DATA_BACKUP)
            && (UTIL_var.ModeConnNow_u8 != _CONNECT_FTP_UPLOAD))  
        {
            Result = _LED_MODE_UPDATE_FW;
        } else if (sWmVar.rTestsPulse_u8 == FALSE)
        {
            if ( (sSimCommVar.State_u8 == _SIM_CONN_MQTT)
            #ifdef USING_APP_ETHERNET
                || (sAppEthVar.Status_u8 == _ETH_MQTT_CONNECTED) 
            #endif
                    )
                Result = _LED_MODE_CONNECT_SERVER;
            else
                Result = _LED_MODE_ONLINE_INIT;
        } else
            Result = _LED_MODE_TEST_PULSE;
    #else
        if (sWmVar.rTestsPulse_u8 == FALSE) 
        {
            Result = _LED_MODE_ONLINE_INIT;
        } else
            Result = _LED_MODE_TEST_PULSE;  
    #endif
        
    return Result;
}

/*
    Func: CB Event Control Led
*/
static uint8_t _Cb_Control_Led1(uint8_t event)
{
    uint8_t ModeControl = 0;
    static uint8_t CountToggle = 0;
    static uint8_t MarkFirst = 0;
    static uint8_t Status = 0;
    static uint64_t LastPulseControl = 0;
    static uint32_t LastTimeCheckPulse = 0;
    
    if ( (sWmVar.rDefault_u8 == true) && (CountToggle <= 10) )
    {
        ModeControl = _LED_MODE_DEFAULT_DEV;
    } else
    {
        ModeControl = AppWm_Get_Mode_Led();
    }
    
    //Handler in step control one
    switch ( ModeControl )
    {
        case _LED_MODE_DEFAULT_DEV: 
            CountToggle++;
            sEventAppWM[event].e_period = 100;
            LED_Toggle(_LED_STATUS);
            break;
        case _LED_MODE_ONLINE_INIT:
            sEventAppWM[event].e_period = 1000;
            LED_Toggle(_LED_STATUS);
            break;
        case _LED_MODE_CONNECT_SERVER:
            if (sEventAppWM[event].e_period != 430)
            {
                sEventAppWM[event].e_period = 430;
                LED_Off (_LED_STATUS);
            } else
            {
                sEventAppWM[event].e_period = 70;
                LED_On (_LED_STATUS);
            }
            break;
        case _LED_MODE_UPDATE_FW:
            sEventAppWM[event].e_period = 100;
            LED_Toggle(_LED_STATUS);
            break;      
        case _LED_MODE_POWER_SAVE:
            sEventAppWM[event].e_period = 1000;
            LED_Off (_LED_STATUS);
            break;
        case _LED_MODE_TEST_PULSE:
            sEventAppWM[event].e_period = 200;
            if (MarkFirst == 0)
            {
                MarkFirst = 1;
                LastPulseControl = sPulse[0].Number_i64;
                LastTimeCheckPulse = HAL_GetTick();
                LED_On (_LED_STATUS);
                
                return 1;
            }
            
            #ifdef LPTIM_ENCODER_MODE 
                LPTIM_Encoder_Get_Pulse(&hlptim1, &sPulse[0]);
            #else   
                LPTIM_Counter_Get_Pulse(&hlptim1, &sPulse[0]);
                AppWm_Plus_5Pulse(&sPulse[0]);
            #endif
                
            if (Cal_Time(LastTimeCheckPulse, HAL_GetTick()) <= 60000)
            {
                //Neu co xung nuoc thay doi -> Nhay led
                if (LastPulseControl != sPulse[0].Number_i64)
                {
                    LastPulseControl = sPulse[0].Number_i64;
                    
                    if (Status == FALSE)
                        Status = TRUE;
                } 
                
                if (Status == TRUE)
                {
                    LED_Off (_LED_STATUS);
                    Status = PENDING;
                } else if (Status == PENDING)   
                {
                    LED_On (_LED_STATUS);
                    Status = FALSE;
                }
            }
        default:
            break;
    }
    
    fevent_enable(sEventAppWM, event);
       
    return 1;
}



uint8_t AppWm_Digital_Decode (uint8_t chann, sData *pdata)
{
    uint8_t result = false;
    sModbusDeviceData *target = &sWmDigVar.sModbDevData[chann];
    uint8_t type = sWmDigVar.sModbInfor[chann].MType_u8;
        
    switch (type)
    {
        case __MET_WOTECK: 
            result = sListWmDigital[type].fDecode(0, pdata, &target->sWoteck01);
            break;
        case __MET_LEVEL_LIQ: 
            result = sListWmDigital[type].fDecode(0, pdata, &target->sLevel01);
            break;
        case __MET_LEVEL_LIQ_SUP: 
            result = sListWmDigital[type].fDecode(0, pdata, &target->sLevelSup);
            break;
        case __MET_MT100: 
            result = sListWmDigital[type].fDecode(0, pdata, &target->sMt100);
            break;
        case __MET_LEVEL_ULTRA: 
            result = sListWmDigital[type].fDecode(0, pdata, &target->sLevelUltra);
            break;
        case __MET_WM_SENCE: 
            result = sListWmDigital[type].fDecode(0, pdata, &target->sSence);
            break;
        case __MET_WOTECK_ULTRA: 
            result = sListWmDigital[type].fDecode(sWmDigVar.sModbDevData[chann].inReg, pdata, \
                                                  &target->sWoteckUltra);
            break;
        default:
            break;
    }
    
    return result;
}



/*
    Func: process read rs485 1 
*/

uint8_t AppWm_RS485_1_Process (uint8_t chann)
{
//    uint8_t result = pending;
//    static uint8_t step = 0;
//    uint8_t nreg = 0, func = FUN_READ_BYTE, type = sWmDigVar.sModbInfor[chann].MType_u8;
//    static uint32_t landmark = 0; 
//    sData  pData;
//    uint16_t addr = 0;
//    
//    if (type == __MET_UNKNOWN) {
//        return false;
//    }
//    
//    switch (step)
//    {
//        case 0:
//            RS485_Stop_RX_Mode();
//            WM_DIG_Init_Uart(&uart_rs485, type);
//            RS485_Init_RX_Mode();
//            sWmDigVar.sModbDevData[chann].inReg = 0;
//            step++;
//            break;
//        case 1:  //Send request frame modrtu 
//            WM_DIG_Get_Infor(chann, type, &addr, &nreg);
//            RS485_Modbus_Send(sWmDigVar.sModbInfor[chann].SlaveId_u8, func, addr, nreg);
//            landmark = RtCountSystick_u32;
//            step++;
//            break;
//        case 2: //Check ACK
//            if (Check_Time_Out(landmark, 1000) == true) {
//                step = 0;
//                result = false;
//            } else {
//                if (Rs485Status_u8 == true)
//                {       
//                    UTIL_Printf_Hex( DBLEVEL_L, sUart485.Data_a8, sUart485.Length_u16);
//                    UTIL_Printf_Str( DBLEVEL_L,"\r\n");
//                    
//                    step = 0;
//                    result = ModRTU_Check_Format(sUart485.Data_a8, sUart485.Length_u16);
//                    
//                    if (result == true) {
//                        pData.Data_a8 = sUart485.Data_a8 + 3;
//                        pData.Length_u16 = sUart485.Length_u16 - 3;
//                        
//                        result = AppWm_Digital_Decode(chann, &pData);
//
//                        if (result == error) { 
//                            result = false;                       
//                        } else if (result != true) {
//                            step = 1;  //tiep tuc hoi tiep lenh
//                        } 
//                        
//                        sWmDigVar.sModbDevData[chann].inReg++;
//                    }
//                }
//            }
//            break;
//        default:
//            step = 0;
//            break;
//    }
    
//    return result;
    return 0;
}




static uint8_t _Cb_Read_RS485_Modbus (uint8_t event)
{
//    static uint8_t retry = 0, cDisconnect = 0;
//    static uint8_t poweron = false;
//    uint8_t result = false;
//    uint8_t status = false;
//    
//    if (retry < 2)
//    {
//        status = AppWm_RS485_1_Process(0);
//        
//        if (status == true) {
//            //cap nhat lai mepdv neu co kenh moi
//            if (sWmDigVar.sModbDevData[0].Status_u8 == false) {
//                sMessage.aMESS_PENDING[DATA_HANDSHAKE] = TRUE;
//            }
//            //get data to channel modbus de hien thay
//            WM_DIG_Get_Data(0, sWmDigVar.sModbInfor[0].MType_u8);
//            
//            result = true;
//            cDisconnect = 0;
//            sWmDigVar.sModbDevData[0].LandMark_u32 = RtCountSystick_u32;
//        } else if (status == false) {
//            retry++;
//        }
//        
//        //enable su kien
//        if (poweron == false) {
//            poweron = true;
//            V_PIN_ON;  //On power: cap nguon cho level
//            RS485_ON;
//            sEventAppWM[event].e_period = 1000;
//        } else {
//            sEventAppWM[event].e_period = 50;
//        }
//        
//        fevent_enable(sEventAppWM, event);
//    } else {
//        cDisconnect++;
//        result = true;
//        
//        if (sWmDigVar.sModbDevData[0].Status_u8 == true) {
//            if (sWmVar.pPack_Alarm_Str != NULL)
//                sWmVar.pPack_Alarm_Str ("u_app_wm: modbus 1 fail!\r\n");
//        } else {
//            UTIL_Printf_Str(DBLEVEL_M, "u_app_wm: modbus 1 fail!\r\n" );
//        }
//        
//        if (cDisconnect >= MAX_DISCONN_RS485) {
//            sWmDigVar.sModbDevData[0].Status_u8 = false;
//        }
//    }
//    
//    if (result == true)
//    {
//        poweron = false;
//        retry = 0;
//        RS485_Stop_RX_Mode();
//        
//        iMarkTSVH |= 0x04;
//        if (iMarkTSVH == 0x0F) {
//            iMarkTSVH = 0;
//            fevent_active(sEventAppWM, _EVENT_LOG_TSVH);
//        }
//    
//        if ( (UTIL_var.ModePower_u8 == _POWER_MODE_SAVE) && (iMarkTSVH == 0x0F) ) {
//            V_PIN_OFF;  
//        } else {
//            sEventAppWM[event].e_period = PERIOD_READ_MODBUS;
//            fevent_enable(sEventAppWM, event);
//        }
//        
//        if ((UTIL_var.ModeConnNow_u8 != _CONNECT_DATA_MAIN) 
//            && (UTIL_var.ModeConnNow_u8 != _CONNECT_DATA_BACKUP)
//            && (UTIL_var.ModeConnNow_u8 != _CONNECT_FTP_UPLOAD)) {
//            fevent_disable(sEventAppWM, event);
//        }
//    }
    
    return true;
}




/*
    Func: process read rs485 2 
*/

uint8_t AppWm_RS485_2_Process (uint8_t chann)
{
//    uint8_t result = pending;
//    static uint8_t step = 0;
//    uint8_t nreg = 0, func = FUN_READ_BYTE, type = sWmDigVar.sModbInfor[chann].MType_u8;
//    static uint32_t landmark = 0; 
//    sData  pData;
//    uint16_t addr = 0;
//    
//    if (type == __MET_UNKNOWN) {
//        return false;
//    }
//    
//    switch (step)
//    {
//        case 0:
//            RS485_2_Stop_RX_Mode();
//            WM_DIG_Init_Uart(&uart_rs485_2, type);
//            RS485_2_Init_RX_Mode();
//            sWmDigVar.sModbDevData[chann].inReg = 0;
//            step++;
//            break;
//        case 1:  //Send request frame modrtu 
//            WM_DIG_Get_Infor(chann, type, &addr, &nreg);
//            RS485_2_Modbus_Send(sWmDigVar.sModbInfor[chann].SlaveId_u8, func, addr, nreg);
//            landmark = RtCountSystick_u32;
//            step++;
//            break;
//        case 2: //Check ACK
//            if (Check_Time_Out(landmark, 500) == true) {
//                step = 0;
//                result = false;
//            } else {
//                if (Rs485_2Status_u8 == true)
//                {       
//                    UTIL_Printf_Hex( DBLEVEL_L, sUart485_2.Data_a8, sUart485_2.Length_u16);
//                    UTIL_Printf_Str( DBLEVEL_L,"\r\n");
//                    
//                    step = 0;
//                    result = ModRTU_Check_Format(sUart485_2.Data_a8, sUart485_2.Length_u16);
//                    
//                    if (result == true) {
//                        pData.Data_a8 = sUart485_2.Data_a8 + 3;
//                        pData.Length_u16 = sUart485_2.Length_u16 - 3;
//                        
//                        result = AppWm_Digital_Decode(chann, &pData);
//                        if (result != true) {
//                            step = 1;  //tiep tuc hoi tiep lenh
//                        }
//                        sWmDigVar.sModbDevData[chann].inReg++;
//                    }
//                }
//            }
//            break;
//        default:
//            step = 0;
//            break;
//    }
    
//    return result;
  return 0;
}



static uint8_t _Cb_Read_RS485_2_Modbus (uint8_t event)
{
//    static uint8_t retry = 0, cDisconnect = 0;
//    static uint8_t poweron = false;
//    uint8_t result = false;
//    uint8_t status = false;
//    
//    if (retry < 2)
//    {
//        status = AppWm_RS485_2_Process(1);
//        
//        if (status == true) {
//            //cap nhat lai mepdv neu co kenh moi
//            if (sWmDigVar.sModbDevData[1].Status_u8 == false) {
//                sMessage.aMESS_PENDING[DATA_HANDSHAKE] = TRUE;
//            }
//            //get data to channel modbus de hien thay
//            WM_DIG_Get_Data(1, sWmDigVar.sModbInfor[1].MType_u8);
//            
//            result = true;
//            cDisconnect = 0;
//            sWmDigVar.sModbDevData[1].LandMark_u32 = RtCountSystick_u32;
//        } else if (status == false) {
//            retry++;
//        }
//        
//        //enable su kien
//        if (poweron == false) {
//            poweron = true;
//            V_PIN_ON;  //On power: cap nguon cho level
//            RS485_ON;
//            sEventAppWM[event].e_period = 1000;
//        } else {
//            sEventAppWM[event].e_period = 50;
//        }
//        
//        fevent_enable(sEventAppWM, event);
//    } else {
//        cDisconnect++;
//        result = true;
//        
//        if (sWmDigVar.sModbDevData[1].Status_u8 == true) {
//            if (sWmVar.pPack_Alarm_Str != NULL)
//                sWmVar.pPack_Alarm_Str ("u_app_wm: modbus 2 fail!\r\n");
//        } else {
//            UTIL_Printf_Str(DBLEVEL_M, "u_app_wm: modbus 2 fail!\r\n" );
//        }
//        
//        if (cDisconnect >= MAX_DISCONN_RS485) {
//            sWmDigVar.sModbDevData[1].Status_u8 = false;
//        }
//    }
//    
//    if (result == true)
//    {
//        poweron = false;
//        retry = 0;
//        RS485_2_Stop_RX_Mode();
//        
//        iMarkTSVH |= 0x08;
//        if (iMarkTSVH == 0x0F) {
//            iMarkTSVH = 0;
//            fevent_active(sEventAppWM, _EVENT_LOG_TSVH);
//        }
//    
//        if ( (UTIL_var.ModePower_u8 == _POWER_MODE_SAVE) && (iMarkTSVH == 0x0F) ) {
//            V_PIN_OFF;  
//        } else {
//            sEventAppWM[event].e_period = PERIOD_READ_MODBUS;
//            fevent_enable(sEventAppWM, event);
//        }
//        
//        if ((UTIL_var.ModeConnNow_u8 != _CONNECT_DATA_MAIN) 
//            && (UTIL_var.ModeConnNow_u8 != _CONNECT_DATA_BACKUP)
//            && (UTIL_var.ModeConnNow_u8 != _CONNECT_FTP_UPLOAD)) {
//            fevent_disable(sEventAppWM, event);
//        }
//    }
//    
    return true;
}



static uint8_t _Cb_Power_Up_12V(uint8_t event)
{
    UTIL_TIMER_Start (&Timer12VPowerUp);

    return 1;
}

/*
    Func: Callback event read input contact
*/

static uint8_t _Cb_Get_Contact_Input(uint8_t event)
{
    fevent_enable(sEventAppWM, event);
    
    return 1;
}



static uint8_t _Cb_Rs485_1_Recv (uint8_t event)
{
    static uint8_t  mFirst = 0;
    static uint16_t lastlen = 0; 
        
    if (mFirst == 0)
    {
        mFirst = 1;
        lastlen = sUart485.Length_u16;
        fevent_enable(sEventAppWM, event);
    } else {
        if (sUart485.Length_u16 == lastlen)
        {
            Rs485Status_u8 = true;
            mFirst = 0;
        } else {
            lastlen = sUart485.Length_u16;
            fevent_enable(sEventAppWM, event);
        }
    }
    
    return 1;
}



static uint8_t _Cb_Rs485_2_Recv (uint8_t event)
{
    static uint8_t  mFirst = 0;
    static uint16_t lastlen = 0; 
        
    if (mFirst == 0)
    {
        mFirst = 1;
        lastlen = sUart485_2.Length_u16;
        fevent_enable(sEventAppWM, event);
    } else {
        if (sUart485_2.Length_u16 == lastlen)
        {
            mFirst = 0;
            Rs485_2Status_u8 = true;
        } else {
            lastlen = sUart485_2.Length_u16;
            fevent_enable(sEventAppWM, event);
        }
    }
    
    return 1;
}



/*================ Function Handler =================*/

uint16_t AppWm_Get_VBat(void)
{
//    //Get batterry
//    sBattery.mVol_u32 = ADC_Get_Value (ADC_CHANNEL_6);
//    sBattery.mVol_u32 *= 2;   //Phan ap chia 2
//    sBattery.Level_u16 = ADC_Convert_Vol_To_Percent (sBattery.mVol_u32, VDD_BAT, VDD_MIN); 

	return sBattery.Level_u16;
}

uint16_t AppWm_Get_VOut(void)
{
    sVout.mVol_u32 = ADC_Get_Value(ADC_CHANNEL_9);
    sVout.mVol_u32 = sVout.mVol_u32 * 517 / 47;
    sVout.Level_u16 = ADC_Convert_Vol_To_Percent (sVout.mVol_u32, VDD_OUT_MAX, VDD_OUT_MIN); 
   
	return sVout.Level_u16;
}


int32_t AppWm_Pressure_ADC_Vol(uint32_t Channel)
{
    static uint16_t aPressmVol[10] = {0x00};
    static uint8_t Index = 0;
    
    uint8_t j;
    uint16_t minData;
    uint16_t tempVol;
    
    int32_t reVal = -1;
    
    //Get mV ADC
    tempVol = ADC_Get_Value(Channel);
    aPressmVol[Index++] = (uint16_t)tempVol;
    
    //If 10 sample -> analys
    if (Index >= 10)
    {
        /* arrange */
        for(Index = 0; Index < 5; Index++)
        {
            for(j = Index+1; j <= (9 - Index); j++)
            {
                if (aPressmVol[j] < aPressmVol[Index])
                {
                    minData = aPressmVol[Index];
                    aPressmVol[Index] = aPressmVol[j];
                    aPressmVol[j] = minData;
                }
                if (aPressmVol[9-j] > aPressmVol[9-Index])
                {
                    minData = aPressmVol[9-Index];
                    aPressmVol[9-Index] = aPressmVol[9-j];
                    aPressmVol[9-j] = minData;
                }
               
            }
        }
        //Get average from index 3 -> 7
        tempVol = 0;
        for(Index = 3; Index < 7; Index++)
        {
            tempVol += aPressmVol[Index];
        }
        
        reVal = (int32_t)(tempVol / 4);
        
        //Reinit index and buff
        Index = 0;
        UTIL_MEM_set( aPressmVol, 0, sizeof(aPressmVol) );
    } 

    return reVal;
}

/*
    Func: chuyen doi mV sang bar
        + Mode 0: cho nguon dong: 4mA -> 20mA tuong ung 0 - 10 bar
        + Mode 1: Cho nguon ap: 0 -> 10V tuong ung 0 -> 10 bar
*/
uint32_t AppWm_Convert_mV_To_Press_Unit (uint32_t mVol, uint8_t mode)
{
    uint32_t Result = 0;
    uint32_t VolReal = 0, ResolutionBar = 0;
    uint16_t VolHigh = 0;
    uint16_t VolLow = 0;
    uint8_t IsOverFlow = false, IsLessFlow = false;
    
    ResolutionBar = (PRESSURE_HIGH_BAR - PRESSURE_LOW_BAR) * 1000; //scale 1000 
    
    switch (mode)
    {
        case 0:  //nguon dong: 4mA -> 20mA
            VolReal = mVol;
            
            VolLow = PRESSURE_LOW_CURR * PRESSURE_RESISTOR;
            VolHigh = PRESSURE_HIGH_CURR * PRESSURE_RESISTOR;

            //Noi suy tuyen tinh
            if ( (VolReal > VolLow) && (VolReal <= VolHigh) )
            {
                Result = ((VolReal - VolLow) * ResolutionBar) / (VolHigh - VolLow);    
            } else if (VolReal <= VolLow) 
            {
                IsLessFlow = true;
            } else if (VolReal > VolHigh) 
            {
                IsOverFlow = true;
            }
            break;
        case 1:  //nguon ap 0 -> 10V
            VolReal = mVol * PRESSURE_DIVIDE_VOL;
        
            if (VolReal <= PRESSURE_HIGH_VOL2)
            {
                Result = VolReal;
            } else
            {
                IsOverFlow = true;
            }
            break;
        case 2:
            VolReal = mVol * PRESSURE_DIVIDE_VOL;
        
            if ((VolReal >= PRESSURE_LOW_VOL3) && (VolReal <= PRESSURE_HIGH_VOL3))
            {
                Result = ((VolReal - PRESSURE_LOW_VOL3) * ResolutionBar) / (PRESSURE_HIGH_VOL3 - PRESSURE_LOW_VOL3) ;  //scale 1000;
            } else if (VolReal < PRESSURE_LOW_VOL3)
            {
                IsLessFlow = true;
            } else
            {
                IsOverFlow = true;
            }
            break;
        case 3:   //0 ->10V tuong ung 0 -> 5 bar
            VolReal = mVol * PRESSURE_DIVIDE_VOL;
        
            if (VolReal <= PRESSURE_HIGH_VOL2)
            {
                Result = VolReal / 2;
            } else
            {
                IsOverFlow = true;
            }
            break;
        default:
            break;
    }
    
    if (IsLessFlow == true)
    {
        Result = 0;
        UTIL_Printf_Str(DBLEVEL_M, "u_app_wm: not insert pressure!\r\n" );
    }
    
    if (IsOverFlow == true)
    {
        Result = 0xFF * ResolutionBar;      /* Over flowing */
        UTIL_Printf_Str(DBLEVEL_M, "u_app_wm: over flowing!\r\n" );     
    }
    
    return Result;
}


//neu khoi dong lai: cong them 5 xung ban dau cua che do counter
void AppWm_Plus_5Pulse (Struct_Pulse *sPulse)
{    
    if ( (sPulse->IsOver5Pulse_u8 == false)
        && (sPulse->Number_i64 != sPulse->NumberInit_i64) ) {
        if (sPulse->Direct_u8 == 1) {
            sPulse->Number_i64  -= 5;
        } else {
            sPulse->Number_i64  += 5;
        }
        
        sPulse->IsOver5Pulse_u8 = true;
    }
}

/*
    Func:  Check Mode tranfer data: ONLINE OR SAVE_POWER
        - VoutmV: Voltage PIN out (2).
        - If: Vout > 10,500 mV -> ONLINE
    Return: 1: If have Change mode
            0: If no change mode
*/
uint8_t AppWm_Check_Change_Mode (uint8_t *Mode, uint32_t VoutmV)
{
    if (*Mode == _POWER_MODE_ONLINE)
    {
        if (VoutmV <= 10100)
        {
            if (sWmVar.pPack_Alarm_Str != NULL)
                sWmVar.pPack_Alarm_Str ("u_app_wm: change to save mode!\r\n");

            *Mode = _POWER_MODE_SAVE;
            return TRUE;
        }
    } else
    {
        if (VoutmV > 10900)
        {
            if (sWmVar.pPack_Alarm_Str != NULL)
                sWmVar.pPack_Alarm_Str ("u_app_wm: change to run mode!\r\n");
            
            *Mode = _POWER_MODE_ONLINE;
            return TRUE;
        }
    }
    
    return FALSE;
}



void AppWm_Check_Mode_Power (void)   
{
    if (AppWm_Analys_Vout() == true)
    {
        if (*sWmVar.pHardware_u8 == __HARD_ONLINE_PSM)
        {
            //do dien ap pin ngoai -> chuyen mode
            if (AppWm_Check_Change_Mode(&UTIL_var.ModePower_u8, sVout.mVol_u32) == TRUE)
            {
                //Print to debug
                APP_LOG(TS_OFF, DBLEVEL_M, "u_app_wm: Vpin out: %d \r\n", sVout.mVol_u32);
                
            #ifdef USING_APP_COMM
                AppComm_Set_Next_TxTimer();
            #endif
                
            #ifdef USING_APP_ETHERNET    
                AppEth_Restart_If_PSM();
            #endif
                
            #ifdef USING_APP_SIM
                AppSim_Restart_If_PSM(); 
            #endif
            }
        }
            
        if (sWmVar.IrqPowUp_u8 == waiting) 
            sWmVar.IrqPowUp_u8 = false;
    }
}


uint8_t AppWm_Analys_Vout (void)
{
    static uint32_t TotalVout = 0;    
    static uint8_t IndexSample = 0;
    
    if (sWmVar.IrqPowUp_u8 == pending) 
    {
        sWmVar.IrqPowUp_u8 = waiting;
        IndexSample = 0;
        TotalVout = 0;
    }
    
    if (IndexSample < 10)
    {
        AppWm_Get_VOut();
        
        TotalVout += sVout.mVol_u32;
        IndexSample++;      
    } else
    {
        sVout.mVol_u32 = TotalVout / 10;
        
        IndexSample = 0;
        TotalVout = 0;
        
        return true;
    }
    
    return false; 
}

/*
    Func: Tat het nguon tieu thu nang luong cua app temh
        + Off 12v
        + Off Led 1 2 3
*/

void AppWm_Off_Energy_Consumption (void)
{
    LED_Off (_LED_STATUS);
    
    V_PIN_OFF;   //off Power
    RS485_OFF;   //Off 485 de go to sleep mode
}


uint8_t AppWm_Scan_Alarm_Channel (uint8_t chann)
{
    uint8_t i = 0;
        
    if (sWmVar.iCaculOK[chann] == true) {
//        if (sPulse[chann].Quantity_f > sMeterThreshold[chann].PeakHigh) 
//        {
//            if (sWmVar.sAlarm[chann].StaQuanHi_u8 == false) {
//                sWmVar.sAlarm[chann].StaQuanHi_u8 = true;
//                UTIL_Log_Str(DBLEVEL_M, "Event: High Quantity Start\r\n");
//                sWmVar.sAlarm[chann].aMARK_ALARM[_ALARM_PEAK_HIGH] = 1;
//            }
//        } else {
//            if (sWmVar.sAlarm[chann].StaQuanHi_u8 == true) {
//                sWmVar.sAlarm[chann].StaQuanHi_u8 = false;
//                UTIL_Log_Str(DBLEVEL_M, "Event: High Quantity Stop\r\n");
//                sWmVar.sAlarm[chann].aMARK_ALARM[_ALARM_PEAK_HIGH] = 1;
//            }
//        }
//        
//        if (sPulse[chann].Quantity_f < sMeterThreshold[chann].PeakLow)  
//        {
//            if (sWmVar.sAlarm[chann].StaQuanLow_u8 == false) {
//                sWmVar.sAlarm[chann].StaQuanLow_u8 = true;
//                UTIL_Log_Str(DBLEVEL_M, "Event: Low Quantity Start\r\n");
//                sWmVar.sAlarm[chann].aMARK_ALARM[_ALARM_PEAK_LOW] = 1;
//            } 
//        } else {
//            if (sWmVar.sAlarm[chann].StaQuanLow_u8 == true) {
//                sWmVar.sAlarm[chann].StaQuanLow_u8 = false;
//                UTIL_Log_Str(DBLEVEL_M, "Event: Low Quantity Stop\r\n");
//                sWmVar.sAlarm[chann].aMARK_ALARM[_ALARM_PEAK_LOW] = 1;
//            }
//        }
//        
//        if (sPulse[chann].Flow_f > sMeterThreshold[chann].FlowHigh) 
//        {
//            if (sWmVar.sAlarm[chann].StaFlowHi_u8 == false) {
//                sWmVar.sAlarm[chann].StaFlowHi_u8 = true;
//                UTIL_Log_Str(DBLEVEL_M, "Event: High Flow Start\r\n");
//                sWmVar.sAlarm[chann].aMARK_ALARM[_ALARM_FLOW_HIGH] = 1;
//            } 
//        } else { 
//            if (sWmVar.sAlarm[chann].StaFlowHi_u8 == true) {
//                sWmVar.sAlarm[chann].StaFlowHi_u8 = false;
//                UTIL_Log_Str(DBLEVEL_M, "Event: High Flow Stop\r\n");
//                sWmVar.sAlarm[chann].aMARK_ALARM[_ALARM_FLOW_HIGH] = 1;
//            }
//        }
//        
//        if (sPulse[chann].Flow_f < sMeterThreshold[chann].FlowLow)  //   
//        {
//            if (sWmVar.sAlarm[chann].StaFlowLow_u8 == false) {
//                sWmVar.sAlarm[chann].StaFlowLow_u8 = true;
//                UTIL_Log_Str(DBLEVEL_M, "Event: Low Flow Start\r\n");
//                sWmVar.sAlarm[chann].aMARK_ALARM[_ALARM_FLOW_LOW] = 1;
//            }
//        } else { 
//            if (sWmVar.sAlarm[chann].StaFlowLow_u8 == true) {
//                sWmVar.sAlarm[chann].StaFlowLow_u8 = false;
//                UTIL_Log_Str(DBLEVEL_M, "Event: Low Flow Stop\r\n");
//                sWmVar.sAlarm[chann].aMARK_ALARM[_ALARM_FLOW_LOW] = 1;
//            }
//        }
    }
    
    //Check Vbat threshold
    if (sBattery.Level_u16 < sMeterThreshold[0].LowBatery) 
    {
        if (sWmVar.sAlarm[0].StaBatLow_u8 == false) {
            sWmVar.sAlarm[0].StaBatLow_u8 = true;
            UTIL_Log_Str(DBLEVEL_M, "Event: Low bat Start\r\n");
            sWmVar.sAlarm[chann].aMARK_ALARM[_ALARM_VBAT_LOW] = 1;
        }
    } else { 
        if (sWmVar.sAlarm[0].StaBatLow_u8 == true) {
            sWmVar.sAlarm[0].StaBatLow_u8 = false;
            UTIL_Log_Str(DBLEVEL_M, "Event: Low Flow Stop\r\n");
            sWmVar.sAlarm[chann].aMARK_ALARM[_ALARM_VBAT_LOW] = 1;
        }
    }
  
    //hard rs
    if (sWmVar.sAlarm[chann].StaHardRs_u8 == true) {
        sWmVar.sAlarm[chann].StaHardRs_u8 = false;
        sWmVar.sAlarm[chann].aMARK_ALARM[_ALARM_HARD_RESET] = 1;
    }
    
    //Check cac mark xem return 1 hay 0
    for(i = 0; i < _ALARM_END;i++) {
        if (sWmVar.sAlarm[chann].aMARK_ALARM[i] == 1) {
            return true;
        }
    }
    
    return false;
}

uint8_t AppWm_Scan_Alarm (void)
{    
    uint8_t result = false;
    
    for (uint8_t i = 0; i < MAX_CHANNEL; i++) {
        if (AppWm_Scan_Alarm_Channel(i) == true) {
            AppWm_Log_Data_Event(i);
            result = true;
        }
    }
    
    return result;
}


uint8_t AppWm_Packet_Intan (sData *pData)
{
    uint16_t PosIndex = 0, LengthPacket = 0;
    uint8_t TempCrc = 0;
    uint16_t i = 0;
    
    //Dong goi tung ban tin cac kenh
    for (i = 0; i < MAX_CHANNEL; i++)
    {
        PosIndex = pData->Length_u16;
        pData->Length_u16 += 2;
        LengthPacket = AppWm_Packet_TSVH_Channel(pData, i);
       
        *(pData->Data_a8 + PosIndex) = i + 1;
        *(pData->Data_a8 + PosIndex + 1) = LengthPacket;
    }

    // caculator crc
    pData->Length_u16++;
	for (i = 0; i < (pData->Length_u16 - 1); i++)
		TempCrc ^= pData->Data_a8[i];

    pData->Data_a8[pData->Length_u16-1] = TempCrc; 
    
    return true;
}


void AppWm_Packet_TSVH (void)
{
    uint8_t aTEMP_PAYLOAD[512] = {0};
    sData pData = {aTEMP_PAYLOAD, 0};
    uint16_t i = 0, a = 0, posindex = 0, length = 0;
    uint8_t TempCrc = 0;// caculator crc
    uint8_t count = 0;
    static uint16_t size_pack = 0;
    
    for (i = 0; i < MAX_CHANNEL; i++)
    {
        posindex = pData.Length_u16;
        pData.Length_u16 += 2;
        
        length = AppWm_Packet_TSVH_Channel(&pData, i);
        
        *(pData.Data_a8 + posindex) = count + 1;
        *(pData.Data_a8 + posindex + 1) = length;
    
        if (count == 0) {
            size_pack = pData.Length_u16;
        }
        
        count++;

        //gop 2 ban tin voi nhau. hoac cuoi cung
        if ( (count > 0) &&
            ( (size_pack + pData.Length_u16 + 5) > sRecTSVH.Size_u16 ) ||  ( (i + 1) >= MAX_CHANNEL ) )
        {
            count = 0;
      
            pData.Length_u16++;
            for (a = 0; a < (pData.Length_u16 - 1); a++)
                TempCrc ^= pData.Data_a8[a];

            pData.Data_a8[pData.Length_u16-1] = TempCrc;
            
            Mem_Write_Data(sMemVar.Type_u8, _MEM_DATA_TSVH, 0, 
                      pData.Data_a8, pData.Length_u16, sRecTSVH.Size_u16);
            
            //reset buff 
            Reset_Buff(&pData);
            TempCrc = 0;
        }
    }
}


uint8_t AppWm_Packet_Param (char *pdata, uint8_t chann, uint8_t param)
{
    float temp_f = 0;
    uint8_t result = false;
      
    switch (param)
    {
        case 0:  //luu luong
            if (chann < MAX_CHANNEL) {
                sprintf(pdata, "%.3lf %s", sPulse[chann].Flow_f, "m3/h");
                result = true;
            } else {
                chann -= MAX_CHANNEL;
                if (sWmDigVar.sModbDevData[chann].Status_u8 == true) {
                    temp_f = sWmDigVar.sModbDevData[chann].Flow_i32 * Convert_Scale(sWmDigVar.sModbDevData[chann].Factor);
                    sprintf(pdata, "%.3lf %s", temp_f, "m3/h");
                    result = true;
                }
            }
            break;
        default:  //muc nuoc
            if (chann >= MAX_CHANNEL) {
                chann -= MAX_CHANNEL;
                if (sWmDigVar.sModbDevData[chann].Status_u8 == true) {
                    temp_f = sWmDigVar.sModbDevData[chann].LVal_i16 * Convert_Scale(0 - sWmDigVar.sModbDevData[chann].LDecimal_u16 - 2);
                    sprintf(pdata, "%.3lf %s", temp_f, "m");
                    result = true;
                }
            }
            break;
    }
    
    return result;
}


void AppWm_Packet_TNMT (void)
{
    uint8_t tnstatus_u8 = 0x00;  
    char aPAY_LOAD[MAX_MEM_DATA] = {0};
    int8_t chann = 0, i = 0, cPack = 0, mPack = false;
    char aTEMP[48] = {0};
    
    char pathtest[40] = "SV";
    char aName[40] = {0};
//    static uint32_t cFile = 0;
    
    switch (sWmVar.ModePacket_u8) 
    {
        case 0:
            chann = AppWm_Is_Firt_Chann_TN();
            if (chann >= 0) {
                if (AppWm_Packet_Param(aTEMP, chann, sWmVar.sChannInfor[chann].aPARAM[0]) == true) {
                    sprintf(aPAY_LOAD + strlen(aPAY_LOAD), "%04d%02d%02d%02d%02d%02d %s %02d\n", 
                                                sRTC.year + 2000, sRTC.month, sRTC.date,
                                                sRTC.hour, sRTC.min, sRTC.sec,
                                                aTEMP, tnstatus_u8);
                    
                    mPack = true;
                }
                
                if (mPack == true) {
                    Mem_Write_Data(sMemVar.Type_u8, _MEM_DATA_GPS, 0, 
                                        (uint8_t *) aPAY_LOAD, strlen(aPAY_LOAD), sRecGPS.Size_u16);
                    
//                    cFile++;
//                    sprintf (aName, "%s_%d.txt", pathtest, cFile);
                    sprintf (aName, "%s_%04d%02d%02d%02d%02d%02d.txt", pathtest, sRTC.year + 2000, sRTC.month, sRTC.date,
                                                sRTC.hour, sRTC.min, sRTC.sec);
                    UTIL_Printf_Str(DBLEVEL_M, "begin...\r\n");
                    sd_write_file(aName, aPAY_LOAD);
                }
                
            }
            break;
        case 1:
            chann = AppWm_Is_Firt_Chann_TN();
            if (chann >= 0) {
                for (i = 0; i < sWmVar.sChannInfor[chann].nParam_u8; i++) {
                    if (AppWm_Packet_Param(aTEMP, chann, sWmVar.sChannInfor[chann].aPARAM[i]) == true) {
                        sprintf(aPAY_LOAD + strlen(aPAY_LOAD), "%s %s %04d%02d%02d%02d%02d%02d %02d\n", 
                                                            AppWm_TN_PARAM[i], aTEMP,
                                                            sRTC.year + 2000, sRTC.month, sRTC.date,
                                                            sRTC.hour, sRTC.min, sRTC.sec,
                                                            tnstatus_u8);
                        
                        UTIL_MEM_set(aTEMP, 0, sizeof(aTEMP));
                        cPack++;
                        mPack = true;
                        if (cPack >= MAX_PACKET_SAVE) {
                            mPack = false;
                            Mem_Write_Data(sMemVar.Type_u8, _MEM_DATA_GPS, 0, 
                                    (uint8_t *) aPAY_LOAD, strlen(aPAY_LOAD), sRecGPS.Size_u16);
                            
                            UTIL_MEM_set(aPAY_LOAD, 0, sizeof(aPAY_LOAD));
                        }
                    }
                }
                //kiem tra xem con cai nao chua dong goi k?
                if (mPack == true) {
                    Mem_Write_Data(sMemVar.Type_u8, _MEM_DATA_GPS, 0, 
                                        (uint8_t *) aPAY_LOAD, strlen(aPAY_LOAD), sRecGPS.Size_u16);
                }
            }
            break;
        default:
            for (chann = 0; chann < MAX_CHANNEL + MAX_SLAVE_MODBUS; chann++) {
                if (strlen(sWmVar.sChannInfor[chann].MA_TINH) != 0) {
                    //dong goi Param channel nay
                    for (i = 0; i < sWmVar.sChannInfor[chann].nParam_u8; i++) {
                        if (AppWm_Packet_Param(aTEMP, chann, sWmVar.sChannInfor[chann].aPARAM[i]) == true) {
                            sprintf(aPAY_LOAD + strlen(aPAY_LOAD), "%s %s %s %04d%02d%02d%02d%02d%02d %02d\n", 
                                                                sWmVar.sChannInfor[chann].MA_TRAM, AppWm_TN_PARAM[i], aTEMP,
                                                                sRTC.year + 2000, sRTC.month, sRTC.date,
                                                                sRTC.hour, sRTC.min, sRTC.sec,
                                                                tnstatus_u8);
                            
                            UTIL_MEM_set(aTEMP, 0, sizeof(aTEMP));
                            cPack++;
                            mPack = true;
                            if (cPack >= MAX_PACKET_SAVE) {
                                mPack = false;
                                
//                                UTIL_Printf_Str(DBLEVEL_M, aPAY_LOAD);
                                Mem_Write_Data(sMemVar.Type_u8, _MEM_DATA_GPS, 0, 
                                        (uint8_t *) aPAY_LOAD, strlen(aPAY_LOAD), sRecGPS.Size_u16);
                                
                                UTIL_MEM_set(aPAY_LOAD, 0, sizeof(aPAY_LOAD));
                            }
                        }
                    }
                }
            }
            
            if (mPack == true) {
//                UTIL_Printf_Str(DBLEVEL_M, aPAY_LOAD);
                Mem_Write_Data(sMemVar.Type_u8, _MEM_DATA_GPS, 0, 
                                    (uint8_t *) aPAY_LOAD, strlen(aPAY_LOAD), sRecGPS.Size_u16);
            }
            break;
    }
}




double AppWm_Linear_Interpolation(double InVal,
                                   double InMin, double InMax,
                                   double OutMin, double OutMax)
{
    if ( (InMin > InMax) || (OutMin > OutMax) )
        return -1000;
    
    //3 truong hop
    //TH1: k lap 
    if (InVal < InMin) 
        return -1000;
    //
    if (InVal > InMax)
        return 0xFFFF *1000;
    
    return ( OutMin + ((InVal - InMin) * (OutMax - OutMin) / (InMax - InMin)) );	
}


uint8_t AppWm_Packet_TSVH_Channel (sData *pData, uint8_t channel)
{
    uint16_t FistPos = pData->Length_u16;
    uint64_t TempPulse = 0;
    
#ifdef USING_APP_COMM   
    uint16_t i = 0;
    uint8_t TempCrc = 0;
    uint8_t TempVal = 0;     
    uint8_t strChanel[] = {"0001"};
    
    //----------sTime--------------------
    SV_Protocol_Packet_Data(pData->Data_a8, &pData->Length_u16, OBIS_TIME_DEVICE, &sRTC, 6, 0xAA);
    
    //Channel ID
    strChanel[3] = 0x31 + channel;
    SV_Protocol_Packet_Data(pData->Data_a8, &pData->Length_u16, OBIS_SERI_SENSOR, &strChanel, 4, 0xAA);
    
    //----------chi so xung--------------------
#if (TSVH_OPERA_OR_FLASH == TSVH_MULTI)
    TempPulse = (uint64_t) ( sPulse[channel].Total_lf * Convert_Scale(0xFF - sPulse[channel].FactorDec_u8 + 1) );
       
    SV_Protocol_Packet_Data(pData->Data_a8, &pData->Length_u16, OBIS_WM_PULSE, &TempPulse, 8, sPulse[channel].FactorDec_u8);
    
    //----------luu luong --------------------
    TempPulse = (uint32_t) ( sPulse[channel].Flow_f * Convert_Scale(0xFF - sPulse[channel].FactorDec_u8 + 1) );
    SV_Protocol_Packet_Data(pData->Data_a8, &pData->Length_u16, OBIS_WM_DIG_FLOW, &TempPulse, 4, sPulse[channel].FactorDec_u8);   
#else
    #ifdef LPTIM_ENCODER_MODE 
        TempPulse = sPulse[channel].Number_i64 / 4;
    #else    
        TempPulse = sPulse[channel].Number_i64;
    #endif
        
    SV_Protocol_Packet_Data(pData->Data_a8, &pData->Length_u16, OBIS_WM_PULSE, &TempPulse, 8, 0x00);

    //----------luu luong --------------------
    SV_Protocol_Packet_Data(pData->Data_a8, &pData->Length_u16, OBIS_WM_FLOW, &TempVal, 2, 0x00);   
#endif
    
    //----------Ap suat--------------------
#ifdef PRESSURE_DECODE
    if (sWmVar.aPRESSURE[channel].sLinearInter.Type_u8 == __AN_PRESS) {
        SV_Protocol_Packet_Data(pData->Data_a8, &pData->Length_u16, OBIS_WM_PRESSURE, &sWmVar.aPRESSURE[channel].Val_i32, 2, 0xFD);    
    } else {
        SV_Protocol_Packet_Data(pData->Data_a8, &pData->Length_u16, OBIS_WM_LEVEL_VAL_SENSOR, &sWmVar.aPRESSURE[channel].Val_i32, 2, 0xFD);    
    } 
#else
    if (sWmVar.aPRESSURE[channel].sLinearInter.Type_u8 == __AN_PRESS) {
        SV_Protocol_Packet_Data(pData->Data_a8, &pData->Length_u16, OBIS_WM_PRESSURE, &sWmVar.aPRESSURE[channel].Val_i32, 2, 0);    
    } else {
        SV_Protocol_Packet_Data(pData->Data_a8, &pData->Length_u16, OBIS_WM_LEVEL_VAL_SENSOR, &sWmVar.aPRESSURE[channel].Val_i32, 2, 0);    
    }
#endif
    //----------Dien ap Pin--------------------
    SV_Protocol_Packet_Data(pData->Data_a8, &pData->Length_u16, OBIS_DEV_VOL1, &sBattery.mVol_u32, 2, 0xFD);
    
    //----------Cuong do song--------------------
#ifdef USING_APP_SIM
    SV_Protocol_Packet_Data(pData->Data_a8, &pData->Length_u16, OBIS_RSSI_1, &sSimCommInfor.RSSI_u8, 1, 0x00);
#endif
    
    //----------Dien ap Pin--------------------
    SV_Protocol_Packet_Data(pData->Data_a8, &pData->Length_u16, OBIS_DEV_VOL2, &sVout.mVol_u32, 2, 0xFD);    
    
    //----------Tan suat--------------------
    SV_Protocol_Packet_Data(pData->Data_a8, &pData->Length_u16, OBIS_FREQ_SEND, &sModemInfor.sFrequence.UnitMin_u16, 2, 0x00);
   
    //----------trang thai tiep diem --------------------
    SV_Protocol_Packet_Data(pData->Data_a8, &pData->Length_u16, OBIS_CONTACT_STATUS, &sWmVar.aINPUT_VAL[channel], 1, 0xAA);
       
    // caculator crc
    pData->Length_u16++;
	for (i = FistPos; i < (pData->Length_u16 - 1); i++)
		TempCrc ^= pData->Data_a8[i];

    pData->Data_a8[pData->Length_u16-1] = TempCrc;
#endif  // USING_APP_COMM  
    
    return  (pData->Length_u16- FistPos);
}


                    
        
/*
    Func: Log Data Event
        + Packet Event Data
        + Save to Flash or ExMem
*/

void AppWm_Log_Data_Event (uint8_t chann)
{
    uint8_t     aMessData[64] = {0};
    uint8_t     Length = 0;
    
    if (sRTC.year <= 20)
        return;
    
    Length = AppWm_Packet_Event (&aMessData[0], chann);


    Mem_Write_Data(sMemVar.Type_u8, _MEM_DATA_EVENT, 0, 
                      &aMessData[0], Length, sRecEvent.Size_u16);
}
             
                    
                    
uint8_t AppWm_Packet_Event (uint8_t *pData, uint8_t chann)
{
    uint16_t     length = 0;
    uint16_t	i = 0;
    uint8_t     TempCrc = 0;
    
    //----------------------- start send data below ---------------------
    //----------sTime--------------------
    SV_Protocol_Packet_Data(pData, &length, OBIS_TIME_DEVICE, &sRTC, 6, 0xAA);
    
    //Chac chan vao day da co 1 alarm
    for (i = 0; i < _ALARM_END;i++)
    {
        if (sWmVar.sAlarm[chann].aMARK_ALARM[i] == 1)
        {
            switch (i)
            {
                case _ALARM_FLOW_LOW:
                    SV_Protocol_Packet_Data(pData, &length, OBIS_AlARM_LOW_FLOW, &sWmVar.sAlarm[chann].StaFlowLow_u8, 1, 0xAA);
                    break;
                case _ALARM_FLOW_HIGH:
                    SV_Protocol_Packet_Data(pData, &length, OBIS_AlARM_HI_FLOW, &sWmVar.sAlarm[chann].StaFlowHi_u8, 1, 0xAA);
                    break;
                case _ALARM_PEAK_LOW:
                    SV_Protocol_Packet_Data(pData, &length, OBIS_AlARM_LOW_QUAN, &sWmVar.sAlarm[chann].StaQuanLow_u8, 1, 0xAA);
                    break;
                case _ALARM_PEAK_HIGH:
                    SV_Protocol_Packet_Data(pData, &length, OBIS_AlARM_HI_QUAN, &sWmVar.sAlarm[chann].StaQuanHi_u8, 1, 0xAA);
                    break;
                case _ALARM_VBAT_LOW:
                    SV_Protocol_Packet_Data(pData, &length, OBIS_AlARM_LOW_BAT, &sWmVar.sAlarm[chann].StaBatLow_u8, 1, 0xAA);
                    break;
                case _ALARM_DIR_CHANGE:
                    SV_Protocol_Packet_Data(pData, &length, OBIS_AlARM_DIR, &sWmVar.sAlarm[chann].StaDir_u8, 1, 0xAA);
                    break;
                case _ALARM_LOSS_DEVICE:
                    SV_Protocol_Packet_Data(pData, &length, OBIS_AlARM_LOST_DEV, &sWmVar.sAlarm[chann].StaLostDev_u8, 1, 0xAA);
                    break;
                case _ALARM_MAGN_DETECT:
                    SV_Protocol_Packet_Data(pData, &length, OBIS_AlARM_MAG_DET, &sWmVar.sAlarm[chann].StaMagne_u8, 1, 0xAA);
                    break;
                case _ALARM_HARD_RESET:
                    SV_Protocol_Packet_Data(pData, &length, OBIS_AlARM_HARD_RS, &sWmVar.sAlarm[chann].StaMagne_u8, 1, 0xAA);
                    break;     
                default:
                    break;   
            }
        }
        
        //Reset Alarm
        sWmVar.sAlarm[chann].aMARK_ALARM[i] = 0;
    }  
            
    // caculator crc
    length++;
	for (i = 0; i < (length - 1); i++)
		TempCrc ^= pData[i];

    pData[length-1] = TempCrc;
    
    return length;
}


int32_t AppWm_Cacul_Quantity (uint32_t PulseCur, uint32_t PulseOld)
{
    int32_t Qual = 0;
    
    if (PulseCur >=  PulseOld)
	{
		if ((PulseCur - PulseOld) < 0xFFFFFFAF)   //khong the quay nguoc dc 0x50 xung trong 10p va khong the quay tien 0xFFAF
			Qual = (PulseCur - PulseOld);  //binh thuong
		else
			Qual = - (0xFFFFFFFF - PulseCur + PulseOld + 1); //quay nguoc
	} else
	{
		if ((PulseOld - PulseCur) > 0x50)   //khong the quay nguoc dc 0x50 xung trong 10p
			Qual = (0xFFFFFFFF - PulseOld + PulseCur + 1);  //binh thuong
        else
			Qual = - (PulseOld - PulseCur);  // quay ngược
	}
    
    return Qual;
}



void AppWm_Init_Thresh_Measure (void)
{
#ifdef BOARD_QN_V5_0
    uint8_t     temp = 0;
    uint8_t 	aBuff[1600] = {0};

    temp = *(__IO uint8_t*) (ADDR_THRESH_MEAS);
    //Check Byte EMPTY
    if (temp != FLASH_BYTE_EMPTY) {
        OnchipFlashReadData(ADDR_THRESH_MEAS, &aBuff[0], 1600);
        UTIL_MEM_cpy(&sWmVar.aPRESSURE, &aBuff[1], sizeof(sWmVar.aPRESSURE));   
        UTIL_MEM_cpy(&sMeterThreshold, &aBuff[512], sizeof(sMeterThreshold));   
    } else {
        aBuff[0] = BYTE_WRITEN;
        UTIL_MEM_cpy(&aBuff[1], &sWmVar.aPRESSURE, sizeof(sWmVar.aPRESSURE));  
        UTIL_MEM_cpy(&aBuff[512], &sMeterThreshold, sizeof(sMeterThreshold)); 
        
        OnchipFlashPageErase(ADDR_THRESH_MEAS);
        OnchipFlashWriteData(ADDR_THRESH_MEAS, &aBuff[0], 1600);
    }
#endif
    
#ifdef BOARD_LC_V1_1
    uint8_t temp = 0;
    uint8_t  aBuff[1600] = {0};
    
    if (CAT24Mxx_Read_Array(CAT_ADDR_WM_CONFIG, aBuff, 1600) == true) {
        temp = aBuff[0];
        //Check Byte EMPTY
        if (temp == BYTE_WRITEN) {
            UTIL_MEM_cpy(&sWmVar.aPRESSURE, &aBuff[1], sizeof(sWmVar.aPRESSURE));   
            UTIL_MEM_cpy(&sMeterThreshold, &aBuff[512], sizeof(sMeterThreshold)); 
        }
    }
#endif
}


void AppWm_Save_Thresh_Measure (void)
{
#ifdef BOARD_QN_V5_0
    uint8_t aBuff[1600] = {0};

    aBuff[0] = BYTE_WRITEN;
    UTIL_MEM_cpy(&aBuff[1], &sWmVar.aPRESSURE, sizeof(sWmVar.aPRESSURE));  
    UTIL_MEM_cpy(&aBuff[512], &sMeterThreshold, sizeof(sMeterThreshold)); 
    
    OnchipFlashPageErase(ADDR_THRESH_MEAS);
    OnchipFlashWriteData(ADDR_THRESH_MEAS, &aBuff[0], 1600);
#endif  
    
    
#ifdef BOARD_LC_V1_1
    uint8_t aBuff[1600] = {0};

    aBuff[0] = BYTE_WRITEN;
    UTIL_MEM_cpy(&aBuff[1], &sWmVar.aPRESSURE, sizeof(sWmVar.aPRESSURE));  
    UTIL_MEM_cpy(&aBuff[512], &sMeterThreshold, sizeof(sMeterThreshold)); 
    
    CAT24Mxx_Write_Buff(CAT_ADDR_WM_CONFIG, aBuff, 1600);
#endif
}



void AppWm_Init_Press_Infor (void)
{
    AppWm_Init_Thresh_Measure();
}


void AppWm_Save_Press_Infor (void)
{
    AppWm_Save_Thresh_Measure();
}


void AppWm_Init_Level_Calib (void)
{

}

void AppWm_Save_Level_Calib (void)
{

}

/*
    Func: Doc xung luu trong flash
        + Doc byte danh dau (4 byte 0xAA)
        + Doc xung neu c� danh dau ( 4 byte) x 4 kenh

    Ex: 0xAA 0x08 0xAA 0xAA 0xAA 0xAA 01 02 03 04 ...
*/


void AppWm_Default_Sett_Pulse (void)
{
    for (uint8_t i = 0; i < MAX_CHANNEL; i++) {     
        sPulse[i].FactorInt_i16 = 1;
        sPulse[i].FactorDec_u8 = 0;
    }
}

void AppWm_Init_Pulse_Infor (void)
{   
    uint8_t temp = 0;
    uint8_t  aBuff[512] = {0};
    
#ifdef BOARD_QN_V5_0
    temp = *(__IO uint8_t*) (ADDR_METER_NUMBER);
    //Check Byte EMPTY
    if (temp != FLASH_BYTE_EMPTY) {
        OnchipFlashReadData(ADDR_METER_NUMBER, &aBuff[0], 512);
        UTIL_MEM_cpy(&sPulse, &aBuff[1], sizeof(sPulse));   
    }
#endif
    
#ifdef BOARD_LC_V1_1
    if (CAT24Mxx_Read_Array(CAT_ADDR_METER_NUMBER, aBuff, 512) == true) {
        //doc 64 byte tu eeprom
        temp = aBuff[0];
        //Check Byte EMPTY
        if (temp == BYTE_WRITEN) {
            UTIL_MEM_cpy(&sPulse, &aBuff[1], sizeof(sPulse));  
        }
    }
#endif
    
    for (uint8_t i = 0; i < MAX_CHANNEL; i++) {
        sPulse[i].IsOver5Pulse_u8 = false;
        sPulse[i].NumberInit_i64 = sPulse[i].Number_i64;
    }
}

uint8_t AppWm_Save_Pulse (void)
{
    uint8_t aBuff[512] = {0};
    uint8_t aBuff_read[512] = {0};
        
    AppWm_Get_Pulse();
    
    aBuff[0] = BYTE_WRITEN;
    UTIL_MEM_cpy(&aBuff[1], &sPulse, sizeof(sPulse));   
    
#ifdef BOARD_QN_V5_0
    OnchipFlashPageErase(ADDR_METER_NUMBER);
    OnchipFlashWriteData(ADDR_METER_NUMBER, &aBuff[0], 512);
    OnchipFlashReadData(ADDR_METER_NUMBER, &aBuff_read[0], 512);
    
    for (uint16_t i = 0; i < 512; i++) {
        if (aBuff_read[i] != aBuff[i]) {
            return false;
        }
    }
#endif   
    
#ifdef BOARD_LC_V1_1
    if (CAT24Mxx_Write_Buff(CAT_ADDR_METER_NUMBER, aBuff, 512) == false) {
        return false;
    } 
#endif

    return true; 
}




uint8_t AppWm_UnMark_Init_Pulse (void)
{
    return true;
}


static void OnTimerLevelPowerOn(void *context)
{
	fevent_active(sEventAppWM, _EVENT_RS485_MODBUS);
}

static void OnTimerRs485_2(void *context)
{
	fevent_active(sEventAppWM, _EVENT_RS485_2_MODBUS);
}



static void OnTimer12VPowerUp(void *context)
{
    sWmVar.IrqPowUp_u8 = pending;
    fevent_active(sEventAppWM, _EVENT_CHECK_MODE);
}


void LED_Toggle (Led_TypeDef Led)
{
    HAL_GPIO_TogglePin(LED_PORT[Led], LED_PIN[Led]);
}


void LED_On (Led_TypeDef Led)
{
    HAL_GPIO_WritePin(LED_PORT[Led], LED_PIN[Led], GPIO_PIN_SET);
}



void LED_Off (Led_TypeDef Led)
{
    HAL_GPIO_WritePin(LED_PORT[Led], LED_PIN[Led], GPIO_PIN_RESET);
}


/*
    Dat fun nay sau Init Pulse from flash
*/
void AppWm_Button_Default_Device (void)
{
    uint32_t DetectButton_u32 = 0;
     //Doc chan PA4 de xem khoi tao xung ve 0?
    if (BUTTON_DEFAULT == true)
    {
        while (DetectButton_u32++ < MAX_BUTTON_DEF_DEV )       //1M
        {
            if (BUTTON_DEFAULT == false)
                break;
        }
    }

    //Neu DetectButton_u32 >  : Nut nhan duoc giu trong khoang do.
    if (DetectButton_u32 >= MAX_BUTTON_DEF_DEV)
    {
        AppWm_Default_Pulse();
        sModemInfor.cReset_u16 = 0;
    }
}

void AppWm_Default_Pulse (void)
{
    if (sWmVar.pPack_Alarm_Str != NULL)
            sWmVar.pPack_Alarm_Str ("u_app_wm: default device!\r\n");  
        
    sWmVar.rDefault_u8 = true;
        
    for (uint8_t i = 0; i < MAX_CHANNEL; i++) {    
        sPulse[i].Number_i64 = 0;
        sPulse[i].Start_lf = 0;   
        
        sPulse[i].NumberInit_i64 = sPulse[i].Number_i64;   
    }
    
    AppWm_Save_Pulse();

    sRecTSVH.iSend_u16 = 0;
    sRecTSVH.iSave_u16 = 0;
    sRecEvent.iSend_u16 = 0;
    sRecEvent.iSave_u16 = 0;
    sRecLog.iSend_u16 = 0;
    sRecLog.iSave_u16 = 0;
    sRecGPS.iSend_u16 = 0;
    sRecGPS.iSave_u16 = 0;
    
    Mem_Save_Index_Rec();
}


/*Func Callback serial*/
#ifdef USING_AT_CONFIG

void AppWm_SER_Set_Default_Pulse(sData *strRecei, uint16_t Pos)
{
    for (uint8_t i = 0; i < MAX_CHANNEL; i++) {
        sPulse[i].Number_i64 = 0;
    }
    
    sWmVar.pRespond_Str(PortConfig, "\r\nreset pulse ok!", 0);
}

void AppWm_SER_Set_Thresh_Flow(sData *str_Receiv, uint16_t Pos)
{
    uint16_t    Posfix = Pos;
    uint16_t    TemLowThresh = 0;
    uint16_t    TemHighThresh = 0;
    uint8_t chann = (uint8_t) UTIL_Get_Num_From_Str (str_Receiv, &Posfix);
    
    TemLowThresh = (uint16_t) UTIL_Get_Num_From_Str (str_Receiv, &Posfix);
    TemHighThresh = (uint16_t) UTIL_Get_Num_From_Str (str_Receiv, &Posfix);
    
    if (chann < MAX_CHANNEL)
    {
//        if (chann == 0) {
//            for (uint8_t i = 0; i < MAX_CHANNEL; i++) {
//                sMeterThreshold[i].FlowLow  = TemLowThresh;
//                sMeterThreshold[i].FlowHigh = TemHighThresh;
//            }
//        } else {
//            if ( (TemLowThresh != 0xFFFF) && (TemHighThresh != 0xFFFF) ) {
//                sMeterThreshold[chann - 1].FlowLow  = TemLowThresh;
//                sMeterThreshold[chann - 1].FlowHigh = TemHighThresh;
//            }        
//        }
        
        AppWm_Save_Thresh_Measure();
            
        sWmVar.pRespond_Str(PortConfig, "\r\nOK\r\n", 0);
    }
    
    sWmVar.pRespond_Str(PortConfig, "\r\nOVER VALUE U16\r\n", 0);
}

void AppWm_SER_Get_Thresh_Flow(sData *str_Receiv, uint16_t Pos)
{
    char aBuff[256] = {0};
    
    sprintf(aBuff + strlen(aBuff), "\r\nflowthresh:\r\n");
    
//    for (uint8_t i = 0; i < MAX_CHANNEL; i++) {
//        sprintf(aBuff + strlen(aBuff), "\r\nchann %d: %f, %f\r\n", i + 1,
//                                                            sMeterThreshold[i].FlowLow, sMeterThreshold[i].FlowHigh);
//    }
    
    sWmVar.pRespond_Str(PortConfig, aBuff, 0);
}


void AppWm_SER_Set_Thresh_Quan(sData *str_Receiv, uint16_t Pos)
{
    uint16_t    Posfix = Pos;
    uint16_t    TemLowThresh = 0;
    uint16_t    TemHighThresh = 0;
    uint8_t chann = (uint8_t) UTIL_Get_Num_From_Str (str_Receiv, &Posfix);
    
    TemLowThresh = (uint16_t) UTIL_Get_Num_From_Str (str_Receiv, &Posfix);
    TemHighThresh = (uint16_t) UTIL_Get_Num_From_Str (str_Receiv, &Posfix);
    
    if (chann < MAX_CHANNEL)
    {
//        if (chann == 0) {
//            for (uint8_t i = 0; i < MAX_CHANNEL; i++) {
//                sMeterThreshold[i].PeakLow  = TemLowThresh;
//                sMeterThreshold[i].PeakHigh = TemHighThresh;
//            }
//        } else {
//            if ( (TemLowThresh != 0xFFFF) && (TemHighThresh != 0xFFFF) ) {
//                sMeterThreshold[chann - 1].PeakLow  = TemLowThresh;
//                sMeterThreshold[chann - 1].PeakHigh = TemHighThresh;
//            }        
//        }
        
        AppWm_Save_Thresh_Measure();
            
        sWmVar.pRespond_Str(PortConfig, "\r\nOK\r\n", 0);
    }
    
    
    sWmVar.pRespond_Str(PortConfig, "\r\nOVER VALUE U16\r\n", 0);
}

void AppWm_SER_Get_Thresh_Quan(sData *str_Receiv, uint16_t Pos)
{
    char aBuff[256] = {0};
    
    sprintf(aBuff + strlen(aBuff), "\r\nflowthresh:\r\n");
    
//    for (uint8_t i = 0; i < MAX_CHANNEL; i++) {
//        sprintf(aBuff + strlen(aBuff), "\r\nchann %d: %f, %f\r\n", i + 1,
//                                                            sMeterThreshold[i].PeakLow, sMeterThreshold[i].PeakHigh);
//    }
    
    sWmVar.pRespond_Str(PortConfig, aBuff, 0);
}


void AppWm_SER_Set_Thresh_Bat(sData *str_Receiv, uint16_t Pos)
{
	uint16_t    Posfix = Pos;
    uint16_t    ValTemp = 0;
	  
    ValTemp = (uint16_t) UTIL_Get_Num_From_Str(str_Receiv, &Posfix);
    
    if (ValTemp != 0xFFFF) 
    {
        for (uint8_t i = 0; i < MAX_CHANNEL; i++) {
            sMeterThreshold[i].LowBatery  = ValTemp;
        }
        
        AppWm_Save_Thresh_Measure();
        
        sWmVar.pRespond_Str(PortConfig, "\r\nOK\r\n", 0);
    }
    
    sWmVar.pRespond_Str(PortConfig, "\r\nOVER VALUE U16\r\n", 0);
}

void AppWm_SER_Get_Thresh_Bat(sData *str_Receiv, uint16_t Pos)
{
    char aData[32] = {0};
    
    sprintf(aData, "\r\nBatthresh: %d\r\n", sMeterThreshold[0].LowBatery);
    
    sWmVar.pRespond_Str(PortConfig, aData, 0);
}
    

void AppWm_SER_Set_Pulse(sData *str_Receiv, uint16_t Pos)
{
    uint16_t Posfix = Pos;

    uint8_t chann = (uint8_t ) UTIL_Get_Num_From_Str(str_Receiv, &Posfix);
    uint32_t value = (uint32_t ) UTIL_Get_Num_From_Str(str_Receiv, &Posfix);
	//
    if (chann <= MAX_CHANNEL) {
        if (chann == 0) {
        #ifdef LPTIM_ENCODER_MODE 
            sPulse[0].Number_i64   = value * 4;
        #else   
            sPulse[0].Number_i64   = value;
        #endif
            
            if (MAX_CHANNEL >= 2) {
            #ifdef LPTIM2_ENCODER_MODE 
                sPulse[1].Number_i64 = value * 4;
            #else   
                sPulse[1].Number_i64 = value;
            #endif
            }
              
            if (MAX_CHANNEL > 2) {
                for (uint8_t i = 2; i < MAX_CHANNEL; i++) {
                    sPulse[i].Number_i64 = value;
                }
            }
            
            for (uint8_t i = 0; i < MAX_CHANNEL; i++) {
                sPulse[i].NumberInit_i64 = sPulse[i].Number_i64;
            }
        } else {
            if ( (chann == 1) || (chann == 2) ) {
            #ifdef LPTIM_ENCODER_MODE 
                sPulse[chann - 1].Number_i64 = value * 4;
            #else   
                sPulse[chann - 1].Number_i64 = value;
            #endif
                sPulse[chann - 1].NumberInit_i64 = sPulse[chann - 1].Number_i64;
            } else {
                sPulse[chann - 1].Number_i64 = value;
                sPulse[chann - 1].NumberInit_i64 = sPulse[chann - 1].Number_i64;
            }
        }
        
        AppWm_Save_Pulse();
        sWmVar.pRespond_Str(PortConfig, "\r\nOK\r\n", 0);
        
        return;
    }

    sWmVar.pRespond_Str(PortConfig, "\r\nERROR\r\n", 0); 
}


void AppWm_SER_Get_Pulse(sData *str_Receiv, uint16_t Pos)
{
    char aData[64] = {0};
    uint32_t aVAL_TEMP[MAX_CHANNEL] = {0};
    
    AppWm_Get_Pulse();
    
#ifdef LPTIM_ENCODER_MODE 
    aVAL_TEMP[0] = sPulse[0].Number_i64 / 4;
#else   
    aVAL_TEMP[0] = sPulse[0].Number_i64;
#endif
        
    if (MAX_CHANNEL >= 2) {
    #ifdef LPTIM2_ENCODER_MODE 
        aVAL_TEMP[1] = sPulse[1].Number_i64 / 4;
    #else   
        aVAL_TEMP[1] = sPulse[1].Number_i64;
    #endif
    }

    if (MAX_CHANNEL > 2) {
        for (uint8_t i = 2; i < MAX_CHANNEL; i++) {
            aVAL_TEMP[i] = sPulse[i].Number_i64;
        }
    }
    
    sprintf(aData, "\r\npulse: ");
    for (uint8_t i = 0; i < MAX_CHANNEL; i++) {
        sprintf(aData + strlen(aData), "%d, ", aVAL_TEMP[i]);
    }
    
    
    sWmVar.pRespond_Str(PortConfig, aData, 0);
}
    

void AppWm_SER_Set_Pulse_Factor(sData *str_Receiv, uint16_t Pos)
{
    uint16_t Posfix = Pos;
    char *sfactor;
    uint8_t TempFactor = 0;
    int16_t TempFacInt = 0;
    
    uint8_t chann = (uint8_t ) UTIL_Get_Num_From_Str(str_Receiv, &Posfix);
    sfactor = (char *) (str_Receiv->Data_a8 + Posfix);
    
    TempFactor = 0xFF - UTIL_Count_Decimal_Places(sfactor) + 1;
    TempFacInt = (int16_t) ( (float) (UtilStringToFloat(sfactor) * Convert_Scale(0xFF - TempFactor + 1) ) ) ; 
    
    
    if (chann <= MAX_CHANNEL) {
        if (chann == 0) {
            for (uint8_t i = 0; i < MAX_CHANNEL; i++) {
                sPulse[i].FactorDec_u8 = TempFactor;
                sPulse[i].FactorInt_i16 = TempFacInt;
            }
        } else {
            sPulse[chann - 1].FactorDec_u8 = TempFactor;
            sPulse[chann - 1].FactorInt_i16 = TempFacInt;
        }
        
        AppWm_Save_Pulse();
        //cap nhat len server
        sMessage.aMESS_PENDING[DATA_HANDSHAKE] = TRUE;
        sWmVar.pRespond_Str(PortConfig, "\r\nOK\r\n", 0);
        
        return;
    }

    sWmVar.pRespond_Str(PortConfig, "\r\nERROR\r\n", 0);  
}

void AppWm_SER_Get_Pulse_Factor(sData *str_Receiv, uint16_t Pos)
{
    char aData[128] = {0};
    
    sprintf(aData + strlen(aData), "\r\npulse factor: ");
    
    for (uint8_t i = 0; i < MAX_CHANNEL; i++) {
        UtilIntToStringWithScale (sPulse[i].FactorInt_i16, aData + strlen(aData), 0xFF - sPulse[i].FactorDec_u8 + 1);
        sprintf(aData + strlen(aData), ", ");
    }
    
    sprintf(aData + strlen(aData), "\r\n");
    
    sWmVar.pRespond_Str(PortConfig, aData, 0);
}



void AppWm_SER_Set_Pulse_Start(sData *str_Receiv, uint16_t Pos)
{
    uint16_t Posfix = Pos;
    char *strStart;
    
    uint8_t chann = (uint8_t ) UTIL_Get_Num_From_Str(str_Receiv, &Posfix);
    strStart = (char *) (str_Receiv->Data_a8 + Posfix);

    if (chann <= MAX_CHANNEL) {
        if (chann == 0) {
            for (uint8_t i = 0; i < MAX_CHANNEL; i++) {
                sPulse[i].Start_lf = UtilStringToDouble(strStart);
            }
        } else {
            sPulse[chann - 1].Start_lf = UtilStringToDouble(strStart) ;
        }
        
        AppWm_Save_Pulse();
        //cap nhat len server
        sMessage.aMESS_PENDING[DATA_HANDSHAKE] = TRUE;
        sWmVar.pRespond_Str(PortConfig, "\r\nOK\r\n", 0);
        
        return;
    }

    sWmVar.pRespond_Str(PortConfig, "\r\nERROR\r\n", 0);
}

void AppWm_SER_Get_Pulse_Start(sData *str_Receiv, uint16_t Pos)
{
    char aData[256] = {0};
    
    sprintf(aData + strlen(aData), "\r\npulse start: ");
    
    for (uint8_t i = 0; i < MAX_CHANNEL; i++) {
        sprintf(aData + strlen(aData), "%.10g", sPulse[i].Start_lf);
    }
    
    sprintf(aData + strlen(aData), "\r\n");
    
    sWmVar.pRespond_Str(PortConfig, aData, 0);
}



void AppWm_SER_Set_Level_Calib(sData *str_Receiv, uint16_t Pos)
{
//    uint8_t Sign = 0;
//    int16_t Value = 0;
//    uint16_t TempVal = 0;
//        
//    if (*(strRecei->Data_a8 + Pos++) == '-')
//        Sign = 1;
//    
//    TempVal = (uint16_t) UTIL_Get_Num_From_Str(strRecei, &Pos);
//        
//    if (TempVal != 0xFFFF)
//    {
//        if (Sign == 1)
//            Value = 0 - TempVal;
//        
//        sWmVar.pRespond_Str(PortConfig, "\r\nOK\r\n", 0);
//    } else
//        sWmVar.pRespond_Str(PortConfig, "\r\nERROR", 0);    
}

void AppWm_SER_Get_Level_Calib(sData *str_Receiv, uint16_t Pos)
{

}

void AppWm_SER_Set_Level_Thresh(sData *str_Receiv, uint16_t Pos)
{
    
    sWmVar.pRespond_Str(PortConfig, "\r\nNOT SUPPORT", 0);   
}

void AppWm_SER_Get_Level_Thresh(sData *str_Receiv, uint16_t Pos)
{
    sWmVar.pRespond_Str(PortConfig, "\r\nNOT SUPPORT", 0);  
}


void AppWm_SER_Set_Num_Channel(sData *str_Receiv, uint16_t Pos)
{
    uint16_t Temp = (uint16_t) UTIL_Get_Num_From_Str(str_Receiv, &Pos);
    
    if (Temp != 0xFFFF)
    {
        sWmVar.nChannel_u8 = Temp;
        AppWm_UnMark_Init_Pulse();
        
        sWmVar.pRespond_Str(PortConfig, "\r\nOK\r\n", 0);
    }
    
    sWmVar.pRespond_Str(PortConfig, "\r\nERROR", 0);  
}


void AppWm_SER_Get_Num_Channel(sData *str_Receiv, uint16_t Pos)
{
    char aData[16] = {0};
    
    sprintf(aData, "\r\n%d\r\n", sWmVar.nChannel_u8);
    
    sWmVar.pRespond_Str(PortConfig, aData, 0);
}



void AppWm_SER_Set_Modbus_Infor(sData *pData, uint16_t Pos)
{
    uint8_t Index = 0;
    uint8_t Mtype = 0xFF;
    uint8_t SlaveId = 0xFF;
    uint8_t Total = (uint8_t) UTIL_Get_Num_From_Str(pData, &Pos);
    
    if (Total <= MAX_SLAVE_MODBUS) 
    {
        sWmDigVar.nModbus_u8 = Total;
        
        while (Index < Total)
        {
            Mtype = (uint8_t) UTIL_Get_Num_From_Str(pData, &Pos);
            SlaveId = (uint8_t) UTIL_Get_Num_From_Str(pData, &Pos);
            
            if ((Mtype != 0xFF) && (SlaveId != 0xFF)) {
                sWmDigVar.sModbInfor[Index].MType_u8 = Mtype;
                sWmDigVar.sModbInfor[Index].SlaveId_u8 = SlaveId;
            } else {
                break;
            }
            
            Index++;
        }
        
        //Luu lai
        AppWm_Save_WM_Dig_Infor();
        
        sWmVar.pRespond_Str(PortConfig, "\r\nOK\r\n", 0);
    } else
    {
        sWmVar.pRespond_Str(PortConfig, "\r\nERROR", 0); 
    }
}


void AppWm_SER_Get_Modbus_Infor(sData *str_Receiv, uint16_t Pos)
{
    char aData[128] = {0};
    
    sprintf(aData, "%d", sWmDigVar.nModbus_u8 );
    
    for (uint8_t i = 0; i < sWmDigVar.nModbus_u8; i++)
    {
        sprintf(aData+strlen(aData), ",%d,%d", sWmDigVar.sModbInfor[i].MType_u8, sWmDigVar.sModbInfor[i].SlaveId_u8);
    }
    
    sWmVar.pRespond_Str(PortConfig, aData, 0);
}


void AppWm_SER_Set_Pressure_Infor(sData *str_Receiv, uint16_t Pos)
{
    uint8_t Channel = 0xFF;
    char aBuff[10] = {0};
    char* start = (char *) (str_Receiv->Data_a8 + Pos);
    char* end = NULL;
    float tempf = 0;
    sPressureLinearInter    sPressSett = {0};
    
    Channel = (uint8_t) strtol(start, &end, 10); start = end + 1;
    sPressSett.InMin_u16 = (uint16_t) strtol(start, &end, 10); start = end + 1;
    sPressSett.InMax_u16 = (uint16_t) strtol(start, &end, 10); start = end + 1;
    sPressSett.InUnit_u8 = (uint16_t) strtol(start, &end, 10); start = end + 1;
    sPressSett.OutMin_u16 = (uint16_t) strtol(start, &end, 10); start = end + 1;
    sPressSett.OutMax_u16 = (uint16_t) strtol(start, &end, 10); start = end + 1;
    sPressSett.OutUnit_u8 = (uint16_t) strtol(start, &end, 10); start = end + 1;
    
    tempf = strtof(start, &end);
    if ((end - start) > 10) {
        sWmVar.pRespond_Str(PortConfig, "ERROR", 0);  
        return; 
    }
    
    UTIL_MEM_cpy(aBuff, start, end - start);
    sPressSett.FactorDec_u8 = 0xFF - UTIL_Count_Decimal_Places(aBuff) + 1;
    sPressSett.Factor_u16 = (uint64_t) ( tempf * Convert_Scale(0xFF - sPressSett.FactorDec_u8 + 1) ); 
    
    start = end + 1;
    sPressSett.Type_u8 = (uint16_t) strtol(start, &end, 10); start = end;
    
    if (AppWm_Set_Pressure_Infor(Channel, sPressSett) == true) {
        sWmVar.pRespond_Str(PortConfig, "OK", 0);
    } else {
        sWmVar.pRespond_Str(PortConfig, "ERROR", 0);  
    }
}

uint8_t AppWm_Set_Pressure_Infor (uint8_t channel, sPressureLinearInter press_sett)
{
    if ( (channel <= MAX_CHANNEL ) && (press_sett.InUnit_u8 < 6) && (press_sett.OutUnit_u8 < 6)
        && ((press_sett.FactorDec_u8 >= 0xFC) || (press_sett.FactorDec_u8 <= 2) ) && (press_sett.Type_u8 <= 2) )
    {
        if (channel == 0)
        {
            //set tat ca nhu nhau
            for (uint8_t i = 0; i < MAX_CHANNEL; i++)
            {
                sWmVar.aPRESSURE[i].sLinearInter.InMin_u16 = press_sett.InMin_u16;
                sWmVar.aPRESSURE[i].sLinearInter.InMax_u16 = press_sett.InMax_u16;
                sWmVar.aPRESSURE[i].sLinearInter.InUnit_u8 = press_sett.InUnit_u8;
                sWmVar.aPRESSURE[i].sLinearInter.OutMin_u16 = press_sett.OutMin_u16;
                sWmVar.aPRESSURE[i].sLinearInter.OutMax_u16 = press_sett.OutMax_u16;
                sWmVar.aPRESSURE[i].sLinearInter.OutUnit_u8 = press_sett.OutUnit_u8;
                sWmVar.aPRESSURE[i].sLinearInter.Factor_u16 = press_sett.Factor_u16;
                sWmVar.aPRESSURE[i].sLinearInter.FactorDec_u8 = press_sett.FactorDec_u8;
                sWmVar.aPRESSURE[i].sLinearInter.Type_u8 = press_sett.Type_u8;
            }
        } else
        {
            sWmVar.aPRESSURE[channel - 1].sLinearInter.InMin_u16 = press_sett.InMin_u16;
            sWmVar.aPRESSURE[channel - 1].sLinearInter.InMax_u16 = press_sett.InMax_u16;
            sWmVar.aPRESSURE[channel - 1].sLinearInter.InUnit_u8 = press_sett.InUnit_u8;
            sWmVar.aPRESSURE[channel - 1].sLinearInter.OutMin_u16 = press_sett.OutMin_u16;
            sWmVar.aPRESSURE[channel - 1].sLinearInter.OutMax_u16 = press_sett.OutMax_u16;
            sWmVar.aPRESSURE[channel - 1].sLinearInter.OutUnit_u8 = press_sett.OutUnit_u8;
            sWmVar.aPRESSURE[channel - 1].sLinearInter.Factor_u16 = press_sett.Factor_u16;
            sWmVar.aPRESSURE[channel - 1].sLinearInter.FactorDec_u8 = press_sett.FactorDec_u8;
            sWmVar.aPRESSURE[channel - 1].sLinearInter.Type_u8 = press_sett.Type_u8;
        }
        
        AppWm_Save_Press_Infor();
        
        return true;
    }
    
    return false;
}


void AppWm_SER_Get_Pressure_Infor(sData *str_Receiv, uint16_t Pos)
{
    char aData[1024] = {0};

    for (uint8_t i = 0; i < MAX_CHANNEL; i++)
    {
        
        sprintf(aData + strlen(aData), "%d. %s : %d - %d (%s) <-> %d - %d (%s) \r\n hs: %.*f;\r\n",
                                        i + 1, AnalogType[sWmVar.aPRESSURE[i].sLinearInter.Type_u8],
                                       sWmVar.aPRESSURE[i].sLinearInter.InMin_u16, sWmVar.aPRESSURE[i].sLinearInter.InMax_u16, aUnitWm[sWmVar.aPRESSURE[i].sLinearInter.InUnit_u8],
                                       sWmVar.aPRESSURE[i].sLinearInter.OutMin_u16, sWmVar.aPRESSURE[i].sLinearInter.OutMax_u16, aUnitWm[sWmVar.aPRESSURE[i].sLinearInter.OutUnit_u8],
                                       (uint8_t ) (0xFF - sWmVar.aPRESSURE[i].sLinearInter.FactorDec_u8 + 1),
                                       sWmVar.aPRESSURE[i].sLinearInter.Factor_u16 * Convert_Scale(sWmVar.aPRESSURE[i].sLinearInter.FactorDec_u8) );
    }
    
    sWmVar.pRespond_Str(PortConfig, aData, 0); 
}


void AppWm_SER_Get_Pressure_Val(sData *str_Receiv, uint16_t Pos)
{
    char aData[512] = {0};

    for (uint8_t i = 0; i < MAX_CHANNEL; i++)
    {
        if  ( (sWmVar.aPRESSURE[i].sLinearInter.Type_u8 <= 2) && 
            (sWmVar.aPRESSURE[i].sLinearInter.OutUnit_u8 < 5) ) {
                    
            sprintf(aData + strlen(aData), "%d: %s: %d (%s)\r\n", i + 1, 
                                            AnalogType[sWmVar.aPRESSURE[i].sLinearInter.Type_u8],
                                            sWmVar.aPRESSURE[i].Val_i32 / 1000,
                                            aUnitWm[sWmVar.aPRESSURE[i].sLinearInter.OutUnit_u8]);
            }
    }
    
    sWmVar.pRespond_Str(PortConfig, aData, 0); 
}

void AppWm_SER_Set_Pressure_Calib(sData *str_Receiv, uint16_t Pos)
{
    //doc ap suat va luu lai
    sWmVar.rPressCalib_u8 = true;
    sWmVar.rIntan_u8 = true;
    fevent_active(sEventAppWM, _EVENT_ENTRY_WM);
    
    sWmVar.pRespond_Str(PortConfig, "OK", 0);
}

void AppWm_SER_Get_Pressure_Calib(sData *str_Receiv, uint16_t Pos)
{
    char aData[128] = {0};

    for (uint8_t i = 0; i < MAX_CHANNEL; i++) {
        sprintf(aData + strlen(aData), "%d: %d (mV)\r\n", i + 1, sWmVar.aPRESSURE[i].Calib_i16);
    }
    
    sWmVar.pRespond_Str(PortConfig, aData, 0); 
}


/*
Func: Set level infor //at+level=1,3000,2860
    + channel
    + level wire (cm)
    + level static (cm)
*/

void AppWm_SER_Set_Level_Infor(sData *str_Receiv, uint16_t Pos)
{

}


void AppWm_SER_Get_Level_Infor(sData *str_Receiv, uint16_t Pos)
{

}


void AppWm_SER_Set_TNMT_Infor(sData *pData, uint16_t Pos)
{
    //cat ra: 0,MaTinh|KyHieuCongTrinh|KyHieuTram|ThongSoDo
    uint8_t chann = (uint8_t ) UTIL_Get_Num_From_Str(pData, &Pos);
    char *pSource = (char *) (pData->Data_a8 + Pos);
    char *pFind = NULL;
    uint8_t len = 0, Index = 0, type = 0;
    sChannelInformation sInfor = {0};
    
    if (chann == 0) {
        //xoa het cac truong cu
        UTIL_MEM_set(&sWmVar.sChannInfor, 0, sizeof(sWmVar.sChannInfor));
        sWmVar.pRespond_Str(PortConfig, "OK", 0);
        AppWm_Save_TNMT_Infor();
        return;
    }
    
    if ( chann <= (MAX_CHANNEL + MAX_SLAVE_MODBUS) ) {
        pFind = strchr(pSource, '|');
        if (pFind != NULL) {
            len = pFind - pSource;
            strncpy(sInfor.MA_TINH, pSource, len);
            
            //tim ma CTrinh
            pSource = pFind + 1;
            pFind = strchr(pSource, '|');
            if (pFind != NULL) {
                len = pFind - pSource;
                strncpy(sInfor.MA_CTRINH, pSource, len);
                
                //tim ma tram
                pSource = pFind + 1;
                pFind = strchr(pSource, '|');
                if (pFind != NULL) {
                    len = pFind - pSource;
                    strncpy(sInfor.MA_TRAM, pSource, len);
                    
                    //tim param
                    Pos = (uint8_t *) pFind - pData->Data_a8;
                    sInfor.nParam_u8 = (uint8_t) UTIL_Get_Num_From_Str(pData, &Pos);
                    
                    if (sInfor.nParam_u8 <= MAX_PARAM_TNMT) {
                        while (Index < sInfor.nParam_u8)
                        {
                            type = (uint8_t) UTIL_Get_Num_From_Str(pData, &Pos);
                            if ( (type == 0xFF) && (type < MAX_PARAM_TYPE) ) {
                                sWmVar.pRespond_Str(PortConfig, "ERROR", 0);
                                return;
                            }
                            
                            sInfor.aPARAM[Index++] = type;
                              
                            if (Index >= MAX_PARAM_TNMT) {
                                break;
                            }
                        }
                        
                        sWmVar.pRespond_Str(PortConfig, "OK", 0);
                        //luu lai
                        UTIL_MEM_set(&sWmVar.sChannInfor[chann - 1], 0, sizeof(sWmVar.sChannInfor[chann- 1]));
                        UTIL_MEM_cpy(&sWmVar.sChannInfor[chann - 1], &sInfor, sizeof(sInfor));
                        AppWm_Save_TNMT_Infor();
                        return;
                    }
                }
            } 
        }
    }

    sWmVar.pRespond_Str(PortConfig, "ERROR", 0);
}


void AppWm_SER_Get_TNMT_Infor(sData *pData, uint16_t Pos)
{
    char aData[256] = {0};

    for (uint8_t i = 0; i < (MAX_CHANNEL + MAX_SLAVE_MODBUS); i++) {
        sprintf(aData + strlen(aData), "%d,%s|%s|%s|%d", i, sWmVar.sChannInfor[i].MA_TINH, sWmVar.sChannInfor[i].MA_CTRINH, 
                                                 sWmVar.sChannInfor[i].MA_TRAM, sWmVar.sChannInfor[i].nParam_u8);
        
        for (uint8_t count = 0; count < sWmVar.sChannInfor[i].nParam_u8; count++) {
            sprintf(aData + strlen(aData), ",%d", sWmVar.sChannInfor[i].aPARAM[count]);
        }
        
        sprintf(aData + strlen(aData), "\r\n");
    }
    
    
    sWmVar.pRespond_Str(PortConfig, aData, 0);

}
  
void AppWm_SER_Set_TNMT_Pack_M(sData *pData, uint16_t Pos)
{
    uint8_t mode = (uint8_t ) UTIL_Get_Num_From_Str(pData, &Pos);
    if (mode != 0xFF) {
        sWmVar.ModePacket_u8 = MIN(mode, MAX_PACKET_MODE);
        sWmVar.pRespond_Str(PortConfig, "OK", 0);
        AppWm_Save_TNMT_Infor();
        return;
    }
    
    sWmVar.pRespond_Str(PortConfig, "ERROR", 0);
}
  

void AppWm_SER_Get_TNMT_Pack_M(sData *pData, uint16_t Pos)
{
    char aData[8] = {0};

    sprintf(aData + strlen(aData), "%d\r\n", sWmVar.ModePacket_u8);
    
    sWmVar.pRespond_Str(PortConfig, aData, 0);
}
  





#endif

    


void AppWm_Packet_Modbus (void)
{
    uint8_t aTEMP_PAYLOAD[512] = {0};
    sData pData = {aTEMP_PAYLOAD, 0};
    uint16_t i = 0, a = 0, posindex = 0, length = 0;
    uint8_t TempCrc = 0;// caculator crc
    uint8_t count = 0;
    static uint16_t size_pack = 0;
    
    if (sRTC.year <= 20)
        return;

    for (i = 0; i < sWmDigVar.nModbus_u8; i++)
    {
        if (sWmDigVar.sModbDevData[i].Status_u8 == true)
        {
            posindex = pData.Length_u16;
            pData.Length_u16 += 2;
            
            length = AppWm_Packet_Mod_Channel(&pData, i);
            
            *(pData.Data_a8 + posindex) = count + 1;
            *(pData.Data_a8 + posindex + 1) = length;
        
            if (count == 0) {
                size_pack = pData.Length_u16;
            }
            count++;
        }
        
        //gop MAX_PACKET_SAVE ban tin voi nhau. hoac cuoi cung
        if ( (count > 0) &&
             ( ( (size_pack + pData.Length_u16 + 5) > sRecTSVH.Size_u16 ) ||  ( (i + 1) >= sWmDigVar.nModbus_u8 ) ) )
        {
            count = 0;
      
            pData.Length_u16++;
            for (a = 0; a < (pData.Length_u16 - 1); a++)
                TempCrc ^= pData.Data_a8[a];

            pData.Data_a8[pData.Length_u16-1] = TempCrc;
            
            Mem_Write_Data(sMemVar.Type_u8, _MEM_DATA_TSVH, 0, 
                      pData.Data_a8, pData.Length_u16, sRecTSVH.Size_u16);
            
            //reset buff 
            Reset_Buff(&pData);
            TempCrc = 0;
        }
    }
}



uint8_t AppWm_Packet_Mod_Channel (sData *pData, uint8_t chann)
{
    uint16_t lastlength = pData->Length_u16, i = 0;
    uint8_t TempCrc = 0;
    uint8_t strChanel[4] = {"0005"};
   
    strChanel[3] = chann + MAX_CHANNEL + 0x31;
    
    SV_Protocol_Packet_Data(pData->Data_a8, &pData->Length_u16, OBIS_TIME_DEVICE, &sRTC, 6, 0xAA);
    SV_Protocol_Packet_Data(pData->Data_a8, &pData->Length_u16, OBIS_SERI_SENSOR, &strChanel, 4, 0xAA);
       
    //----------Dien ap Pin--------------------
    SV_Protocol_Packet_Data(pData->Data_a8, &pData->Length_u16, OBIS_DEV_VOL1, &sBattery.mVol_u32, 2, 0xFD);
    //----------Dien ap Pin--------------------
    SV_Protocol_Packet_Data(pData->Data_a8, &pData->Length_u16, OBIS_DEV_VOL2, &sVout.mVol_u32, 2, 0xFD);   
        //----------Cuong do song--------------------
#ifdef USING_APP_SIM
    SV_Protocol_Packet_Data(pData->Data_a8, &pData->Length_u16, OBIS_RSSI_1, &sSimCommInfor.RSSI_u8, 1, 0x00);
#endif
    //----------Tan suat--------------------
    SV_Protocol_Packet_Data(pData->Data_a8, &pData->Length_u16, OBIS_FREQ_SEND, &sModemInfor.sFrequence.UnitMin_u16, 2, 0x00);
   
    /*
        Them data phan wm digital
    */
    WM_DIG_Packet_Mess(pData, chann);

    // caculator crc
    pData->Length_u16++;
	for (i = 0; i < (pData->Length_u16 - 1); i++)
		TempCrc ^= pData->Data_a8[i];

    pData->Data_a8[pData->Length_u16-1] = TempCrc;
    
    return (pData->Length_u16 - lastlength);
}




void AppWm_Init_WM_Dig_Infor (void)
{
    uint8_t temp = 0;
    uint8_t  aBuff[64] = {0};
    
#ifdef BOARD_QN_V5_0
    temp = *(__IO uint8_t*) (ADDR_MODBUS_INFOR);
    //Check Byte EMPTY
    if (temp != FLASH_BYTE_EMPTY) {
        OnchipFlashReadData(ADDR_MODBUS_INFOR, &aBuff[0], 64);
        UTIL_MEM_cpy(&sWmDigVar.sModbInfor, &aBuff[1], sizeof(sWmDigVar.sModbInfor)); 
        UTIL_MEM_cpy(&sWmDigVar.nModbus_u8, &aBuff[48], sizeof(sWmDigVar.nModbus_u8)); 
    }
#endif
  
#ifdef BOARD_LC_V1_1
    if (CAT24Mxx_Read_Array(CAT_ADDR_MODBUS_INFOR, aBuff, 64) == true) {
        //doc 64 byte tu eeprom
        temp = aBuff[0];
        //Check Byte EMPTY
        if (temp == BYTE_WRITEN) {
            UTIL_MEM_cpy(&sWmDigVar.sModbInfor, &aBuff[1], sizeof(sWmDigVar.sModbInfor)); 
            UTIL_MEM_cpy(&sWmDigVar.nModbus_u8, &aBuff[48], sizeof(sWmDigVar.nModbus_u8)); 
        } else {
            AppWm_Save_WM_Dig_Infor();
        }
    }
#endif
}
    

void AppWm_Save_WM_Dig_Infor (void)
{
    uint8_t aBuff[64] = {0};

    aBuff[0] = BYTE_WRITEN;
    UTIL_MEM_cpy(&aBuff[1], &sWmDigVar.sModbInfor, sizeof(sWmDigVar.sModbInfor));  
    UTIL_MEM_cpy(&aBuff[48], &sWmDigVar.nModbus_u8, sizeof(sWmDigVar.nModbus_u8)); 
    
#ifdef BOARD_QN_V5_0
    OnchipFlashPageErase(ADDR_MODBUS_INFOR);
    OnchipFlashWriteData(ADDR_MODBUS_INFOR, aBuff, 64);
#endif
    
#ifdef BOARD_LC_V1_1
    CAT24Mxx_Write_Buff(CAT_ADDR_MODBUS_INFOR, aBuff, 64);
#endif
}



void AppWm_Init_TNMT_Infor (void)
{    
    uint8_t temp = 0;
    uint8_t  aBuff[1664] = {0};
#ifdef BOARD_QN_V5_0
    temp = *(__IO uint8_t*) (ADDR_TNMT_CONFIG);
    //Check Byte EMPTY
    if (temp != FLASH_BYTE_EMPTY) {
        OnchipFlashReadData(ADDR_TNMT_CONFIG, &aBuff[0], 1664);
        sWmVar.ModePacket_u8 = aBuff[1];
        UTIL_MEM_cpy(&sWmVar.sChannInfor, &aBuff[2], sizeof(sWmVar.sChannInfor));
    }
#endif
    
#ifdef BOARD_LC_V1_1
    if (CAT24Mxx_Read_Array(CAT_ADDR_TNMT_CONFIG, aBuff, 1664) == true) {
        temp = aBuff[0];
        //Check Byte EMPTY
        if (temp == BYTE_WRITEN) {
            sWmVar.ModePacket_u8 = aBuff[1];
            UTIL_MEM_cpy(&sWmVar.sChannInfor, &aBuff[2], sizeof(sWmVar.sChannInfor)); 
        }
    }
#endif
    
    //kiem tra lai
    sWmVar.ModePacket_u8 = MIN(sWmVar.ModePacket_u8, MAX_PACKET_MODE);
    
    for (uint8_t i = 0; i < MAX_CHANNEL + MAX_SLAVE_MODBUS; i++) {
        if ((sWmVar.sChannInfor[i].MA_TINH[0] == FLASH_BYTE_EMPTY) 
            && (sWmVar.sChannInfor[i].MA_TINH[1] == FLASH_BYTE_EMPTY)
            && (sWmVar.sChannInfor[i].MA_TINH[2] == FLASH_BYTE_EMPTY) )
        {
            UTIL_MEM_set(&sWmVar.sChannInfor[i], 0, sizeof(sWmVar.sChannInfor[i]));
        }
        
        sWmVar.sChannInfor[i].nParam_u8 = MIN(sWmVar.sChannInfor[i].nParam_u8, MAX_PARAM_TNMT);
        
        for (uint8_t c = 0; c < sWmVar.sChannInfor[i].nParam_u8; c++) {
            sWmVar.sChannInfor[i].aPARAM[c] = MIN(sWmVar.sChannInfor[i].aPARAM[c], MAX_PARAM_TYPE);
        }
    }
    
    //kiem tra t�n
}

void AppWm_Save_TNMT_Infor (void)
{
    uint8_t aBuff[1664] = {0};

    aBuff[0] = BYTE_WRITEN;
    aBuff[1] = sWmVar.ModePacket_u8;
    
    UTIL_MEM_cpy(&aBuff[2], &sWmVar.sChannInfor, sizeof(sWmVar.sChannInfor)); 
    
#ifdef BOARD_QN_V5_0
    OnchipFlashPageErase(ADDR_TNMT_CONFIG);
    OnchipFlashWriteData(ADDR_TNMT_CONFIG, aBuff, 1664);
#endif
    
#ifdef BOARD_LC_V1_1
    CAT24Mxx_Write_Buff(CAT_ADDR_TNMT_CONFIG, aBuff, 1664);
#endif
}


void AppWm_Init_Default_Pressure (void)
{
    for (uint8_t i = 0; i < MAX_CHANNEL; i++)
    {
        sWmVar.aPRESSURE[i].sLinearInter.Factor_u16 = 100;
        sWmVar.aPRESSURE[i].sLinearInter.InMin_u16 = 4;
        sWmVar.aPRESSURE[i].sLinearInter.InMax_u16 = 20;
        sWmVar.aPRESSURE[i].sLinearInter.InUnit_u8 = _UNIT_MILIAM;
        sWmVar.aPRESSURE[i].sLinearInter.OutMin_u16 = 0;
        sWmVar.aPRESSURE[i].sLinearInter.OutMax_u16 = 10;
        sWmVar.aPRESSURE[i].sLinearInter.OutUnit_u8 = _UNIT_BAR;
    }
}

/*
    Func: Measure pressure process
        + On Power during 3s.   
        + Read ADC each channel
            ++ each channel get 10 sample and cacul average
            ...
        + Off Power and finish    
*/
uint8_t AppWm_Meas_Pressure_Process (uint32_t *delaynext)
{   
    static uint8_t Step_u8 = 0;
    static uint8_t Index = 0;
    double InMin = 0, InMax = 0, HeSo = 0; 
     
    uint32_t Channel = 0;
    uint8_t Result = PENDING;
    int32_t mVolTemp = 0;
    
    switch(Step_u8)
    {
        case 0:
            V_PIN_ON;
            Step_u8++;
            *delaynext = 3000; 
            break;
        case 1:  //6 kenh
            if (Index < MAX_CHANNEL)
            {
                switch (Index)
                {
                    case 0: Channel = ADC_CHANNEL_10; break;
                    case 1: Channel = ADC_CHANNEL_11; break;
                    case 2: Channel = ADC_CHANNEL_12; break;
                    case 3: Channel = ADC_CHANNEL_13; break;
                    case 4: Channel = ADC_CHANNEL_14; break;
                    case 5: Channel = ADC_CHANNEL_15; break;
                    default: Channel = ADC_CHANNEL_10; break;
                }
                
                mVolTemp = AppWm_Pressure_ADC_Vol (Channel);
                //Neu gia tri tra ve != -1: do ket thuc
                if (mVolTemp != -1)
                {
                    sWmVar.aPRESSURE[Index].VolAdc_u32 = mVolTemp;
                    
                    if (sWmVar.rPressCalib_u8 == true) {
                        sWmVar.aPRESSURE[Index].Calib_i16 = mVolTemp;
                    }
                    
                    sWmVar.aPRESSURE[Index].mVol_i32 = mVolTemp - sWmVar.aPRESSURE[Index].Calib_i16;
                    //convert gia tri mVol sang gia tri don vi ap suat
                #ifdef PRESSURE_DECODE
                    //Chuyen doi Pressure ra gia tri thuc te
                    HeSo = sWmVar.aPRESSURE[Index].sLinearInter.Factor_u16 * Convert_Scale(sWmVar.aPRESSURE[Index].sLinearInter.FactorDec_u8);
                        
                    InMin = sWmVar.aPRESSURE[Index].sLinearInter.InMin_u16 * HeSo;                    
                    InMax = sWmVar.aPRESSURE[Index].sLinearInter.InMax_u16 * HeSo;
                    if (sWmVar.aPRESSURE[Index].sLinearInter.InUnit_u8 == _UNIT_VOL) {
                        InMin *= 1000;
                        InMax *= 1000;
                    }
                    
                    sWmVar.aPRESSURE[Index].Val_i32 = (int32_t) AppWm_Linear_Interpolation(sWmVar.aPRESSURE[Index].mVol_i32, InMin, InMax,
                                                                                         sWmVar.aPRESSURE[Index].sLinearInter.OutMin_u16 * 1000, 
                                                                                         sWmVar.aPRESSURE[Index].sLinearInter.OutMax_u16 * 1000 );
                #else
                    sWmVar.aPRESSURE[Index].Val_i32 = sWmVar.aPRESSURE[Index].mVol_u32;
                #endif
                    
                    Index++;
                }
            } else
            {
                Index = 0;
                Step_u8++;
            }
            //enable 5m read again.
            *delaynext = 5;
            break;
        case 2: 
            if (sWmVar.rPressCalib_u8 == true) {
                sWmVar.rPressCalib_u8 = false;
                AppWm_Save_Press_Infor();
            }
            
            Step_u8 = 0; 
            Result = true;
            break;
        default:
            break;
    }   
    
    return Result;
}




uint16_t AppWm_Packet_Setting (uint8_t *aData, uint8_t chann)
{
    sData pData = {aData, 0};
    uint8_t aSERI_SENSOR[4] = {"0001"};
    aSERI_SENSOR[3] = chann + 0x31;
    uint8_t aBuff[32] = {0}, cObis = 0;
    uint64_t TempU64 = 0;
#ifdef PRESSURE_DECODE
    uint8_t DcuType = 1;
#else
    uint8_t DcuType = 0;
#endif
    
    SV_Protocol_Packet_Data(pData.Data_a8, &pData.Length_u16, OBIS_SERI_SENSOR, &aSERI_SENSOR, 4, 0xAA);
    
    SV_Protocol_Packet_Data(pData.Data_a8, &pData.Length_u16, OBIS_TYPE, &DcuType, 1, 0xAA);
    SV_Protocol_Packet_Data(pData.Data_a8, &pData.Length_u16, OBIS_SETT_PULSE_FACTOR, &sPulse[chann].FactorInt_i16, 2, sPulse[chann].FactorDec_u8);
    
    TempU64 = (uint64_t) ( sPulse[chann].Start_lf * Convert_Scale(0xFF - sPulse[chann].FactorDec_u8 + 1) );
    SV_Protocol_Packet_Data(pData.Data_a8, &pData.Length_u16, OBIS_SETT_PULSE_START, &TempU64, 8, sPulse[chann].FactorDec_u8);
    
#ifdef PRESSURE_DECODE
    SV_Protocol_Packet_Data(pData.Data_a8, &pData.Length_u16, OBIS_SETT_LI_IN_MIN, &sWmVar.aPRESSURE[chann].sLinearInter.InMin_u16, 2, 0);
    SV_Protocol_Packet_Data(pData.Data_a8, &pData.Length_u16, OBIS_SETT_LI_IN_MAX, &sWmVar.aPRESSURE[chann].sLinearInter.InMax_u16, 2, 0);
    SV_Protocol_Packet_Data(pData.Data_a8, &pData.Length_u16, OBIS_SETT_LI_IN_UNIT, &sWmVar.aPRESSURE[chann].sLinearInter.InUnit_u8, 1, 0xAA);
    
    SV_Protocol_Packet_Data(pData.Data_a8, &pData.Length_u16, OBIS_SETT_LI_OUT_MIN, &sWmVar.aPRESSURE[chann].sLinearInter.OutMin_u16, 2, 0);
    SV_Protocol_Packet_Data(pData.Data_a8, &pData.Length_u16, OBIS_SETT_LI_OUT_MAX, &sWmVar.aPRESSURE[chann].sLinearInter.OutMax_u16, 2, 0);
    SV_Protocol_Packet_Data(pData.Data_a8, &pData.Length_u16, OBIS_SETT_LI_OUT_UNIT, &sWmVar.aPRESSURE[chann].sLinearInter.OutUnit_u8, 1, 0xAA);
    
    SV_Protocol_Packet_Data(pData.Data_a8, &pData.Length_u16, OBIS_SETT_PRESS_FACTOR, &sWmVar.aPRESSURE[chann].sLinearInter.Factor_u16, 2, sWmVar.aPRESSURE[chann].sLinearInter.FactorDec_u8); 
#endif
    
    SV_Protocol_Packet_Data(pData.Data_a8, &pData.Length_u16, OBIS_SETT_PRESS_TYPE, &sWmVar.aPRESSURE[chann].sLinearInter.Type_u8, 1, 0xAA);
    
    
    
    aBuff[cObis++] = OBIS_TIME_DEVICE;
    aBuff[cObis++] = OBIS_SERI_SENSOR;
    aBuff[cObis++] = OBIS_WM_PULSE; 
#if (TSVH_OPERA_OR_FLASH == TSVH_MULTI)
    aBuff[cObis++] = OBIS_WM_DIG_FLOW;
#else
    aBuff[cObis++] = OBIS_WM_FLOW; 
#endif
    
    switch (sWmVar.aPRESSURE[chann].sLinearInter.Type_u8)
    {
        case __AN_PRESS:
            aBuff[cObis++] = OBIS_WM_PRESSURE;    
            break;
        case __AN_LEVEL:
            aBuff[cObis++] = OBIS_WM_LEVEL_VAL_SENSOR;
            break;
        default:
            aBuff[cObis++] = OBIS_WM_V_ANALOG;
            break;
    }

    aBuff[cObis++] = OBIS_DEV_VOL1;
#ifdef USING_APP_SIM
    aBuff[cObis++] = OBIS_RSSI_1;
#endif
    aBuff[cObis++] = OBIS_DEV_VOL2;
    aBuff[cObis++] = OBIS_FREQ_SEND;
    aBuff[cObis++] = OBIS_CONTACT_STATUS;
    
    SV_Protocol_Packet_Data(pData.Data_a8, &pData.Length_u16, OBIS_LIST_OBIS_USE, &aBuff, cObis, 0xAA);
    
    return pData.Length_u16;
}

/*
    @brief  Log Data TSVH
*/
void AppSensor_Log_Data_TSVH (void)
{
  if(sRTC.year > 20)
  {
    uint8_t     aMessData[256] = {0};
    uint8_t     Length = 0;
    
//    if (sRTC.year <= 20)
//        return;
    
    Length = SensorRS485_Packet_TSVH (&aMessData[0]);
//#ifdef USING_APP_MEM
//    AppMem_Write_Data(_MEM_DATA_TSVH_A, &aMessData[0], Length, sRecTSVH.SizeRecord_u16);
//#endif
        
//#ifdef USING_APP_MEM
    
    Mem_Write_Data(sMemVar.Type_u8, _MEM_DATA_TSVH, 0, aMessData, Length, sRecTSVH.Size_u16);
//#endif
  }
}
/*
    @brief  Packet TSVH
*/
uint8_t SensorRS485_Packet_TSVH (uint8_t *pData)
{
    uint16_t    length = 0;
    uint16_t	i = 0;
    uint8_t     TempCrc = 0;  
    
    //----------sTime--------------------
    SV_Protocol_Packet_Data(pData, &length, OBIS_TIME_DEVICE, &sRTC, 6, 0xAA);
      
    //----------  Clo_Du ------
    if(sDataSensorMeasure.sClo.State_u8 == 1)
        Sensor_Packet_Data(pData, &length, OBIS_ENVI_CLO_DU, &sDataSensorMeasure.sClo.Value_i32, sDataSensorMeasure.sClo.Scale_u8);
    
    //----------  pH ------
    if(sDataSensorMeasure.spH.State_u8 == 1)
        Sensor_Packet_Data(pData, &length, OBIS_ENVI_PH_WATER, &sDataSensorMeasure.spH.Value_i32, sDataSensorMeasure.spH.Scale_u8);
    
    //----------  Do Duc------
    if(sDataSensorMeasure.sTurb.State_u8 == 1)
        Sensor_Packet_Data(pData, &length, OBIS_ENVI_NTU, &sDataSensorMeasure.sTurb.Value_i32, sDataSensorMeasure.sTurb.Scale_u8);
    
    //----------  SALINITY %------
    if(sDataSensorMeasure.sSal.State_u8 == 1)
        Sensor_Packet_Data(pData, &length, OBIS_ENVI_SALINITY_UNIT, &sDataSensorMeasure.sSal.Value_i32, sDataSensorMeasure.sSal.Scale_u8);
    
    //----------  Temperature ------
    if(sDataSensorMeasure.sTemp.State_u8 == 1)
        Sensor_Packet_Data(pData, &length, OBIS_ENVI_OXY_TEMPERATURE, &sDataSensorMeasure.sTemp.Value_i32, sDataSensorMeasure.sTemp.Scale_u8);
    
    //----------  EC ------
    if(sDataSensorMeasure.sEC.State_u8 == 1)
        Sensor_Packet_Data(pData, &length, OBIS_ENVI_EC, &sDataSensorMeasure.sEC.Value_i32, sDataSensorMeasure.sEC.Scale_u8);

    //----------  COD ------
    if(sDataSensorMeasure.sCOD.State_u8 == 1)
        Sensor_Packet_Data(pData, &length, OBIS_ENVI_COD, &sDataSensorMeasure.sCOD.Value_i32, sDataSensorMeasure.sCOD.Scale_u8);
    
    //----------  NH4------
    if(sDataSensorMeasure.sNH4.State_u8 == 1)
        Sensor_Packet_Data(pData, &length, OBIS_ENVI_NH4, &sDataSensorMeasure.sNH4.Value_i32, sDataSensorMeasure.sNH4.Scale_u8);
    
    //----------  DO------
    if(sDataSensorMeasure.sDO.State_u8 == 1)
        Sensor_Packet_Data(pData, &length, OBIS_ENVI_OXY_MG_L, &sDataSensorMeasure.sDO.Value_i32, sDataSensorMeasure.sDO.Scale_u8);
    
    //----------  TSS ------
    if(sDataSensorMeasure.sTSS.State_u8 == 1)
        Sensor_Packet_Data(pData, &length, OBIS_ENVI_TSS, &sDataSensorMeasure.sTSS.Value_i32, sDataSensorMeasure.sTSS.Scale_u8);
    
    //----------  TDS ------
    if(sDataSensorMeasure.sTDS.State_u8 == 1)
        Sensor_Packet_Data(pData, &length, OBIS_ENVI_TDS, &sDataSensorMeasure.sTDS.Value_i32, sDataSensorMeasure.sTDS.Scale_u8);
    
    //----------  NO3 ------
    if(sDataSensorMeasure.sNO3.State_u8 == 1)
        Sensor_Packet_Data(pData, &length, OBIS_ENVI_NO3, &sDataSensorMeasure.sNO3.Value_i32, sDataSensorMeasure.sNO3.Scale_u8);
    
    //----------Tan suat--------------------
    SV_Protocol_Packet_Data(pData, &length, OBIS_RSSI_1, &sSimCommInfor.RSSI_u8, 1, 0x00);
    SV_Protocol_Packet_Data(pData, &length, OBIS_FREQ_SEND, &sModemInfor.sFrequence.DurOnline_u32 , 2, 0x00);
    // caculator crc
    length++;
	for (i = 0; i < (length - 1); i++)
		TempCrc ^= pData[i];

    pData[length-1] = TempCrc;
    
    return length;
}

/*
    @brief  Sensor Packet Data
*/
void Sensor_Packet_Data(uint8_t *pTarget, uint16_t *LenTarget, uint8_t Obis,
                        void *pData, uint8_t Scale)
{
    uint16_t Pos = *LenTarget;
    uint8_t LenData = 0;
    int32_t Data_i32 = 0;
    
    Data_i32 = *(int32_t *)pData;
    if(Data_i32 > 32767 || Data_i32 < -32768)
        LenData = 4;
    else 
        LenData = 2;
    
//    if (LenData == 0)
//        return;
    
    pTarget[Pos++] = Obis;
    pTarget[Pos++] = LenData;
            
    switch (Obis)
    {
        case OBIS_ENVI_OXY_MG_L:
        case OBIS_ENVI_OXY_PERCENT:
        case OBIS_ENVI_OXY_TEMPERATURE:
        case OBIS_ENVI_EC:
        case OBIS_ENVI_SALINITY:
        case OBIS_ENVI_PH_WATER:
        case OBIS_ENVI_SALINITY_UNIT:
        case OBIS_ENVI_CLO_DU:
        case OBIS_ENVI_NTU:
        case OBIS_ENVI_NH4:
        case OBIS_ENVI_COD:
        case OBIS_ENVI_TSS:
        case OBIS_ENVI_TDS:
        case OBIS_ENVI_NO3:

            AppComm_Sub_Packet_Integer(pTarget, &Pos, pData, LenData);
            
            if (Scale != 0xAA)
                pTarget[Pos++] = Scale;
            break;
            
        default:
            break;
    }
    
    *LenTarget = Pos ;
}

void AppWm_Get_Pulse (void)
{
#ifdef LPTIM_ENCODER_MODE 
    LPTIM_Encoder_Get_Pulse(&hlptim1, &sPulse[0]);
#else   
    LPTIM_Counter_Get_Pulse(&hlptim1, &sPulse[0]);
    AppWm_Plus_5Pulse(&sPulse[0]);
#endif
    
#ifdef LPTIM2_ENCODER_MODE 
    LPTIM_Encoder_Get_Pulse(&hlptim2, &sPulse[1]);
#else   
    LPTIM_Counter_Get_Pulse(&hlptim2, &sPulse[1]);
    AppWm_Plus_5Pulse(&sPulse[1]);
#endif
      
    TIM_Counter_Get_Pulse(&htim1, &sPulse[2]);  
    TIM_Counter_Get_Pulse(&htim2, &sPulse[3]);
}



void AppWm_RS485_Rx_Done (void)
{
    fevent_active(sEventAppWM, _EVENT_RS485_1_RECV);
}

void AppWm_RS485_2_Rx_Done (void)
{
    fevent_active(sEventAppWM, _EVENT_RS485_2_RECV);
}

/*
    Kiem tra xem kenh nao da cai dat ten: tinh + ctrinh + tram
        de ghep vao FILE name va ti nua ghep vao payload
*/
int8_t AppWm_Is_Firt_Chann_TN (void)
{   
    uint8_t i = 0;
    
    for (i = 0; i < MAX_CHANNEL + MAX_SLAVE_MODBUS; i++) {
        if (strlen(sWmVar.sChannInfor[i].MA_TINH) != 0) {
            return i;
        }
    }

    return -1;
}


uint8_t AppWm_Over_5Pulse (void)
{
    for (uint8_t i = 0; i < MAX_CHANNEL; i++) {
        if (sPulse[i].IsOver5Pulse_u8 == false) {
            return false;
        }
    }
    
    return true;
}

static void OnTimerPressure(void *context)
{
    fevent_active(sEventAppWM, _EVENT_MEAS_PRESSURE);
    fevent_active(sEventAppWM, _EVENT_RS485_MODBUS);
    iMarkTSVH = 0x01;
}

void AppComm_Sub_Packet_Integer (uint8_t *pTarget, uint16_t *LenTarget, void *Data, uint8_t LenData)
{
    uint16_t Pos = *LenTarget;
    
    uint8_t  *TempU8;
    uint16_t *TempU16;
    uint32_t *TempU32;
    uint64_t *TempU64;
    
    switch (LenData)
    {
        case 1:    
            TempU8 = (uint8_t *) Data;
            pTarget[Pos++] = *TempU8;
            break;
        case 2:
            TempU16 = (uint16_t *) Data;
            pTarget[Pos++] = ((*TempU16) >> 8) & 0xFF;
            pTarget[Pos++] =  (*TempU16) & 0xFF;
            break;
        case 4:
            TempU32 = (uint32_t *) Data;
            pTarget[Pos++] = ((*TempU32) >> 24) & 0xFF;
            pTarget[Pos++] = ((*TempU32) >> 16) & 0xFF;
            pTarget[Pos++] = ((*TempU32) >> 8) & 0xFF;
            pTarget[Pos++] =  (*TempU32) & 0xFF;
            break;
        case 8:
            TempU64 = (uint64_t *) Data;
            pTarget[Pos++] = ((*TempU64) >> 56) & 0xFF;
            pTarget[Pos++] = ((*TempU64) >> 48) & 0xFF;
            pTarget[Pos++] = ((*TempU64) >> 40) & 0xFF;
            pTarget[Pos++] = ((*TempU64) >> 32) & 0xFF;
            pTarget[Pos++] = ((*TempU64) >> 24) & 0xFF;
            pTarget[Pos++] = ((*TempU64) >> 16) & 0xFF;
            pTarget[Pos++] = ((*TempU64) >> 8) & 0xFF;
            pTarget[Pos++] =  (*TempU64) & 0xFF;
            break;
        default:
            break;
    }
    
    *LenTarget = Pos ;
}



