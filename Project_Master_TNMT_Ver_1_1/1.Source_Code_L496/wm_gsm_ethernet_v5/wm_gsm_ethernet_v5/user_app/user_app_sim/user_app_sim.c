

#include "user_app_sim.h"
#include "user_string.h"
#include "math.h"
#include "user_timer.h"
#include "user_time.h"


/*================ Var struct =================*/  
static uint8_t _Cb_Sim_Hard_Reset(uint8_t event);
static uint8_t _Cb_Sim_Turn_On (uint8_t event);
static uint8_t _Cb_Sim_WakeUp(uint8_t event);
static uint8_t _Cb_Sim_Ctrl_DTR(uint8_t event);
static uint8_t _Cb_Sim_Power_Off(uint8_t event);
static uint8_t _Cb_Sim_PSM(uint8_t event);

static uint8_t _Cb_Event_Sim_Send_Ping (uint8_t event);
static uint8_t _Cb_Event_Sim_Send_Mess (uint8_t event);
static uint8_t _Cb_Event_Sim_Req_GPS (uint8_t event);
static uint8_t _Cb_Event_Sim_Get_GPS (uint8_t event);
static uint8_t _Cb_Event_Sim_Sms_Ctrl (uint8_t event);
static uint8_t _Cb_Event_Sim_Call_Ctrl (uint8_t event);

sEvent_struct sEventAppSim[] = 
{
    { _EVENT_SIM_HARD_RESET,		0, 0, 0, 		_Cb_Sim_Hard_Reset },
	{ _EVENT_SIM_TURN_ON, 			0, 0, 0, 		_Cb_Sim_Turn_On },
	{ _EVENT_SIM_WAKEUP, 			0, 0, 50, 		_Cb_Sim_WakeUp },
    { _EVENT_SIM_DTR_PIN, 	        0, 0, 1000, 	_Cb_Sim_Ctrl_DTR },
    { _EVENT_SIM_POWER_OFF, 		0, 0, 0, 		_Cb_Sim_Power_Off },
    { _EVENT_SIM_PSM, 		        0, 0, 0, 		_Cb_Sim_PSM },
    
    { _EVENT_SIM_SEND_MESS,  		0, 0, 1000,     _Cb_Event_Sim_Send_Mess },  
    { _EVENT_SIM_SEND_PING,  		1, 0, 30000,    _Cb_Event_Sim_Send_Ping },
    { _EVENT_SIM_REQ_GPS,  		    1, 0, 5000,     _Cb_Event_Sim_Req_GPS },  
    { _EVENT_SIM_GET_GPS,  		    1, 0, 100,      _Cb_Event_Sim_Get_GPS },   
    
    { _EVENT_SIM_SMS_CTRL,  		0, 0, 1000,     _Cb_Event_Sim_Sms_Ctrl }, 
    { _EVENT_SIM_CALL_CTRL,  		0, 0, 1000,     _Cb_Event_Sim_Call_Ctrl },  
};


static void AppSim_AT_Faillure (uint8_t Step);
static void AppSim_Handle_AT (uint8_t step);
static void AppSim_Process_URC (uint8_t Type);
static void AppSim_Uart_Send (uint8_t *pData, uint16_t length);
static void AppSim_Process_Recv (uint8_t *pdata, uint16_t length);

sAppSimVariable sAppSimVar;

static UTIL_TIMER_Object_t TimerPowerOn;
static void OnTimerPowerOn(void *context);

static char aAPN_NAME[BUFF_LENGTH_SETUP] = "SV123";  
static char aAPN_DIAL_STRING[BUFF_LENGTH_SETUP] = "12345";
static char aAPN_USERNAME[BUFF_LENGTH_SETUP] = "xyz";  
static char aAPN_PASSWORD[BUFF_LENGTH_SETUP] = "1111";
/*================ Func =================*/
static uint8_t _Cb_Sim_Hard_Reset(uint8_t event)
{
    fevent_active(sEventAppSim, _EVENT_SIM_TURN_ON);

	return 1;
}



static uint8_t _Cb_Sim_Turn_On (uint8_t event)
{
#ifdef BOARD_SIM_V1_5 
    static uint8_t mark_u8 = false;
#endif
    AppSim_Default_Sms_Call();
    Sim_Common_Init(&sSimCommVar);
    //Disable Event
	Sim_Disable_All_Event();  
    
#ifdef BOARD_SIM_V1_5
    if (SIM_CARD_DT == 0) {
        if (mark_u8 == false) {
            mark_u8 = true;
            UTIL_Log_Str(DBLEVEL_M, "u_app_sim: sim card not detect...\r\n" ); 
        }
        
        sAppSimVar.StepPowerOn_u8 = 0;
        sAppSimVar.cSoftReset_u8 = MAX_SOFT_RESET;
        AppSim_AT_Faillure(0);
        
        return 1;
    }
    
    mark_u8 = false;
#endif

    //Power on 
	if (AppSim_Power_On() == 1)
	{
        sSimCommVar.State_u8  = _SIM_INIT;
        //Push AT cmd 
        if (UTIL_var.ModeConnFuture_u8 != 0)
        {
            if ((UTIL_var.ModeConnNow_u8 == _CONNECT_DATA_MAIN) 
                || (UTIL_var.ModeConnNow_u8 == _CONNECT_DATA_BACKUP))
            {
                UTIL_var.ModeConnLast_u8 = UTIL_var.ModeConnNow_u8;   
            } else if (UTIL_var.ModeConnNow_u8 == _CONNECT_FTP_UPLOAD) {
                UTIL_var.ModeConnLast_u8 = _CONNECT_DATA_MAIN;
            }
            
            UTIL_var.ModeConnNow_u8 = UTIL_var.ModeConnFuture_u8;
            UTIL_var.ModeConnFuture_u8 = 0; 
        }

        if (sAppSimVar.pSim_Set_Server != NULL) {
            sAppSimVar.pSim_Set_Server();
        }
        
        switch (UTIL_var.ModeConnNow_u8)
        {
            case _CONNECT_DATA_MAIN:    
            case _CONNECT_DATA_BACKUP:
                Sim_Clear ();
                Sim_Push_Block_To_Queue(aSIM_BLOCK_STEP_INIT);  
                Sim_Push_Block_To_Queue(aSIM_BLOCK_STEP_NETWORK);
                if (sSimCommVar.iGetClock_u8 == false) {
                    Sim_Push_Block_To_Queue(aSIM_BLOCK_STEP_CLOCK); 
                }
                if (AppSim_Need_Sms() == true)
                    AppSim_Push_Send_Sms();
                
                if (sAppSimVar.sCall.Mark_u8 == true)
                    AppSim_Push_Call();
                
                Sim_Push_Block_To_Queue(aSIM_BLOCK_STEP_CONNECT);
                break;
            case _CONNECT_FTP_UPLOAD:
                Sim_Push_Block_To_Queue(aSIM_BLOCK_STEP_INIT);  
                Sim_Push_Block_To_Queue(aSIM_BLOCK_STEP_NETWORK);
                if (sSimCommVar.iGetClock_u8 == false) {
                    Sim_Push_Block_To_Queue(aSIM_BLOCK_STEP_CLOCK); 
                }
            
                Sim_Push_Block_To_Queue(aSIM_BLOCK_STEP_FTP_CONN);
                break;
            case _CONNECT_FTP:
                sAppSimVar.cHardReset_u16 += MAX_SIM_CHANGE_SERVER*2;  //de chuyen mode conn
                break;
            case _CONNECT_HTTP:
                //Dung chung http va ftp = http
                Sim_Clear ();
                Sim_Push_Block_To_Queue(aSIM_BLOCK_STEP_INIT); 
                Sim_Push_Block_To_Queue(aSIM_BLOCK_STEP_HTTP_INIT);
                Sim_Push_Block_To_Queue(aSIM_BLOCK_STEP_HTTP_READ);
                break;
            default:
                break;      
        } 
	}

	return 1;
}


static uint8_t _Cb_Sim_WakeUp (uint8_t event)
{
#ifdef SIM_BC660_LIB
    sSimCommVar.State_u8  = _SIM_INIT_TCP;
	switch ( AppSim_Wake_Up())
	{
        case 0:
            fevent_active(sEventAppSim, _EVENT_SIM_TURN_ON);
            break;
        case 1:
            Sim_Clear();
            Sim_Push_Block_To_Queue(aSIM_BLOCK_STEP_CONNECT, strlen(aSIM_BLOCK_STEP_CONNECT));
            break;
        default:
            fevent_enable(sEventAppSim, _EVENT_SIM_WAKEUP);
            break;  
	} 
#endif
    
	return 1;
}

/*
    Func: Control DTR PIN to Out Data mode
    Lan dau tien vao keo chan DTR low -> 1s sau Keo DTR lï¿½n high: High to low?
*/

static uint8_t _Cb_Sim_Ctrl_DTR(uint8_t event)
{
    static uint8_t DTRStep_u8 = 0;
    
    if (DTRStep_u8 == 0)
    {
        SIM_DTR_ON1;
        DTRStep_u8++;
        fevent_enable(sEventAppSim, _EVENT_SIM_DTR_PIN);
    } else
    {
        SIM_DTR_OFF1;
        DTRStep_u8 = 0;
    }
    
	return 1;
}

static uint8_t _Cb_Sim_Power_Off(uint8_t event)
{
    Sim_Disable_All_Event();
    
    UTIL_TIMER_Stop (&TimerPowerOn);
    sAppSimVar.StepPowerOn_u8 = 0;

    UTIL_Log_Str(DBLEVEL_M, "u_app_sim: power off...\r\n" );
    //clear queue
    Sim_Clear ();
    //Set status PSM
    sSimCommVar.State_u8 = _SIM_POWER_OFF;
    //Tat ngat nhan Uart
    Sim_Stop_RX_Mode();
    
    //Power off bang cach ngat nguon dien
    SIM_PW_OFF1; 
    sAppSimVar.iOnPower_u8 = false;
        
    Get_RTC();
    sModemVar.DuraOpera_u32 = HW_RTC_GetCalendarValue_Second(sRTC, 0) - sModemVar.LandMarkOpera_u32;
    
    return 1;
}

static uint8_t _Cb_Sim_PSM(uint8_t event)
{
#ifdef SIM_BC660_LIB
    Sim_Push_Block_To_Queue(aSIM_BLOCK_STEP_DIS_CONN);
    Sim_Push_Block_To_Queue(aSIM_BLOCK_STEP_PSM);
#endif

    return 1;
}


static uint8_t _Cb_Event_Sim_Send_Ping (uint8_t event)
{
    static uint8_t Count_Ping = 0;
    
    if (UTIL_var.ModePower_u8 == _POWER_MODE_ONLINE) {
        sMessage.aMESS_PENDING[DATA_PING] = TRUE;
        fevent_enable(sEventAppSim, _EVENT_SIM_SEND_PING);   
    } else {
        Count_Ping++;
        
        if (Count_Ping < MAX_PING_TO_CONFIG) {
            sMessage.aMESS_PENDING[DATA_PING] = TRUE;
            fevent_enable(sEventAppSim, _EVENT_SIM_SEND_PING); 
        } else {
            Count_Ping = 0;
            sAppSimVar.rExternIrq_u8 = false;
        }
    }
    
    return 1;
}


/*
    Cu 5s vao day de push at get GPS
*/
static uint8_t _Cb_Event_Sim_Req_GPS (uint8_t event)
{
 
    return 1;
}


static uint8_t _Cb_Event_Sim_Get_GPS (uint8_t event)
{

    return 1;
}


static uint8_t _Cb_Event_Sim_Sms_Ctrl (uint8_t event)
{
    static uint8_t Retry = 0;
    
    if (sSimCommVar.State_u8 >= _SIM_SETTING)
    {
        if (sAppSimVar.sSms.Pending_u8 == false)
        {
            if (AppSim_Need_Sms() == true)
            {
                AppSim_Push_Send_Sms();
            } else
            {
                Retry = 0;
            }
        }
    }

    //qua so lan retry
    if (Retry >= 3)
    {
        Retry = 0;
        sAppSimVar.sCall.Pending_u8 = false;
        sAppSimVar.sCall.Mark_u8 = false;
    }
    
     //check queue sim ma k co lenh at thi push lại call
    if (qGet_Number_Items(&qSimStep) == 0)
    {
        sAppSimVar.sSms.Pending_u8 = false;
    }
    
    fevent_enable(sEventAppSim, event); 
    
    return 1;
}


static uint8_t _Cb_Event_Sim_Call_Ctrl (uint8_t event)
{
    static uint8_t Retry = 0;
    
    if (sSimCommVar.State_u8 >= _SIM_SETTING)
    {
        if (sAppSimVar.sCall.Pending_u8 == false)
        {
            if (sAppSimVar.sCall.Mark_u8 == true)
            {
                Retry++;
                AppSim_Push_Call();
            } else
            {
                Retry = 0;
            }
        }
    }
    
    //qua so lan retry
    if (Retry >= 3)
    {
        Retry = 0;
        sAppSimVar.sCall.Pending_u8 = false;
        sAppSimVar.sCall.Mark_u8 = false;
    }
    
    //check queue sim ma k co lenh at thi push lại call
    if (qGet_Number_Items(&qSimStep) == 0)
    {
        sAppSimVar.sCall.Pending_u8 = false;
    }

    fevent_enable(sEventAppSim, event); 
    
    return 1;
} 


/*
    
*/
static uint8_t _Cb_Event_Sim_Send_Mess (uint8_t event)
{
    if (sSimCommVar.State_u8 != _SIM_CONN_MQTT)
    {
        return 1;
    }
       
    //
    sSimCommVar.ServerReady_u8 = true;
    
    if (AppSim_Send_Mess () == true)  //Co ban tin
    {
        fevent_disable(sEventAppSim, _EVENT_SIM_SEND_PING);
        
        if (UTIL_var.ModePower_u8 == _POWER_MODE_ONLINE)
            sEventAppSim[event].e_period = 5000;
        else
            sEventAppSim[event].e_period = 2000;
    } else
    {
        sEventAppSim[event].e_period = 10;
        //Neu Online: enable Ping  | Enable event Power SIM
        if ( (UTIL_var.ModePower_u8 == _POWER_MODE_ONLINE)
            || (sAppSimVar.rExternIrq_u8 == true) )
        {
            if (UTIL_var.ModeConnNow_u8 == _CONNECT_FTP_UPLOAD) {
                //kiem tra neu gui xong roi va k co lenh at nao nua -> chuyen mode
                if ( ( sRecGPS.iSend_u16 == sRecGPS.iSave_u16 )
                    && (sSimVar.Pending_u8 == false) ) {
                    UTIL_var.ModeConnNow_u8 = UTIL_var.ModeConnLast_u8;
                    //close va chuyen mode
                    Sim_Push_Block_To_Queue(aSIM_BLOCK_STEP_FTP_DISCONN);
                    Sim_Push_Block_To_Queue(aSIM_BLOCK_STEP_CONNECT);
                    sSimCommVar.State_u8 = _SIM_INIT;                     
//                    AppSim_Restart_Imediate();
                    return 1;
                }
            } else {
                //Mark Send PING
                if (sEventAppSim[_EVENT_SIM_SEND_PING].e_status == 0)
                    fevent_enable(sEventAppSim, _EVENT_SIM_SEND_PING);
            }
        } else
        {
            //Neu con ban tin moi: Enable lai check new rec
            if (Mem_Is_New_Record() != true)
            {
            #ifdef SIM_BC660_LIB  
                fevent_active(sEventAppSim, _EVENT_SIM_PSM);
            #else
                if (UTIL_var.ModeConnNow_u8 == _CONNECT_FTP_UPLOAD) {
                    Sim_Push_Block_To_Queue(aSIM_BLOCK_STEP_FTP_DISCONN);
                } else {
                    Sim_Push_Block_To_Queue(aSIM_BLOCK_STEP_DIS_CONN);
                }
            #endif
                return 1;
            }
        }
    }
    
    fevent_enable(sEventAppSim, event);
    
    return 1;
}


/*============== Function Handler ====================*/   

/*
    Func Init App SIM
*/

void AppSim_Init (void)
{
    sCbSimComm.pFailure         = AppSim_AT_Faillure;
    sCbSimComm.pHandle_AT       = AppSim_Handle_AT;
    sCbSimComm.phandle_URC      = AppSim_Process_URC;
    sCbSimComm.phandle_Recv     = AppSim_Process_Recv;
    sCbSimComm.pGet_GPS_OK      = AppSim_GPS_OK;
    sCbSimComm.pGet_GPS_ERROR   = AppSim_GPS_Error;
    sCbSimComm.pConfig_Update   = AppSim_Config_Update; 
    sCbSimComm.pSend            = AppSim_Uart_Send;
    
    //Init infor
    sSimCommVar.sAPN.sName = aAPN_NAME; 
    sSimCommVar.sAPN.sDialString = aAPN_DIAL_STRING; 
    sSimCommVar.sAPN.sUsername = aAPN_USERNAME; 
    sSimCommVar.sAPN.sPassword = aAPN_PASSWORD; 
        
#ifdef USING_AT_CONFIG
    sATCmdList[_QUERY_SIM_ID].CallBack = AppSim_SER_Get_Sim_ID,
#endif
    
#ifdef USING_APP_WM
    sWmVar.pRq_Instant = AppSim_Set_Instant;
    sWmVar.pCtrl_Sim = AppSim_Allow_Send;
#endif
    
    //Init Module Sim
    Sim_Init();
         
#ifdef MQTT_PROTOCOL
    sMessage.aMESS_PENDING[DATA_HANDSHAKE] = TRUE;
#endif
    
//    sMessage.aMESS_PENDING[DATA_INTAN] = TRUE;
    sMessage.aMESS_PENDING[SEND_SERVER_TIME_PENDING] = TRUE;
    //Firt: mark error GPS
    AppSim_GPS_Error();
    
    //active power on first
    fevent_active(sEventAppSim, _EVENT_SIM_TURN_ON);
    UTIL_TIMER_Create(&TimerPowerOn, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, OnTimerPowerOn, NULL);
    
    sAppSimVar.iOnPower_u8 = true;
}


 
uint8_t AppSim_Task(void)
{
	uint8_t i = 0;
	uint8_t Result = 0;

	for (i = 0; i < _EVENT_END_SIM; i++)
	{
		if (sEventAppSim[i].e_status == 1)
		{
            if ( (i != _EVENT_SIM_REQ_GPS) && (i != _EVENT_SIM_GET_GPS) ) 
                Result = 1;
            
			if ((sEventAppSim[i].e_systick == 0) ||
					((HAL_GetTick() - sEventAppSim[i].e_systick)  >=  sEventAppSim[i].e_period))
			{
                sEventAppSim[i].e_status = 0; 
				sEventAppSim[i].e_systick = HAL_GetTick();
				sEventAppSim[i].e_function_handler(i);
			}
		}
	}
    
    Result |= Sim_Task();

	return Result;
}



/*-------Init From memory ----*/
/*
    Func: Check New Mess
*/

uint8_t AppSim_Send_Mess (void)
{
    uint16_t i = 0;
    uint8_t Result = FALSE;
    
    //Kiem tra xem co ban tin nao can gui di khong
    for (i = TOPIC_NOTIF; i < END_MQTT_SEND; i++)
    {
        if (sMessage.aMESS_PENDING[i] == TRUE)
        {
            if (UTIL_var.ModeConnNow_u8 == _CONNECT_FTP_UPLOAD) {
                if (i != DATA_GPS) {
                    continue;
                }
            } else {
                if (i == DATA_GPS) {
                    continue;
                }
            }

            // Dang ket noi ethernet chi xu ly SIM Ping.
        #ifdef USING_APP_ETHERNET
            if ( ( (sAppEthVar.Status_u8 >= _ETH_TCP_CONNECT) && (i != DATA_PING) ) 
                  || (i == DATA_PING_2)
                )
            {
                continue;
            }
        #endif
            
            Result = TRUE;
            //
            if (sMessage.Status_u8 != PENDING)
            {
                //Danh dau Mess type hien tai ->neu OK clear di
                sMessage.Type_u8 = i;
                //Dong goi ban tin vao buff aPAYLOAD bang cach thuc hien callback
                if (sMessSend[i].CallBack(i) == TRUE)
                {
                    //Day 2 step Publish vao Queue send AT: 2 option wait ACK and No wait ACK 
                    if (sMessage.Type_u8 == DATA_PING) {
                        Sim_Push_Block_To_Queue(aSIM_BLOCK_STEP_PING);
                    } else {
                        if (UTIL_var.ModeConnNow_u8 == _CONNECT_FTP_UPLOAD) {
                            Sim_Push_Block_To_Queue(aSIM_BLOCK_STEP_FTP_PUB);
                        } else {
                            if (sMessage.PubQos != 0) {
                                Sim_Push_Block_To_Queue(aSIM_BLOCK_STEP_PUB_FB);
                            } else {
                                Sim_Push_Block_To_Queue(aSIM_BLOCK_STEP_PUB); 
                            }
                        }
                    }
                     //Set flag status= pending
                    sMessage.Status_u8 = PENDING;
                } else
                    sMessage.aMESS_PENDING[i] = FALSE;
            } 
        }
    }
    
    return Result;
}



/*
    Func: Func pointer Handler AT cmd
        + Input: Type: Get ID SIM,....
        + Ouput: Handler
*/


static void AppSim_Handle_AT (uint8_t step)
{    
    switch (step)
    { 
        case _SIM_COMM_GET_STIME:  
            _CbAppSim_Recv_sTime (&sSimCommVar.sTime);
            break;
        case _SIM_COMM_SMS_SEND_1:  
            _CbAppSim_SMS_Send_1();
            break;
        case _SIM_COMM_SMS_SEND_2:  
            _CbAppSim_SMS_Send_2();
            break;
        case _SIM_COMM_SMS_SEND_3:  
            _CbAppSim_SMS_Send_3();
            break;
        case _SIM_COMM_CALLING_1:  
            _CbAppSim_Calling_1();
            break;
        case _SIM_COMM_CALLING_2:  
            _CbAppSim_Calling_2();
            break;
        case _SIM_COMM_TCP_CLOSE:  
            if (UTIL_var.ModePower_u8 == _POWER_MODE_SAVE) {
                fevent_active(sEventAppSim, _EVENT_SIM_POWER_OFF);
            }
            break;
        case _SIM_COMM_TCP_SEND_1:  
            _CbAppSim_TCP_Send_1(&sSimCommVar.pSender);
            break;
        case _SIM_COMM_TCP_SEND_2:  
            sSimCommVar.pSender.Data_a8 = sMessage.str.Data_a8;
            sSimCommVar.pSender.Length_u16 = sMessage.str.Length_u16;
            break;
        case _SIM_COMM_MQTT_CONN_1:
            mConnect_MQTT( sMessage.pModemId,
                           sSimCommVar.sServer.sMQTTUserName, 
                           sSimCommVar.sServer.sMQTTPassword,
                           60 );
  
            sSimCommVar.pSender.Data_a8 = sMessage.str.Data_a8;
            sSimCommVar.pSender.Length_u16 = sMessage.str.Length_u16;
            
            break;
        case _SIM_COMM_MQTT_SUB_1:
            //Truyen chuoi Subcribe
            mSubcribe_MQTT ();
        
            sSimCommVar.pSender.Data_a8 = sMessage.str.Data_a8;
            sSimCommVar.pSender.Length_u16 = sMessage.str.Length_u16;
            break;
        case _SIM_COMM_MQTT_SUB_2:
            mSet_default_MQTT(); 
            fevent_active(sEventAppSim, _EVENT_SIM_SEND_MESS);
            UTIL_Printf_Str( DBLEVEL_M, "u_app_sim: mqtt connect ok!\r\n" );
            if (UTIL_var.ModeConnNow_u8 == _CONNECT_FTP_UPLOAD) {
                if (AppSim_Set_FileName_FTP() == false) {
                    //k thanh cong se can quay lai connect mqtt ngay
                    sMessage.aMESS_PENDING[SEND_SERVER_FTP_FAIL] = TRUE;  //bao len FTP fail
                    UTIL_var.ModeConnNow_u8 = UTIL_var.ModeConnLast_u8;
                    //close va chuyen mode
                    Sim_Push_Block_To_Queue(aSIM_BLOCK_STEP_FTP_DISCONN);
                    Sim_Push_Block_To_Queue(aSIM_BLOCK_STEP_CONNECT);
                    sSimCommVar.State_u8 = _SIM_INIT; 
//                    AppSim_Restart_Imediate();
                }
            }
            break;
        case _SIM_COMM_MQTT_PUB_1:
            sSimCommVar.pSender.Data_a8 = sMessage.str.Data_a8;
            sSimCommVar.pSender.Length_u16 = sMessage.str.Length_u16;           
            break;
        case _SIM_COMM_MQTT_PUB_2:
            _CbAppSim_Recv_PUBACK ();
            break;
        case _SIM_COMM_HTTP_HEAD_1:  
            _CbAppSim_Http_Head_1(&sMessage.str);
            break;
         case _SIM_COMM_HTTP_HEAD_2:  
            _CbAppSim_Http_Head_2();
            break;
        case _SIM_COMM_HTTP_SEND_1:  
            _CbAppSim_Http_Send_1(&sMessage.str);
            break;
        case _SIM_COMM_HTTP_SEND_2:  
            sSimCommVar.pSender.Data_a8 = sMessage.str.Data_a8;
            sSimCommVar.pSender.Length_u16 = sMessage.str.Length_u16;
            break;
        case _SIM_COMM_HTTP_SEND_EX:  
            _CbAppSim_Http_Send_Ex(&sMessage.str);
            break;
        case _SIM_COMM_HTTP_READ_OK:
            Sim_Push_Block_To_Queue(aSIM_BLOCK_STEP_HTTP_READ); 
            break; 
        case _SIM_COMM_HTTP_UPDATE_OK: 
        #ifdef USING_INTERNAL_MEM
            Erase_Firmware(ADDR_FLAG_HAVE_NEW_FW, 1);
            //ghi Flag update va Size firm vao Inflash
            HAL_FLASH_Unlock();
            HAL_Delay(10);
            
        #if (FLASH_BYTE_WRTIE == 8)
            HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, 
                              ADDR_FLAG_HAVE_NEW_FW, 0xAA);
            HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, 
                              ADDR_FLAG_HAVE_NEW_FW + FLASH_BYTE_WRTIE, sSimFwUpdate.CountByteTotal_u32);
        #else
            HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, 
                              ADDR_FLAG_HAVE_NEW_FW, 0xAA);
            HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, 
                              ADDR_FLAG_HAVE_NEW_FW + FLASH_BYTE_WRTIE, sSimFwUpdate.CountByteTotal_u32);
        #endif
            
            HAL_Delay(10);
            HAL_FLASH_Lock();
        #endif
            sMessage.aMESS_PENDING[SEND_UPDATE_FIRM_OK] = TRUE;
            //Init Sim again
            UTIL_var.ModeConnNow_u8 = UTIL_var.ModeConnLast_u8;
            AppSim_Restart_Imediate();
            break;
        case _SIM_COMM_HTTP_UPDATE_FAIL:
            sMessage.aMESS_PENDING[SEND_UPDATE_FIRM_FAIL] = TRUE;
            AppSim_Restart_Imediate();
            break;
        case _SIM_COMM_FTP_ACCOUNT:   
            AppSim_FTP_Account();
            break;
        case _SIM_COMM_AUDIO_OK:
            //Hoan thanh cuoc goi canh bao
            sAppSimVar.sCall.Mark_u8 = false;
            sAppSimVar.sCall.Pending_u8 = false;
            break;
        default: 
            break;
    }
}

   


/*
    Func: _Cb Get sTime From AT cmd
        + Input: sTime
        + Active Log TSVH for the first Get sTime
*/

void _CbAppSim_Recv_sTime (ST_TIME_FORMAT *sClock)
{    
    if (sAppSimVar.pReq_Set_sTime != NULL)
    {
        sAppSimVar.pReq_Set_sTime(sClock, 7);
    }
}

/*
    Func: _cb Publish AT cmd
        + Set Status Send OK
        + Increase Index send
        + Active Event Send Mess
*/

void _CbAppSim_Recv_PUBACK (void)
{
    sMessage.Status_u8 = TRUE; 

    switch (sMessage.Type_u8)
    {
        case DATA_TSVH_MULTI:
        case DATA_TSVH_FLASH:
        case DATA_TSVH_OPERA:
            Mem_Inc_Index_Send(&sRecTSVH, 1);
            mReset_Raw_Data(); 
            break;
        case DATA_EVENT:
            Mem_Inc_Index_Send(&sRecEvent, 1);
            mReset_Raw_Data();
            break;
        case DATA_GPS:
            Mem_Inc_Index_Send(&sRecGPS, 1);
            mReset_Raw_Data();
            break;
        case SEND_SAVE_BOX_OK:
            sMessage.aMESS_PENDING[sMessage.Type_u8] = FALSE; 
            
            if (sAppSimVar.pReq_Save_Box != NULL)
                sAppSimVar.pReq_Save_Box();
            break;
        case SEND_SHUTTING_DOWN:
            if (sAppSimVar.pReset_MCU_Imediate != NULL)
                sAppSimVar.pReset_MCU_Imediate();
            break;
        case SEND_UPDATE_FIRM_OK:
        case SEND_UPDATE_FIRM_FAIL:
            sAppSimVar.IsFwUpdateSuccess_u8 = TRUE;          
            sMessage.aMESS_PENDING[sMessage.Type_u8] = FALSE; 
            
            if (sAppSimVar.pReset_MCU_Imediate != NULL)
                sAppSimVar.pReset_MCU_Imediate();
            break;
        default:
            sMessage.aMESS_PENDING[sMessage.Type_u8] = FALSE;  
            break;
    }
            
    fevent_active( sEventAppSim, _EVENT_SIM_SEND_MESS);
    sAppSimVar.cHardReset_u16 = 0;
    sAppSimVar.cSoftReset_u8 = 0;
    sAppSimVar.LandMarkConnMQTT_u32 = RtCountSystick_u32;
}
    


void AppSim_Process_Server_DL (sData *sUartSim)
{
    uint8_t var = 0;
    int PosFind = 0;
    
    for (var = REQUEST_RESET; var < END_MQTT_RECEI; ++var)
    {
        PosFind = Find_String_V2 ((sData*) &sMessRecv[var].sKind, sUartSim);
        
        if ((PosFind >= 0) && (sMessRecv[var].CallBack != NULL))
            sMessRecv[var].CallBack(sUartSim, PosFind);
    }
}

static void AppSim_Process_Sms (sData *sUartSim)
{
    UTIL_Printf_Str( DBLEVEL_M, "u_app_sim: request at by sms: \r\n" );
    
    if (sAppSimVar.pCheck_AT_User != NULL)
        sAppSimVar.pCheck_AT_User(sUartSim, _UART_SIM);
}


void _CbAppSim_SMS_Send_1 (void)
{
    //Sdt
    Sim_Common_Send_AT_Cmd( (uint8_t *) "979156872", strlen("979156872") );
}

void _CbAppSim_SMS_Send_2 (void)
{
    //content
    Sim_Common_Send_AT_Cmd( (uint8_t *) "test sms alarm!", strlen("test sms alarm!") );
}

void _CbAppSim_SMS_Send_3 (void)
{
    //Send SMS OK: unmark, pending = false
    sAppSimVar.sSms.Mark_u8 = false;
    sAppSimVar.sSms.Pending_u8 = false;
}


void _CbAppSim_Calling_1 (void)
{
    Sim_Common_Send_AT_Cmd( (uint8_t *) "0979156872", strlen("0979156872") );
}

void _CbAppSim_Calling_2 (void)
{
    //Da thiet lap dc cuoc goi: push lenh
}


/*
    Func: Process at: AT+CIPSEND=cid,length...
        + Send: Cid + length Data
*/

void _CbAppSim_TCP_Send_1 (sData *pData)
{
    char aTEMP[40] = {0};
    
    aTEMP[0] = CID_SERVER; 
    aTEMP[1] = ',';
    
    UtilIntToString(pData->Length_u16, aTEMP + strlen(aTEMP));
    
    Sim_Common_Send_AT_Cmd((uint8_t *) aTEMP, strlen(aTEMP));
}



void _CbAppSim_Http_Head_1 (sData *pData)
{
#ifdef HTTP_PROTOCOL
    char aDATA_TEMP[40] = {0};

    if (sSimCommVar.HttpHeaderIndex_u8 >= HTTP_HEADER_MAX)
    {
        sSimCommVar.HttpHeaderIndex_u8 = 0;
    }
    
    switch (sSimCommVar.HttpHeaderIndex_u8)
    {
        case 0:
            UtilIntToString(pData->Length_u16, aDATA_TEMP);
            Sim_Common_Send_AT_Cmd((uint8_t *) "content-length,", 15); 
            Sim_Common_Send_AT_Cmd((uint8_t *) aDATA_TEMP, strlen(aDATA_TEMP));
            break;
        case 1:
            Sim_Common_Send_AT_Cmd((uint8_t *) "content-type,application/json", strlen("content-type,application/json"));
            break;
        case 2:
            Sim_Common_Send_AT_Cmd((uint8_t *) "host,n8n.ilotusland.com", strlen("host,n8n.ilotusland.com")); 
            break;
        default:
            Sim_Common_Send_AT_Cmd((uint8_t *) "content-type,application/json", strlen("content-type,application/json")); //  "api-key": "123456"
            break;
    }
  
    sSimCommVar.State_u8 = _SIM_CONN_MQTT;    
#endif
}


void _CbAppSim_Http_Head_2 (void)
{
#ifdef HTTP_PROTOCOL
    uint8_t StepSet = 0;;
    uint8_t Stepcurr = 0;
    
    sSimCommVar.HttpHeaderIndex_u8++;
    
    if (sSimCommVar.HttpHeaderIndex_u8 < HTTP_HEADER_MAX)
    {
        Stepcurr = Sim_Get_Step_From_Queue(0);
        
        //Tien them 1 step
        Sim_Get_Step_From_Queue(1);
        
        StepSet = aSIM_STEP_HTTP_HEAD[1];
        qQueue_Send(&qSimStep, (uint8_t *) &StepSet, _TYPE_SEND_TO_HEAD);
        StepSet = aSIM_STEP_HTTP_HEAD[0];
        qQueue_Send(&qSimStep, (uint8_t *) &StepSet, _TYPE_SEND_TO_HEAD);
        //tra lai step cu
        StepSet = Stepcurr;
        qQueue_Send(&qSimStep, (uint8_t *) &StepSet, _TYPE_SEND_TO_HEAD);
    }
#endif
}




void _CbAppSim_Http_Send_1 (sData *pData)
{
#ifdef HTTP_PROTOCOL
    char aDATA_TEMP[40] = {0};
    
    UtilIntToString(pData->Length_u16, aDATA_TEMP);
    
    Sim_Common_Send_AT_Cmd(aDATA_TEMP, strlen(aDATA_TEMP));
#endif
}


void _CbAppSim_Http_Send_Ex (sData *pData)
{
#ifdef HTTP_PROTOCOL
    char aDATA_TEMP[40] = {0};
    
    UtilIntToString(pData->Length_u16, aDATA_TEMP);
    
    Sim_Common_Send_AT_Cmd(aDATA_TEMP, strlen(aDATA_TEMP));
    Sim_Common_Send_AT_Cmd((uint8_t *) ",\"", 2);
    Sim_Common_Send_AT_Cmd((uint8_t *) pData->Data_a8, pData->Length_u16);
    Sim_Common_Send_AT_Cmd((uint8_t *) "\"", 1);
#endif
}



static void AppSim_Process_URC (uint8_t Type)
{
    uint8_t StepHandle = 0;
    
    switch (Type)
    {
        case _SIM_COMM_URC_RESET_SIM: 
            //Print power down
            UTIL_Log_Str (DBLEVEL_M, "u_app_sim: URC POWER DOWN\r\n" );
            fevent_active(sEventSim, _EVENT_SIM_AT_SEND_TIMEOUT);
            
            sSimVar.Retry_u8 = SIM_CMD_RETRY;
            break;
        case _SIM_COMM_URC_SIM_LOST: 
            //Sim Remove
            UTIL_Log_Str (DBLEVEL_M, "u_app_sim: URC SIM LOSST\r\n" );
            fevent_active(sEventSim, _EVENT_SIM_AT_SEND_TIMEOUT);
            sSimVar.Retry_u8 = SIM_CMD_RETRY;
            break;
        case _SIM_COMM_URC_CLOSED: 
            //Push lenh dong ket noi va Khoi dong lai
            UTIL_Log_Str (DBLEVEL_M, "u_app_sim: URC TCP CLOSE\r\n" );

            //Trong truong h?p ngat nguon: power off dot ngot: -> k cho no la tcp close
            if (sSimCommVar.State_u8 != _SIM_POWER_OFF) 
            {
                fevent_active(sEventSim, _EVENT_SIM_AT_SEND_TIMEOUT);  
                sSimVar.Retry_u8 = SIM_CMD_RETRY;
            }
            break;
        case _SIM_COMM_URC_CALLING: 
            
            break;
        case _SIM_COMM_URC_CALL_FAIL: 
            StepHandle = Sim_Get_Step_From_Queue(0);      
            if (StepHandle == SIM_STEP_CALLING)
                fevent_active(sEventSim, _EVENT_SIM_AT_SEND_TIMEOUT);
            
            break;
        case _SIM_COMM_URC_SMS_CONTENT: 
            //Check at cmd trong noi dung sms
            AppSim_Process_Sms (&sUartSim);
            break;    
        case _SIM_COMM_URC_SMS_IND: 
            //push at cmd read sms
            Sim_Push_Block_To_Queue(aSIM_BLOCK_STEP_READ_SMS); 
            break;                     
        case _SIM_COMM_URC_ERROR: 
            if (sSimCommVar.State_u8 != _SIM_POWER_OFF)
            {
                UTIL_Printf_Str ( DBLEVEL_M, "u_app_sim: URC ERROR\r\n" );
                
                StepHandle = Sim_Get_Step_From_Queue(0);
                
                if (SIM_IS_STEP_SKIP_ERROR(StepHandle) == true) {
                    fevent_active(sEventSim, _EVENT_SIM_AT_SEND_OK );
                } else {
                    if (StepHandle != _EC200_AT_GET_ID) {
                        fevent_active(sEventSim, _EVENT_SIM_AT_SEND_TIMEOUT);
                    }
                }
            }
            break;
        case _SIM_COMM_URC_RECEIV_SERVER: 
            AppSim_Process_Server_DL(&sUartSim);
            break;
        case _SIM_COMM_URC_ATTACH: 
            Sim_Process_Attach();
            break; 
        default:
            break;
    }
}


static void AppSim_Process_Recv (uint8_t *pdata, uint16_t length)
{

}

/*
    Func: get location value
*/
void AppSim_GPS_OK (uint8_t *pData, uint16_t Length)
{

}

/*
    Func: tach gia tri lat va long tu chuoi gps nhan dc tu module sim
        + Format: format: (-)dd.ddddd,(-)ddd.ddddd
        + Ex: 
*/
uint8_t AppSim_GPS_Extract_Lat_Long (uint8_t *pData, uint16_t Length, double *Lat, double *Long)
{
#ifdef SIM_EC200U_LIB
    uint16_t    i = 0;
    uint32_t	TempU32 = 0;
    uint8_t 	Decimal = 0;
    uint8_t		MarkDecimal = false;
         
    double 		TempFloat[3] = {0};
    uint8_t 	Index = 0;
    uint8_t 	Sign[3] = {false, false};
    uint8_t     Status[3] = {false, false};

    //Cho them 1 vi tri: de tao so ket thuc
    for (i = 0; i < (Length + 1) ; i++)
    {
        if ( (pData[i] == ',') || (i == Length) )
        {
            //Get Value
            Status[Index] = true; 
            
            if (Sign[Index] == true)
            {
                TempFloat[Index] = 0 - TempU32 / ( pow (10, Decimal) ) ;
            } else
            {
                TempFloat[Index] = TempU32 / ( pow (10, Decimal) ) ;
            }
            //Tang index
            Index++;	
            
            //mem set variable
            Decimal = 0;
            MarkDecimal = false;
            TempU32 = 0;
            
            continue;
        }
        
        //Danh dau so am
        if (pData[i] == '-')
        {
            Sign[Index] = true;	
            continue;
        } 

        //Danh dau vi tri decimal
        if (pData[i] == '.')
        {
            MarkDecimal = true;
            continue;
        }

        //Increase count decimal
        if (MarkDecimal == true)
        {
            Decimal++;
        }
        
        //Value
        if ( (pData[i] < 0x30) || ( pData[i] > 0x39 ) )     
        {
            return false;
        }
        
        TempU32 = TempU32 * 10 + pData[i] - 0x30;
    }
    
    //Kiem tra gia tri lay ra
    if ((Status[0] == true) && (Status[1] == true) )
    {
        *Lat = TempFloat[0];
        *Long = TempFloat[1];
        
        return true;
    }
    
#endif
    
    return false;
}



/*
    Func: Kiem tra khoang cach giua cac diem gps voi diem goc
        + Neu qua 1 nua so diem: co khoang cach >50m
            -> xe dang chay: Copy het gps vao buff -> Push mess
        + Neu < 1 nua so diem: co khoang cach > 50m
            -> xe k di chuyen: tinh trung binh

        return: 0xFF: xe dang di chuyen -> Dong goi toan bo data gps nhuu thu thap
                Vi tri gan nhat voi trung binh: -> dong goi toan bo data la diem gan nhat do
*/


uint8_t AppSim_GPS_Check_Moving (void)
{
#ifdef SIM_EC200U_LIB
    double Distance[20] = {0};
    uint16_t i = 0;
    uint8_t CountMore50m = 0;
    uint8_t CountPosError = 0;
    double DistAverage = 0, TempDist = 0, MinDist = 0;
    uint8_t PosFind = 0;
    
    //Lan luot tinh toan khoang cach
    for (i = 0; i < sAppSimVar.sGPS.Index_u8 ; i++)
    {
        if ( (sAppSimVar.sGPS.sLocation[i].Lat == 0) && (sAppSimVar.sGPS.sLocation[i].Long == 0) )
        {
            Distance[i] = -1;   //Nhung diem error danh dau gia tri dis = -1
            CountPosError++;
            continue;
        }
        
        Distance[i] = 1000 * UTIL_Cacul_Distance(sAppSimVar.sGPS.sLocation[i].Lat, sAppSimVar.sGPS.sLocation[i].Long, 
                                                 sAppSimVar.sGPS.sLocaOrigin.Lat, sAppSimVar.sGPS.sLocaOrigin.Long, 'K'); 
    }
    
    //so sanh
    for (i = 0; i < sAppSimVar.sGPS.Index_u8; i++)
    {
        if (Distance[i] > DISTANCE_GPS_MOVE)
            CountMore50m++;
    }
    
    
    if ( CountMore50m > ( (sAppSimVar.sGPS.Index_u8 - CountPosError) / 2) )   
    {
        //Neu qua nua vi tri > 50m: Xem dang di chuyen
        return 0xFF;   
    } else
    {
        //Neu chua dc 1 nua vi tri > 50m. Xe dang dung yen: 
        // Tinh trung binh va tim vi tri gan nhat de dong goi ban tin
        for (i = 0; i < sAppSimVar.sGPS.Index_u8 ; i++)
        {
            if (Distance[i] == -1)
                continue;
            
            DistAverage += Distance[i];
        }
        DistAverage = DistAverage / (sAppSimVar.sGPS.Index_u8 - CountPosError);
        
        //Tim gia tri gan voi gia tri trung binh    
        MinDist = DistAverage;
        for (i = 0; i < sAppSimVar.sGPS.Index_u8 ; i++)
        {
            if (Distance[i] == -1)
            {
                continue;
            }
            //Tinh khoang c�ch voi gia tri trung binh
            if (DistAverage > Distance[i])
            {
                TempDist = DistAverage - Distance[i];
            } else
            {
                TempDist = Distance[i] - DistAverage;
            }
            
            //Check xem co phai gia tri nho nhat k?
            if (TempDist < MinDist)
            {
                MinDist = TempDist;
                PosFind = i;
            }
        }
        
        return PosFind;
    }
#else
    return 1;
#endif
}

/*
    Func: Dong goi GPS Record to save
*/

void AppSim_GPS_Packet_Record (uint8_t CheckResult)
{

}

/*
    Func: callback gps error  : Packet lat and long 0,0
        - Neu lan tiep theo van loi: k lay du lieu 0,0
        - util next gps ok:
*/
void AppSim_GPS_Error (void)
{

}

void AppSim_Config_Update (void)  
{
    Erase_Firmware(ADDR_UPDATE_PROGRAM, FIRMWARE_SIZE_PAGE); 
    sSimFwUpdate.AddSave_u32 = ADDR_UPDATE_PROGRAM;
}


/*
    Func: Restart module sim neu nhu sim dang o che do PSM
*/
void AppSim_Restart_If_PSM(void)
{
    //Check them thoi gian hoạt dong
    if ( ( sSimCommVar.State_u8 == _SIM_POWER_OFF )
        || (Check_Time_Out(sSimVar.LandMark_u32, 120000) == true) )
    {
        sSimVar.LandMark_u32 = RtCountSystick_u32;
        sSimCommVar.State_u8  = _SIM_START;
        fevent_active(sEventAppSim, _EVENT_SIM_TURN_ON);
    }
    
    //neu dang psm-> wakeup
    if ( ( sSimCommVar.State_u8 == _SIM_PSM )
        || (Check_Time_Out(sSimVar.LandMark_u32, 120000) == true) )
    {
        sSimVar.LandMark_u32 = RtCountSystick_u32;
        fevent_active(sEventAppSim, _EVENT_SIM_WAKEUP);
    }
}

/*
    Func: Restart Module sim send data
*/
void AppSim_Restart_Imediate (void)
{
    if (sAppSimVar.StepPowerOn_u8 == 0) {
        sSimCommVar.State_u8 = _SIM_START;
        fevent_active(sEventAppSim, _EVENT_SIM_TURN_ON);
        Sim_Clear();
    }
}

void AppSim_FTP_Account (void)
{
    char aTEMP[128] = {0};
  
    sprintf(aTEMP, "\"%s\",\"%s\"", sModemInfor.sServerTn.sServer.aUSER, sModemInfor.sServerTn.sServer.aPASS); 
    
    Sim_Common_Send_AT_Cmd( (uint8_t *) aTEMP, strlen(aTEMP) );
}

/*
    Func:Power ON module SIM
        + Giu chan Boot va Reset o muc Thap
        + Cap nguon cho module bang chan POW
*/

/*
    Func:Power ON module SIM
        + Giu chan Boot va Reset o muc Thap
        + Cap nguon cho module bang chan POW
*/
uint8_t AppSim_Power_On(void)
{   
	switch (sAppSimVar.StepPowerOn_u8)
	{
		case 0:     
            //Reinit uart
            Sim_Reinit_Uart();
            
            if (sAppSimVar.iOnPower_u8 == true) {
                UTIL_Log_Str(DBLEVEL_M, "u_app_sim: power off...\r\n" );  
    
                SIM_PW_OFF1;
                
                UTIL_TIMER_SetPeriod(&TimerPowerOn, ((sAppSimVar.cHardReset_u16 % 10) * 1000) + 3000);
                UTIL_TIMER_Start(&TimerPowerOn);
            } else {
                fevent_active(sEventAppSim, _EVENT_SIM_TURN_ON);
            }
			break;
        case 1:   
            SIM_PWKEY_OFF1;
            SIM_RESET_OFF1;
            SIM_PW_ON1;
            UTIL_Log_Str(DBLEVEL_M, "u_app_sim: power on...\r\n" );  
            
			UTIL_TIMER_SetPeriod(&TimerPowerOn, 1000);
            UTIL_TIMER_Start(&TimerPowerOn);
			break;
        case 2:   
            sAppSimVar.iOnPower_u8 = true;
            SIM_PWKEY_ON1;
        #ifdef SIM_L506_LIB
			UTIL_TIMER_SetPeriod(&TimerPowerOn, 500);
            UTIL_TIMER_Start(&TimerPowerOn);
        #endif
            
        #ifdef SIM_EC200U_LIB
            UTIL_TIMER_SetPeriod(&TimerPowerOn, 3000);
            UTIL_TIMER_Start(&TimerPowerOn);
        #endif
            
        #ifdef SIM_L511_LIB
            UTIL_TIMER_SetPeriod(&TimerPowerOn, 1500);
            UTIL_TIMER_Start(&TimerPowerOn);
        #endif
			break;
        case 3:      
            SIM_PWKEY_OFF1;
        #ifdef SIM_L506_LIB
			UTIL_TIMER_SetPeriod(&TimerPowerOn, 15000);
            UTIL_TIMER_Start(&TimerPowerOn);
        #endif
            
        #ifdef SIM_EC200U_LIB
            fevent_active(sEventAppSim, _EVENT_SIM_TURN_ON);
        #endif
            
        #ifdef SIM_L511_LIB
            UTIL_TIMER_SetPeriod(&TimerPowerOn, 10000);
            UTIL_TIMER_Start(&TimerPowerOn);
        #endif
			break;
		default:
			sAppSimVar.StepPowerOn_u8 = 0;
            UTIL_Printf_Str( DBLEVEL_M, "\r\nu_app_sim: power on success!\r\n" );
			return 1;
	}

	sAppSimVar.StepPowerOn_u8++;

	return 0;
}

static void OnTimerPowerOn(void *context)
{
	fevent_active(sEventAppSim, _EVENT_SIM_TURN_ON);
}





static void AppSim_AT_Faillure (uint8_t Step)
{
    //Neu dang poweron module thi bo qua
    if (sAppSimVar.StepPowerOn_u8 != 0)
        return;
    
    Sim_Stop_RX_Mode();
    HAL_UART_DeInit(&uart_sim);
    Sim_Init_Uart();
    Sim_Init_RX_Mode();
    //Tang soft Reset
	sAppSimVar.cSoftReset_u8++;
    
	Sim_Clear();
    
    switch (UTIL_var.ModeConnNow_u8)
    {
        case _CONNECT_FTP:
        case _CONNECT_HTTP:
            sAppSimVar.cSoftReset_u8 = MAX_SOFT_RESET;
            break;
    }

    if (Step == SIM_STEP_ATTACH) {
        sAppSimVar.cSoftReset_u8 = MAX_SOFT_RESET;
    }
    
	//Neu Soft Reset > 2
	if (sAppSimVar.cSoftReset_u8 < MAX_SOFT_RESET)
	{        
		switch (sSimCommVar.State_u8)
		{
			case _SIM_INIT:
			case _SIM_NETWORK:
			case _SIM_SETTING:
				Sim_Push_Block_To_Queue(aSIM_BLOCK_STEP_INIT);
                Sim_Push_Block_To_Queue(aSIM_BLOCK_STEP_NETWORK);
                if (sSimCommVar.iGetClock_u8 == false) {
                    Sim_Push_Block_To_Queue(aSIM_BLOCK_STEP_CLOCK); 
                }
                Sim_Push_Block_To_Queue(aSIM_BLOCK_STEP_CONNECT);
                sSimCommVar.State_u8 = _SIM_INIT; 
				break;
			case _SIM_INIT_TCP:
                if (UTIL_var.ModeConnNow_u8 == _CONNECT_FTP_UPLOAD ) {
                    Sim_Push_Block_To_Queue(aSIM_BLOCK_STEP_INIT);
                    Sim_Push_Block_To_Queue(aSIM_BLOCK_STEP_NETWORK);
                    if (sSimCommVar.iGetClock_u8 == false) {
                        Sim_Push_Block_To_Queue(aSIM_BLOCK_STEP_CLOCK); 
                    }
                    Sim_Push_Block_To_Queue(aSIM_BLOCK_STEP_FTP_CONN);
                    sSimCommVar.State_u8 = _SIM_INIT; 
                } else {
                    Sim_Push_Block_To_Queue(aSIM_BLOCK_STEP_INIT);
                    Sim_Push_Block_To_Queue(aSIM_BLOCK_STEP_NETWORK);
                    if (sSimCommVar.iGetClock_u8 == false) {
                        Sim_Push_Block_To_Queue(aSIM_BLOCK_STEP_CLOCK); 
                    }
                    Sim_Push_Block_To_Queue(aSIM_BLOCK_STEP_CONNECT);
                    sSimCommVar.State_u8 = _SIM_INIT; 
                    UTIL_Log_Str(DBLEVEL_M, "u_modem: change server >>> \r\n" );
                    Modem_Change_Server();
                }
                break;
			case _SIM_OPENED_TCP:
            case _SIM_CONN_MQTT:
                if (UTIL_var.ModeConnNow_u8 == _CONNECT_FTP_UPLOAD ) {
                    Sim_Push_Block_To_Queue(aSIM_BLOCK_STEP_FTP_DISCONN);
                    Sim_Push_Block_To_Queue(aSIM_BLOCK_STEP_FTP_CONN);
                    sSimCommVar.State_u8 = _SIM_INIT_TCP; 
                } else {
                    sSimCommVar.ServerReady_u8 = false;
                    Sim_Push_Block_To_Queue(aSIM_BLOCK_STEP_DIS_CONN);
                    Sim_Push_Block_To_Queue(aSIM_BLOCK_STEP_CONNECT);
                    sSimCommVar.State_u8 = _SIM_INIT_TCP; 
                    //chuyen server ơ buoc nay
                    UTIL_Log_Str(DBLEVEL_M, "u_modem: change server >>> \r\n" );
                    Modem_Change_Server();
                }
				break;
            case _SIM_CONN_HTTP:
                Sim_Clear();
                fevent_active(sEventAppSim, _EVENT_SIM_TURN_ON); 
                sSimCommVar.State_u8 = _SIM_INIT; 
                break;
			default:
				break;
		}
	} else
	{
        sAppSimVar.cSoftReset_u8 = 0;
        sAppSimVar.cHardReset_u16++;
        
        //Neu dang Case Online ->Pow on lai | 
        //Neu case Save Mode -> Pow off module sim | PSM
        if (UTIL_var.ModePower_u8 == _POWER_MODE_SAVE) {
        #ifdef SIM_BC660_LIB  //van psm neu qua dc attach
            if (sSimCommVar.State_u8 >= _SIM_SETTING) {
                fevent_active(sEventAppSim, _EVENT_SIM_PSM);
            } else {
                fevent_active(sEventAppSim, _EVENT_SIM_POWER_OFF);
            }
        #else
            if ( (UTIL_var.ModeConnNow_u8 != _CONNECT_DATA_MAIN) 
                && (UTIL_var.ModeConnNow_u8 != _CONNECT_DATA_BACKUP)
                && (UTIL_var.ModeConnNow_u8 != _CONNECT_DATA_BACKUP) ) {
                fevent_active(sEventAppSim, _EVENT_SIM_TURN_ON);   
            } else {
                fevent_active(sEventAppSim, _EVENT_SIM_POWER_OFF);
            }
        #endif
        } else {
            fevent_active(sEventAppSim, _EVENT_SIM_TURN_ON);
        }
        sSimCommVar.State_u8 = _SIM_INIT; 
	}
}



static void AppSim_Uart_Send (uint8_t *pData, uint16_t length)
{
    HAL_UART_Transmit(&uart_sim, pData, length, 1000);
}



#ifdef USING_AT_CONFIG

void AppSim_SER_Get_Sim_ID (sData *str, uint16_t Pos)
{
    if (sAppSimVar.pRespond_Str != NULL) {
        sAppSimVar.pRespond_Str(PortConfig, sSimCommInfor.aSIM_ID, 0);
    } else {
        sAppSimVar.pRespond_Str(PortConfig, "\r\nERROR\r\n", 0);
    }
}

#endif


void AppSim_Set_Instant (void)
{
    sAppSimVar.rExternIrq_u8 = true;
}


void AppSim_Allow_Send(uint8_t allow)
{
    sSimVar.IsPause_u8 = allow;
}

void AppSim_Default_Sms_Call(void)
{
    sAppSimVar.sSms.Pending_u8 = false;
    sAppSimVar.sCall.Pending_u8 = false;
}


uint8_t AppSim_Need_Sms (void)
{
    if (sAppSimVar.sSms.Mark_u8 == true)
        return true;
    
    return false;
}


void AppSim_Push_Send_Sms (void)
{
    //check xem co ban tin nao can gui
    sAppSimVar.sSms.Pending_u8 = true;
    sAppSimVar.sSms.Type_u8 = 1;
    //Push block send sms
    Sim_Push_Block_To_Queue(aSIM_BLOCK_STEP_SEND_SMS);
}


void AppSim_Push_Call (void)
{
    //check xem co ban tin nao can gui
    sAppSimVar.sCall.Pending_u8 = true;
    //Push block send sms
    Sim_Push_Block_To_Queue(aSIM_BLOCK_STEP_CALL);
}


uint8_t AppSim_Wake_Up (void)
{
	static uint8_t step = 0;

	switch(step)
	{
		case 0:
            UTIL_Printf_Str( DBLEVEL_M, "u_app_sim: wakeup...\r\n" );
			sSimCommVar.Wakeup_u8 = false;
			SIM_WAKE_LOW; 
            sEventAppSim[_EVENT_SIM_WAKEUP].e_period = 50;
			break;
		case 1:
			SIM_WAKE_HI;
			break;
		case 2:
			SIM_WAKE_LOW;
			break;
		case 3:
			SIM_WAKE_HI;
			break;
		case 4:
            SIM_WAKE_LOW;
//		case 5:
//		case 6:
//		case 7:
//			//Cho day 2000 s de check xem wakeup OK không
//			if (sSimCommVar.Wakeup_u8 == true)
//				step += 4;
//
//			sEventAppSim[_EVENT_SIM_WAKEUP].e_period = 500;
			break;
		default:
			step = 0;
			return 1;
	}

	step++;

	return pending;
}




void AppSim_Test_FTP_Init (void)
{
    //init IP port user pass
    UTIL_MEM_set(&sModemVar.sFwUpdate, 0, sizeof(sModemVar.sFwUpdate));

    if (strlen(sModemInfor.sServerTn.sServer.aIP) == 0) {
        UTIL_MEM_cpy(sModemVar.sFwUpdate.sServer.aIP, "124.158.5.154", strlen("124.158.5.154") );
        UTIL_MEM_cpy(sModemVar.sFwUpdate.sServer.aPORT, "21", strlen("21") );
        UTIL_MEM_cpy(sModemVar.sFwUpdate.sServer.aUSER, "admin", strlen("admin") );
        UTIL_MEM_cpy(sModemVar.sFwUpdate.sServer.aPASS, "admin", strlen("admin") );
        
        UTIL_MEM_cpy(sModemVar.sFwUpdate.aPATH, "chien/test_tn", strlen("chien/test_tn") );
    } else {
        UTIL_MEM_cpy(&sModemVar.sFwUpdate, &sModemInfor.sServerTn, sizeof(sModemInfor.sServerTn) );
    }
    
////    ftp://103.9.86.28/HY/{8D7CEA5C-8DC6-4E17-8958-8633881E9A39}/{9359354D-FC37-43F3-83B8-F379DB2ABBE9}  bitexconamlong / cnUG8HENGj
//      
//    UTIL_MEM_cpy(sModemVar.sFwUpdate.sServer.aIP, "103.9.86.28", strlen("103.9.86.28") );
//    UTIL_MEM_cpy(sModemVar.sFwUpdate.sServer.aPORT, "21", strlen("21") );
//    UTIL_MEM_cpy(sModemVar.sFwUpdate.sServer.aUSER, "bitexconamlong", strlen("bitexconamlong") );
//    UTIL_MEM_cpy(sModemVar.sFwUpdate.sServer.aPASS, "cnUG8HENGj", strlen("cnUG8HENGj") );
//
////    UTIL_MEM_cpy( sModemVar.sFwUpdate.aPATH, "HY/{8D7CEA5C-8DC6-4E17-8958-8633881E9A39}/{9359354D-FC37-43F3-83B8-F379DB2ABBE9}",
////                 strlen("HY/{8D7CEA5C-8DC6-4E17-8958-8633881E9A39}/{9359354D-FC37-43F3-83B8-F379DB2ABBE9}") );
}



uint8_t AppSim_Set_FileName_FTP (void)
{
    char File_Name[MAX_LENGTH_FILE_NAME] = {0};
    int8_t chann = 0;
    uint32_t length = 0;
//    char File_Name[MAX_LENGTH_FILE_NAME] = "/HY/{8D7CEA5C-8DC6-4E17-8958-8633881E9A39}/{9359354D-FC37-43F3-83B8-F379DB2ABBE9}/SV_HOANGNGOC_";
    
    switch (sWmVar.ModePacket_u8) 
    {
        case 0:
            chann = AppWm_Is_Firt_Chann_TN();
            if (chann >= 0) {
                length = strlen(sWmVar.sChannInfor[chann].MA_TINH) + strlen(sWmVar.sChannInfor[chann].MA_CTRINH) 
                                        + strlen(sWmVar.sChannInfor[chann].MA_TRAM) + strlen(AppWm_TN_PARAM[ sWmVar.sChannInfor[chann].aPARAM[0] ]); 
                
                if (length < MAX_LENGTH_FILE_NAME ) {
                    sprintf(File_Name, "%s_%s_%s_%s_", sWmVar.sChannInfor[chann].MA_TINH, sWmVar.sChannInfor[chann].MA_CTRINH,
                                                           sWmVar.sChannInfor[chann].MA_TRAM, AppWm_TN_PARAM[ sWmVar.sChannInfor[chann].aPARAM[0] ] );
                }
            }
            break;
        case 1:
            chann = AppWm_Is_Firt_Chann_TN();
            if (chann >= 0) {
                length = strlen(sWmVar.sChannInfor[chann].MA_TINH) 
                        + strlen(sWmVar.sChannInfor[chann].MA_CTRINH) + strlen(sWmVar.sChannInfor[chann].MA_TRAM); 
                
                if (length < MAX_LENGTH_FILE_NAME ) {
                    sprintf(File_Name, "%s_%s_%s_", sWmVar.sChannInfor[chann].MA_TINH, 
                                        sWmVar.sChannInfor[chann].MA_CTRINH, sWmVar.sChannInfor[chann].MA_TRAM);
                }
            }
            break;
        default:
            chann = AppWm_Is_Firt_Chann_TN();
            if (chann >= 0) {
                length = strlen(sWmVar.sChannInfor[chann].MA_TINH) + strlen(sWmVar.sChannInfor[chann].MA_CTRINH); 
                
                if (length < MAX_LENGTH_FILE_NAME ) {
                    sprintf(File_Name, "%s_%s_", sWmVar.sChannInfor[chann].MA_TINH, sWmVar.sChannInfor[chann].MA_CTRINH);
                }
            }
            break;
    }

    if (strlen(File_Name) == 0) {
        return false;
    }
    
    sprintf(File_Name + strlen(File_Name), "%04d%02d%02d%02d%02d%02d.txt", sRTC.year + 2000, sRTC.month, sRTC.date,
                                                    sRTC.hour, sRTC.min, sRTC.sec  );
    //init lai file name, path
    UTIL_MEM_set(sModemVar.sFwUpdate.aFILE_NAME, 0, sizeof(sModemVar.sFwUpdate.aFILE_NAME));
    UTIL_MEM_cpy(sModemVar.sFwUpdate.aFILE_NAME, File_Name, strlen(File_Name) );
    
    sSimFwUpdate.FirmLength_u32 = 0;
    
    return true;
}




                 





