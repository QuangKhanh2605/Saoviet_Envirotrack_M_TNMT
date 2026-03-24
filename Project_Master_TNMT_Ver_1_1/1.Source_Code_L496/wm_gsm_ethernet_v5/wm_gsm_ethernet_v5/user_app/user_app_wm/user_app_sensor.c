
#include "user_app_sensor.h"
#include "user_define.h"
#include "user_convert_variable.h"
#include "math.h"
#include "sd_functions.h"
#include "rtc.h"

/*============== Function static ===============*/
static uint8_t fevent_sensor_entry(uint8_t event);

static uint8_t fevent_sensor_data_measure(uint8_t event);
static uint8_t fevent_sensor_handle_state(uint8_t event);
/*=================== struct ==================*/
sEvent_struct               sEventAppSensor[] = 
{
  {_EVENT_SENSOR_ENTRY,              1, 5, 60000,                fevent_sensor_entry},
  
  {_EVENT_SENSOR_DATA_MEASURE,       1, 5, 500,                  fevent_sensor_data_measure}, 
  {_EVENT_SENSOR_HANDLE_STATE,       0, 5, 500,                  fevent_sensor_handle_state},
};
uint8_t DurationTimeWarningSensor = 0;

uint8_t _Cb_Handle_Detect_Power(uint8_t State);
uint8_t _Cb_Handle_SS_pH(uint8_t State);
uint8_t _Cb_Handle_SS_Clo(uint8_t State);
uint8_t _Cb_Handle_SS_EC(uint8_t State);
uint8_t _Cb_Handle_SS_Turb(uint8_t State);

uint8_t _Cb_Handle_SS_COD(uint8_t State);
uint8_t _Cb_Handle_SS_TSS(uint8_t State);
uint8_t _Cb_Handle_SS_NH4(uint8_t State);
uint8_t _Cb_Handle_SS_DO(uint8_t State);

uint8_t _Cb_Handle_SS_SALT(uint8_t State);
uint8_t _Cb_Handle_SS_TDS(uint8_t State);
uint8_t _Cb_Handle_SS_NO3(uint8_t State);

char AppSensor_TN_PARAM[_END_SENSOR][10] = 
{ 
    "TEMP",     // 0 Nhiet do
    "PH",       // 1 pH
    "CLO",      // 2 Clo
    "EC",       // 3 EC
    "TURB",     // 4 TURB
    "COD",      // 5 COD
    "TSS",      // 6 TSS
    "NH4_N",    // 7 NH4
    "DO",       // 8 DO
    "SALT",     // 9 SALT
    "TDS",      //10 TDS
    "NO3_N"     //11 NO3
};

char AppSensor_TN_UNIT[_END_SENSOR][10] = 
{ 
    "°C",    // 0 Nhiet do
    "-",      // 1 pH
    "mg/l",  // 2 Clo
    "µS/cm", // 3 EC
    "NTU",   // 4 TURB
    "mg/l",  // 5 COD
    "mg/l",  // 6 TSS
    "mg/l",  // 7 NH4
    "mg/l",  // 8 DO
    "%",     // 9 SALT
    "mg/l",  //10 TDS
    "mg/l"   //11 NO3
};

Struct_AverageOneHour       sAverageOneHour[_END_SENSOR] = {0};     //Struct tinh gia tri trung binh trong 1 gio
Struct_AverageMeasure       sAverageMeasure[_END_SENSOR] = {0};     //Struct luu gia tri trung binh
uint8_t StateCalibSensor = 0;

Struct_RS485_Measure            s485Measure[]=
{
  {_SS_TEMP},       // 0 Temperature
  {_SS_PH},         // 1 pH
  {_SS_CLO},        // 2 Clo
  {_SS_EC},         // 3 EC
  {_SS_TURB},       // 4 Turb
  {_SS_COD},        // 5 COD
  {_SS_TSS},        // 6 TSS
  {_SS_NH4},        // 7 NH4
  {_SS_DO},         // 8 DO
  {_SS_SALT},       // 9 SALT
  {_SS_TDS},        //10 TDS
  {_SS_NO3},        //11 NO3
};

Struct_SensorWarning        sSensorWarning[] = 
{
    //e_Name        State_Active             State_Connect_Now                   State_Connect_Befor    Gettick_Handle  _Cb_Handler_SS_Connect
  {_SS_TEMP,    NULL,                         NULL,                               _SENSOR_CONNECT,       0,               _Cb_Handle_Detect_Power},
  {_SS_PH,      &s485Measure[_SS_PH].sUser,   &s485Measure[_SS_PH].sConnect_u8,   _SENSOR_DISCONNECT,    0,               _Cb_Handle_SS_pH},
  {_SS_CLO,     &s485Measure[_SS_CLO].sUser,  &s485Measure[_SS_CLO].sConnect_u8,  _SENSOR_DISCONNECT,    0,               _Cb_Handle_SS_Clo},
  {_SS_EC,      &s485Measure[_SS_EC].sUser,   &s485Measure[_SS_EC].sConnect_u8,   _SENSOR_DISCONNECT,    0,               _Cb_Handle_SS_EC},
  {_SS_TURB,    &s485Measure[_SS_TURB].sUser, &s485Measure[_SS_TURB].sConnect_u8, _SENSOR_DISCONNECT,    0,               _Cb_Handle_SS_Turb},
  
  {_SS_COD,     &s485Measure[_SS_COD].sUser,  &s485Measure[_SS_COD].sConnect_u8,  _SENSOR_DISCONNECT,    0,               _Cb_Handle_SS_COD},
  {_SS_TSS,     &s485Measure[_SS_TSS].sUser,  &s485Measure[_SS_TSS].sConnect_u8,  _SENSOR_DISCONNECT,    0,               _Cb_Handle_SS_TSS},
  {_SS_NH4,     &s485Measure[_SS_NH4].sUser,  &s485Measure[_SS_NH4].sConnect_u8,  _SENSOR_DISCONNECT,    0,               _Cb_Handle_SS_NH4},
  {_SS_DO,      &s485Measure[_SS_DO].sUser,   &s485Measure[_SS_DO].sConnect_u8,   _SENSOR_DISCONNECT,    0,               _Cb_Handle_SS_DO},
  {_SS_SALT,    &s485Measure[_SS_SALT].sUser, &s485Measure[_SS_SALT].sConnect_u8, _SENSOR_DISCONNECT,    0,               _Cb_Handle_SS_SALT},
  
  {_SS_TDS,     &s485Measure[_SS_TDS].sUser,  &s485Measure[_SS_TDS].sConnect_u8,  _SENSOR_DISCONNECT,    0,               _Cb_Handle_SS_TDS},
  {_SS_NO3,     &s485Measure[_SS_NO3].sUser,  &s485Measure[_SS_NO3].sConnect_u8,  _SENSOR_DISCONNECT,    0,               _Cb_Handle_SS_NO3},
};

Struct_MeasureHanle         sMeasureHandle [] =
{
    //e_Name        sMeasureSensor            sMeasureHanlde              Scale_1  ParaScale  Scale_2
  {_SS_TEMP,    NULL,                        NULL,                         0x00,       0,      0x00},       //Temperature
  {_SS_PH,      &s485Measure[_SS_PH],        &sDataSensorMeasure.spH,      0xFE,       0,      0xFE},
  {_SS_CLO,     &s485Measure[_SS_CLO],       &sDataSensorMeasure.sClo,     0xFE,       0,      0xFE},
  {_SS_EC,      &s485Measure[_SS_EC],        &sDataSensorMeasure.sEC,      0x00,       0,      0x00},
  {_SS_TURB,    &s485Measure[_SS_TURB],      &sDataSensorMeasure.sTurb,    0xFE,       10,     0xFF},
  
  {_SS_COD,     &s485Measure[_SS_COD],       &sDataSensorMeasure.sCOD,     0xFE,       0,      0xFE},
  {_SS_TSS,     &s485Measure[_SS_TSS],       &sDataSensorMeasure.sTSS,     0xFE,       0,      0xFE},
  {_SS_NH4,     &s485Measure[_SS_NH4],       &sDataSensorMeasure.sNH4,     0xFE,       0,      0xFE},
  {_SS_DO,      &s485Measure[_SS_DO],        &sDataSensorMeasure.sDO,      0xFE,       0,      0xFE},
  {_SS_SALT,    &s485Measure[_SS_SALT],      &sDataSensorMeasure.sSal,     0xFE,       0,      0xFE},
  
  {_SS_TDS,     &s485Measure[_SS_TDS],       &sDataSensorMeasure.sTDS,     0x00,       0,      0x00},
  {_SS_NO3,     &s485Measure[_SS_NO3],       &sDataSensorMeasure.sNO3,     0xFE,       0,      0xFE},
};

//======================================================
static inline void clear_measure_block(Struct_SS_Value *blk)
{
    blk->State_u8 = 0;
    blk->Value_i32 = 0;
    blk->Scale_u8 = 0;
}

static inline void set_measure_block(Struct_SS_Value *blk, float value, float offset, uint8_t scale)
{
    float stamp = value + offset;
    float factor = Calculator_Scale(scale);

    blk->State_u8 = 1;
    blk->Value_i32 = (int32_t)((stamp >= 0) ? (stamp * factor) : 0);
    blk->Scale_u8 = scale;
}

/*================= Function Handle ==============*/
static uint8_t fevent_sensor_entry(uint8_t event)
{
    fevent_enable(sEventAppSensor, _EVENT_SENSOR_HANDLE_STATE);
    return 1;
}

static uint8_t fevent_sensor_data_measure(uint8_t event)
{
    float *ptemp = NULL;

    for(uint8_t i = _SS_TEMP; i<_END_SENSOR; i++)
    {
        if (sMeasureHandle[i].sMeasureSensor->nConnect_u8 == 0) 
        {                                
            sMeasureHandle[i].sMeasureSensor->sConnect_u8 = _SENSOR_DISCONNECT;             
            sMeasureHandle[i].sMeasureSensor->Temp_f = 0;
            sMeasureHandle[i].sMeasureSensor->Value_f = 0;
        } else if (sMeasureHandle[i].sMeasureSensor->nConnect_u8 >= 3) 
        {
            sMeasureHandle[i].sMeasureSensor->sConnect_u8 = _SENSOR_CONNECT;                
        }   
        
        if (sMeasureHandle[i].sMeasureSensor->sConnect_u8 == _SENSOR_CONNECT)
        {
            uint8_t sc = (sMeasureHandle[i].sMeasureSensor->Value_f <= sMeasureHandle[i].ParaScale) ? sMeasureHandle[i].Scale_1 : sMeasureHandle[i].Scale_2;
            set_measure_block(sMeasureHandle[i].sMeasureHanlde, sMeasureHandle[i].sMeasureSensor->Value_f, sMeasureHandle[i].sMeasureSensor->Offset_f, sc);
            if (ptemp == NULL) ptemp = &sMeasureHandle[i].sMeasureSensor->Temp_f;
        }
        else clear_measure_block(sMeasureHandle[i].sMeasureHanlde);
    }

    //TEMP
    if (ptemp != NULL)
        set_measure_block(&sDataSensorMeasure.sTemp, *ptemp, 0, 0xFE);
    else
        clear_measure_block(&sDataSensorMeasure.sTemp);

    fevent_enable(sEventAppSensor, event);
    return 1;
}

static uint8_t fevent_sensor_handle_state(uint8_t event)
{   
    static uint8_t i = _SS_TEMP;
    static uint8_t Power_User = _ACTIVE_SENSOR;
    static uint8_t Power_Connect = _SENSOR_CONNECT;
    
    sSensorWarning[_SS_TEMP].State_Active = &Power_User;
    sSensorWarning[_SS_TEMP].State_Connect_Now = &Power_Connect;
    
    if(sVout.mVol_u32 < 5000)
        *sSensorWarning[_SS_TEMP].State_Connect_Now = _SENSOR_DISCONNECT;
    else
        *sSensorWarning[_SS_TEMP].State_Connect_Now = _SENSOR_CONNECT;
    
    if(*sSensorWarning[i].State_Active == _ACTIVE_SENSOR)
    {
        if(*sSensorWarning[i].State_Connect_Now != sSensorWarning[i].State_Connect_Befor)
        {
            sSensorWarning[i].Gettick_Handle = HAL_GetTick();
            if(*sSensorWarning[i].State_Connect_Now == _SENSOR_DISCONNECT)
                sSensorWarning[i]._Cb_Handler_SS_Connect(*sSensorWarning[i].State_Connect_Now);
            
            sSensorWarning[i].State_Connect_Befor = *sSensorWarning[i].State_Connect_Now;
        }
        
        if(sSensorWarning[i].State_Connect_Now == _SENSOR_DISCONNECT)
        {
            if(HAL_GetTick() - sSensorWarning[i].Gettick_Handle >= DurationTimeWarningSensor*60000)
            {
                sSensorWarning[i].Gettick_Handle = HAL_GetTick();
                sSensorWarning[i]._Cb_Handler_SS_Connect(*sSensorWarning[i].State_Connect_Now);
            }
        }
    }
    else
    {
        sSensorWarning[i].Gettick_Handle = HAL_GetTick();
    }
    
    if(i+1 < _END_SENSOR)
        i++;
    else
        i = _SS_TEMP;
    
    if(sSensorWarning[_SS_TEMP].State_Connect_Now == _SENSOR_DISCONNECT)
    {
        i = _SS_TEMP;
        for(uint8_t j = _SS_TEMP + 1; j < _END_SENSOR; j++)
        {
            sSensorWarning[j].Gettick_Handle = HAL_GetTick();
            sSensorWarning[j].State_Connect_Befor = _SENSOR_DISCONNECT;
        }
    }
    else
    {
        sSensorWarning[_SS_TEMP].Gettick_Handle = 0;
    }
    
    fevent_enable(sEventAppSensor, event);
    return 0;
}
/*====================Function Handle====================*/
uint8_t _Cb_Handle_Detect_Power(uint8_t State)
{
    uint8_t aData[2] = {0};
    aData[0] = 0x00;
    aData[1] = 0x00;
    Log_EventWarnig(OBIS_WARNING_DETECT_POWER, 0x01, aData);
    return 1;
}

uint8_t _Cb_Handle_SS_pH(uint8_t State)
{
    uint8_t aData[2] = {0};
    aData[0] = 0x00;
    aData[1] = 0x00;
    Log_EventWarnig(OBIS_WARNING_SENSOR_CONNECT, 0x01, aData);
    return 1;
}

uint8_t _Cb_Handle_SS_Clo(uint8_t State)
{
    uint8_t aData[2] = {0};
    aData[0] = 0x02;
    aData[1] = 0x00;
    Log_EventWarnig(OBIS_WARNING_SENSOR_CONNECT, 0x01, aData);
    return 1;
}

uint8_t _Cb_Handle_SS_EC(uint8_t State)
{
    uint8_t aData[2] = {0};
    aData[0] = 0x04;
    aData[1] = 0x00;
    Log_EventWarnig(OBIS_WARNING_SENSOR_CONNECT, 0x01, aData);
    return 1;
}

uint8_t _Cb_Handle_SS_Turb(uint8_t State)
{
    uint8_t aData[2] = {0};
    aData[0] = 0x06;
    aData[1] = 0x00;
    Log_EventWarnig(OBIS_WARNING_SENSOR_CONNECT, 0x01, aData);
    return 1;
}

uint8_t _Cb_Handle_SS_COD(uint8_t State)
{
    uint8_t aData[2] = {0};
    aData[0] = 0x08;
    aData[1] = 0x00;
    Log_EventWarnig(OBIS_WARNING_SENSOR_CONNECT, 0x01, aData);
    return 1;
}

uint8_t _Cb_Handle_SS_TSS(uint8_t State)
{
    uint8_t aData[2] = {0};
    aData[0] = 0x0A;
    aData[1] = 0x00;
    Log_EventWarnig(OBIS_WARNING_SENSOR_CONNECT, 0x01, aData);
    return 1;
}

uint8_t _Cb_Handle_SS_NH4(uint8_t State)
{
    uint8_t aData[2] = {0};
    aData[0] = 0x0C;
    aData[1] = 0x00;
    Log_EventWarnig(OBIS_WARNING_SENSOR_CONNECT, 0x01, aData);
    return 1;
}

uint8_t _Cb_Handle_SS_DO(uint8_t State)
{
    uint8_t aData[2] = {0};
    aData[0] = 0x0E;
    aData[1] = 0x00;
    Log_EventWarnig(OBIS_WARNING_SENSOR_CONNECT, 0x01, aData);
    return 1;
}

uint8_t _Cb_Handle_SS_SALT(uint8_t State)
{
    uint8_t aData[2] = {0};
    aData[0] = 0x10;
    aData[1] = 0x00;
    Log_EventWarnig(OBIS_WARNING_SENSOR_CONNECT, 0x01, aData);
    return 1;
}

uint8_t _Cb_Handle_SS_TDS(uint8_t State)
{
    uint8_t aData[2] = {0};
    aData[0] = 0x12;
    aData[1] = 0x00;
    Log_EventWarnig(OBIS_WARNING_SENSOR_CONNECT, 0x01, aData);
    return 1;
}

uint8_t _Cb_Handle_SS_NO3(uint8_t State)
{
    uint8_t aData[2] = {0};
    aData[0] = 0x14;
    aData[1] = 0x00;
    Log_EventWarnig(OBIS_WARNING_SENSOR_CONNECT, 0x01, aData);
    return 1;
}
/*=======================Handle Sensor======================*/
void Log_EventWarnig(uint8_t Obis, uint8_t LengthData, uint8_t *aDataWaring)
{
  Get_RTC();
  
  if(sRTC.year > 20)
  {
    uint8_t     aData[10]={0};
    uint16_t    length = 0;
    uint16_t	i = 0;
    uint8_t     TempCrc = 0;
    
    SV_Protocol_Packet_Data(aData, &length, OBIS_TIME_DEVICE, &sRTC, 6, 0xAA);
    aData[length++] = Obis;
    aData[length++] = LengthData;
    
    for(uint8_t i = 0; i < LengthData; i++)
        aData[length++] = *(aDataWaring+i);
    
    // caculator crc
    length++;
	for (i = 0; i < (length - 1); i++)
		TempCrc ^= aData[i];

    aData[length-1] = TempCrc;
  
//#ifdef USING_APP_MEM
    Mem_Write_Data(sMemVar.Type_u8, _MEM_DATA_EVENT, 0, aData, length, sRecEvent.Size_u16);
//#endif
  }
}
/*=====================Packet Parameter TNMT====================*/
void AppSensor_Packet_TNMT(void)
{
    uint8_t tnstatus_u8 = 0x00;  
    char aPAY_LOAD[MAX_MEM_DATA] = {0};
//    int8_t chann = 0, i = 0, cPack = 0, mPack = false;
    char aTEMP[48] = {0};
    
//    char pathtest[40] = "SV";
//    char aName[40] = {0};
//    static uint32_t cFile = 0;
    uint8_t c_pack = 0;
    char File_Name[MAX_LENGTH_FILE_NAME] = {0};
    int8_t chann = 0;
    uint32_t length = 0;
    
    chann = AppWm_Is_Firt_Chann_TN();
    if (chann >= 0) 
    {
        for(uint8_t i = _SS_PH; i<_END_SENSOR; i++)
        {
            if(s485Measure[i].sUser == 1)
            {
                c_pack++;
                tnstatus_u8 = sAverageMeasure[i].State;
                AppSensor_Packet_Param(aTEMP, i);
                sprintf(aPAY_LOAD + strlen(aPAY_LOAD), "%s %s %04d%02d%02d%02d%02d%02d %02d\n", 
                                        AppSensor_TN_PARAM[i], aTEMP,
                                        sRTC.year + 2000, sRTC.month, sRTC.date,
                                        sRTC.hour, sRTC.min, sRTC.sec,
                                        tnstatus_u8);
            }
        }
        
        if(c_pack > 0)
        {
            AppSensor_Packet_Param(aTEMP, 0);
            sprintf(aPAY_LOAD + strlen(aPAY_LOAD), "%s %s %04d%02d%02d%02d%02d%02d %02d\n", 
                                    AppSensor_TN_PARAM[0], aTEMP,
                                    sRTC.year + 2000, sRTC.month, sRTC.date,
                                    sRTC.hour, sRTC.min, sRTC.sec,
                                    tnstatus_u8);
            Mem_Write_Data(sMemVar.Type_u8, _MEM_DATA_GPS, 0, 
            (uint8_t *) aPAY_LOAD, strlen(aPAY_LOAD), sRecGPS.Size_u16);
            
            chann = AppWm_Is_Firt_Chann_TN();
            if (chann >= 0) {
                length = strlen(sWmVar.sChannInfor[chann].MA_TINH) 
                        + strlen(sWmVar.sChannInfor[chann].MA_CTRINH) + strlen(sWmVar.sChannInfor[chann].MA_TRAM); 
                
                if (length < MAX_LENGTH_FILE_NAME ) {
                    sprintf(File_Name, "%s_%s_%s_", sWmVar.sChannInfor[chann].MA_TINH, 
                                        sWmVar.sChannInfor[chann].MA_CTRINH, sWmVar.sChannInfor[chann].MA_TRAM);
                }
                sprintf(File_Name + strlen(File_Name), "%04d%02d%02d%02d%02d%02d.txt", sRTC.year + 2000, sRTC.month, sRTC.date,
                                                    sRTC.hour, sRTC.min, sRTC.sec  );
                sd_unmount();
                sd_mount();
                sd_write_file(File_Name, aPAY_LOAD);
            }
        }
    }
}

uint8_t AppSensor_Packet_Param (char *pdata, uint8_t chann)
{
    sprintf(pdata, "%.3lf %s", sAverageMeasure[chann].Value_f , AppSensor_TN_UNIT[chann]);

    return 1;
}

/*==================Handle Define AT command=================*/
#ifdef USING_AT_CONFIG
void AT_CMD_Get_Time_Warning_Sensor(sData *str, uint16_t Pos)
{
    uint8_t aTemp[50] = "TimeWarningSensor: ";   //13 ki tu dau tien
    sData StrResp = {&aTemp[0], 19}; 

    Convert_Uint64_To_StringDec (&StrResp, (uint64_t) (DurationTimeWarningSensor), 0);
    Insert_String_To_String(StrResp.Data_a8, &StrResp.Length_u16, (uint8_t*)" min",0 , 4);

	Modem_Respond(PortConfig, StrResp.Data_a8, StrResp.Length_u16, 0);
}

void AT_CMD_Set_Time_Warning_Sensor (sData *str_Receiv, uint16_t Pos)
{
    uint32_t TempU32 = 0;
    if( str_Receiv->Data_a8[0] >= '0' && str_Receiv->Data_a8[0] <= '9')
    {
        uint8_t length = 0;
        for(uint8_t i = 0; i < str_Receiv->Length_u16; i++)
        {
            if( str_Receiv->Data_a8[i] < '0' || str_Receiv->Data_a8[i]>'9') break;
            else length++;
        }
        TempU32 = Convert_String_To_Dec(str_Receiv->Data_a8 , length);
        if(TempU32 <=255 && TempU32 >=1)
        {
            Save_TimeWarningSensor(TempU32);
            Modem_Respond(PortConfig, (uint8_t*)"OK", 2, 0);
        }
        else
        {
            Modem_Respond(PortConfig, (uint8_t*)"ERROR", 5, 0);
        }
    }
    else
    {
        Modem_Respond(PortConfig, (uint8_t*)"ERROR", 5, 0);
    }
}

void AT_CMD_Get_State_Sensor(sData *str, uint16_t Pos)
{
    uint8_t aTemp[200] = "State: ";   //11 ki tu dau tien
    uint16_t length = 7;
    
    for (uint8_t i = 1; i < _END_SENSOR; i++)   
    {
        Insert_String_To_String(aTemp, &length,(uint8_t*)" ", 0, 1);
        Insert_String_To_String(aTemp, &length,(uint8_t*)AppSensor_TN_PARAM[i], 0,strlen(AppSensor_TN_PARAM[i]));
        Insert_String_To_String(aTemp, &length,(uint8_t*)":", 0, 1);

        Convert_Point_Int_To_String_Scale(aTemp,&length,(int)(s485Measure[i].sConnect_u8), 0x00);     
    }

    Modem_Respond(PortConfig, aTemp, length, 0);
}

void AT_CMD_Get_Measure_Value (sData *str_Receiv, uint16_t Pos)
{
    uint8_t aTemp[300] = "Measure_Value: ";   //11 ki tu dau tien
    uint16_t length = 15;

    for (uint8_t i = 1; i < _END_SENSOR; i++)   
    {
        Insert_String_To_String(aTemp, &length,(uint8_t*)" ", 0, 1);
        Insert_String_To_String(aTemp, &length,(uint8_t*)AppSensor_TN_PARAM[i], 0,strlen(AppSensor_TN_PARAM[i]));
        Insert_String_To_String(aTemp, &length,(uint8_t*)":", 0, 1);

        Convert_Point_Int_To_String_Scale(aTemp,&length,(int)(s485Measure[i].Value_f*100), 0xFE);  
    }

    Modem_Respond(PortConfig, aTemp, length, 0);
}

void AT_CMD_Get_Measure_Filter (sData *str_Receiv, uint16_t Pos)
{
    uint8_t aTemp[300] = "Measure_Filter: ";   //11 ki tu dau tien
    uint16_t length = 16;
    
    for (uint8_t i = 1; i < _END_SENSOR; i++)   
    {
        Insert_String_To_String(aTemp, &length,(uint8_t*)" ", 0, 1);
        Insert_String_To_String(aTemp, &length,(uint8_t*)AppSensor_TN_PARAM[i], 0,strlen(AppSensor_TN_PARAM[i]));
        Insert_String_To_String(aTemp, &length,(uint8_t*)":", 0, 1);

        Convert_Point_Int_To_String_Scale(aTemp,&length,(int)(sMeasureHandle[i].sMeasureHanlde->Value_i32), sMeasureHandle[i].sMeasureHanlde->Scale_u8);
    }

    Modem_Respond(PortConfig, aTemp, length, 0);
}

void AT_CMD_Get_Measure_Average (sData *str_Receiv, uint16_t Pos)
{
    uint8_t aTemp[300] = "Measure_Average: ";   //11 ki tu dau tien
    uint16_t length = 17;
    
    for (uint8_t i = 1; i < _END_SENSOR; i++)   
    {
        Insert_String_To_String(aTemp, &length,(uint8_t*)" ", 0, 1);
        Insert_String_To_String(aTemp, &length,(uint8_t*)AppSensor_TN_PARAM[i], 0,strlen(AppSensor_TN_PARAM[i]));
        Insert_String_To_String(aTemp, &length,(uint8_t*)":", 0, 1);

        Convert_Point_Int_To_String_Scale(aTemp,&length,(int)(sAverageMeasure[i].Value_f*100), 0xFE); 
    }

    Modem_Respond(PortConfig, aTemp, length, 0);
}

void AT_CMD_Set_Reset_Average (sData *str_Receiv, uint16_t Pos)
{
    uint8_t aTemp[300] = "Reset_Average: OK";   //11 ki tu dau tien
    uint16_t length = 17;
    
    for(uint8_t i = 0; i < _END_SENSOR; i++)
    {
        UTIL_MEM_set(&sAverageOneHour[i], 0, sizeof(Struct_AverageOneHour));
    }
    OnchipFlashPageErase(ADDR_VALUE_AVG_ONE_HOUR);

    Modem_Respond(PortConfig, aTemp, length, 0);
}

void AT_CMD_Get_User_Sensor (sData *str_Receiv, uint16_t Pos)
{
    uint8_t aTemp[200] = "User: ";   //11 ki tu dau tien
    uint16_t length = 6;
    
    for (uint8_t i = 1; i < _END_SENSOR; i++)   
    {
        Insert_String_To_String(aTemp, &length,(uint8_t*)" ", 0, 1);
        Insert_String_To_String(aTemp, &length,(uint8_t*)AppSensor_TN_PARAM[i], 0,strlen(AppSensor_TN_PARAM[i]));
        Insert_String_To_String(aTemp, &length,(uint8_t*)":", 0, 1);

        Convert_Point_Int_To_String_Scale(aTemp,&length,(int)(s485Measure[i].sUser), 0x00);
    }

    Modem_Respond(PortConfig, aTemp, length, 0);
}

            
void AT_CMD_Set_User_Sensor (sData *str_Receiv, uint16_t Pos)
{
    uint32_t TempU32_1 = 255; 
    uint32_t TempU32_2 = 255;
    if( str_Receiv->Data_a8[0] >= '0' && str_Receiv->Data_a8[0] <= '9')
    {
        uint8_t length_1 = 0;
        uint8_t length_2 = 0;
        for(uint8_t i = 0; i < str_Receiv->Length_u16; i++)
        {
            if( str_Receiv->Data_a8[i] < '0' || str_Receiv->Data_a8[i]>'9') break;
            else length_1++;
        }
        if(length_1 > 0)
            TempU32_1 = Convert_String_To_Dec(str_Receiv->Data_a8 , length_1);

        for(uint8_t i = length_1+1; i < str_Receiv->Length_u16; i++)
        {
            if( str_Receiv->Data_a8[i] < '0' || str_Receiv->Data_a8[i]>'9') break;
            else length_2++;
        }
        if(length_2 > 0)
            TempU32_2 = Convert_String_To_Dec(&str_Receiv->Data_a8[length_1+1] , length_2);
        
        if(TempU32_1 <= _END_SENSOR && TempU32_1 > 0 && TempU32_2 <= 1)
        {
            Save_UserSensor(TempU32_1, TempU32_2);
            Modem_Respond(PortConfig, (uint8_t*)"OK", 2, 0);
        }
        else
        {
            Modem_Respond(PortConfig, (uint8_t*)"ERROR", 5, 0);
        }
    }
    else
    {
        Modem_Respond(PortConfig, (uint8_t*)"ERROR", 5, 0);
    }
}

void AT_CMD_Get_Offset_Sensor (sData *str_Receiv, uint16_t Pos)
{
    uint8_t aTemp[200] = "Offset: ";
    uint16_t length = 8;

    for (uint8_t i = 1; i < _END_SENSOR; i++)   
    {
        Insert_String_To_String(aTemp, &length,(uint8_t*)" ", 0, 1);
        Insert_String_To_String(aTemp, &length,(uint8_t*)AppSensor_TN_PARAM[i], 0,strlen(AppSensor_TN_PARAM[i]));
        Insert_String_To_String(aTemp, &length,(uint8_t*)":", 0, 1);

        Convert_Point_Int_To_String_Scale(aTemp,&length,(int)(s485Measure[i].Offset_f * 100), 0xFE);
    }

    Modem_Respond(PortConfig, aTemp, length, 0);
}

            
void AT_CMD_Set_Offset_Sensor (sData *str_Receiv, uint16_t Pos)
{
    uint32_t Temp_U32 = 0;
    int32_t  Temp_I32 = 0;
    float    Temp_f = 0;
    uint8_t checkTemp = 0;
    if( str_Receiv->Data_a8[0] >= '0' && str_Receiv->Data_a8[0] <= '9')
    {
        uint8_t length_1 = 0;
        uint8_t length_2 = 0;
        for(uint8_t i = 0; i < str_Receiv->Length_u16; i++)
        {
            if( str_Receiv->Data_a8[i] < '0' || str_Receiv->Data_a8[i]>'9') break;
            else length_1++;
        }
        if(length_1 > 0)
            Temp_U32 = Convert_String_To_Dec(str_Receiv->Data_a8 , length_1);
        
        if(str_Receiv->Data_a8[length_1 + 1] == '-')
            checkTemp = 1;

        for(uint8_t i = length_1+checkTemp+1; i < str_Receiv->Length_u16; i++)
        {
            if( str_Receiv->Data_a8[i] < '0' || str_Receiv->Data_a8[i]>'9') break;
            else length_2++;
        }
        if(length_2 > 0)
            Temp_I32 = Convert_String_To_Dec(&str_Receiv->Data_a8[length_1+checkTemp+1] , length_2);
        
        if(checkTemp == 1)
          Temp_I32 = 0 - Temp_I32;

        if(Temp_U32 <= _END_SENSOR && Temp_U32 > 0 && length_2 > 0)
        {
            Temp_f = Handle_int32_To_Float_Scale(Temp_I32, 0xFE);
            Save_OffsetMeasure(Temp_U32, Temp_f);
            Modem_Respond(PortConfig, (uint8_t*)"OK", 2, 0);
        }
        else
        {
            Modem_Respond(PortConfig, (uint8_t*)"ERROR", 5, 0);
        }
    }
    else
    {
        Modem_Respond(PortConfig, (uint8_t*)"ERROR", 5, 0);
    }
}

void AT_CMD_Get_State_Calib_SS(sData *str, uint16_t Pos)
{
    uint8_t aTemp[50] = "StateCalibSensor: ";   //13 ki tu dau tien
    sData StrResp = {&aTemp[0], 18}; 

    Convert_Uint64_To_StringDec (&StrResp, (uint64_t) (StateCalibSensor), 0);
    Insert_String_To_String(StrResp.Data_a8, &StrResp.Length_u16, (uint8_t*)" ",0 , 1);

	Modem_Respond(PortConfig, StrResp.Data_a8, StrResp.Length_u16, 0);
}

void AT_CMD_Set_State_Calib_SS(sData *str_Receiv, uint16_t Pos)
{
    uint32_t TempU32 = 0;
    if( str_Receiv->Data_a8[0] >= '0' && str_Receiv->Data_a8[0] <= '9')
    {
        uint8_t length = 0;
        for(uint8_t i = 0; i < str_Receiv->Length_u16; i++)
        {
            if( str_Receiv->Data_a8[i] < '0' || str_Receiv->Data_a8[i]>'9') break;
            else length++;
        }
        TempU32 = Convert_String_To_Dec(str_Receiv->Data_a8 , length);
        if(TempU32 <=1)
        {
            Save_StateCalibSensor(TempU32);
            Modem_Respond(PortConfig, (uint8_t*)"OK", 2, 0);
        }
        else
        {
            Modem_Respond(PortConfig, (uint8_t*)"ERROR", 5, 0);
        }
    }
    else
    {
        Modem_Respond(PortConfig, (uint8_t*)"ERROR", 5, 0);
    }
}

#endif

/*---------------------Save and Init User Sensor----------------------*/
void Save_UserSensor(uint8_t KindSensor, uint8_t State)
{
    uint8_t aData[30] = {0};
    uint8_t length = 0;
           
    if(KindSensor < _END_SENSOR && KindSensor > 0)
        s485Measure[KindSensor].sUser = State;
    
    for(uint8_t i = _SS_PH; i < _END_SENSOR; i++)
    {
        aData[length++] = s485Measure[i].sUser;
    }

    Save_Array(ADDR_USER_SENSOR, aData, length);
}

void Init_UserSensor(void)
{
#ifdef USING_INTERNAL_MEM
    if(*(__IO uint8_t*)(ADDR_USER_SENSOR) != FLASH_BYTE_EMPTY)
    {
        for(uint8_t i = _SS_PH; i < _END_SENSOR; i++)
        {
            s485Measure[i].sUser = *(__IO uint8_t*)(ADDR_USER_SENSOR+ 2 + i - 1);
            if(s485Measure[i].sUser > 1)
                s485Measure[i].sUser = 0;
        }
    }
    else
    {
        for(uint8_t i = _SS_PH; i < _END_SENSOR; i++)
        {
            s485Measure[i].sUser = 0;
        }
    }
#endif    
}
/*===================Save and Init Offset Measure=================*/
void Save_OffsetMeasure(uint8_t KindOffset, float Var_Offset_f)
{
#ifdef USING_INTERNAL_MEM
    uint8_t aData[300] = {0};
    uint8_t length = 0;
    
    uint32_t Stamp_Hex = 0;
   
    if(KindOffset < _END_SENSOR && KindOffset > 0)
        s485Measure[KindOffset].Offset_f = Var_Offset_f;
    
    for(uint8_t i = _SS_PH; i < _END_SENSOR; i++)
    {
        Stamp_Hex = Handle_Float_To_hexUint32(s485Measure[i].Offset_f);
        aData[length++] = Stamp_Hex >> 24;
        aData[length++] = Stamp_Hex >> 16;
        aData[length++] = Stamp_Hex >> 8;
        aData[length++] = Stamp_Hex ;
    }
    
    Save_Array(ADDR_OFFSET_MEASURE, aData, length);
#endif
}

void Init_OffsetMeasure(void)
{
#ifdef USING_INTERNAL_MEM
    uint32_t Stamp_Hex = 0;
    
    if(*(__IO uint8_t*)(ADDR_OFFSET_MEASURE) != FLASH_BYTE_EMPTY)
    {
        for(uint8_t i = _SS_PH; i < _END_SENSOR; i++)
        {
            Stamp_Hex   = *(__IO uint8_t*)(ADDR_OFFSET_MEASURE+2 + (i-1)*4) << 24;
            Stamp_Hex  |= *(__IO uint8_t*)(ADDR_OFFSET_MEASURE+3 + (i-1)*4)<< 16;
            Stamp_Hex  |= *(__IO uint8_t*)(ADDR_OFFSET_MEASURE+4 + (i-1)*4)<< 8;
            Stamp_Hex  |= *(__IO uint8_t*)(ADDR_OFFSET_MEASURE+5 + (i-1)*4);
            if(Stamp_Hex != 0xFFFFFFFF)
                Convert_uint32Hex_To_Float(Stamp_Hex,  &s485Measure[i].Offset_f);
            else
                s485Measure[i].Offset_f = 0;
        }
    }
    else
    {
        for(uint8_t i = _SS_PH; i < _END_SENSOR; i++)
            s485Measure[i].Offset_f = 0;
    }
#endif    
}

/*====================Save and Init Time Warning==================*/
void Save_TimeWarningSensor(uint8_t Duration)
{
#ifdef USING_INTERNAL_MEM
    uint8_t aData[8] = {0};
    uint8_t length = 0;
    
    DurationTimeWarningSensor = Duration;
    
    aData[length++] = DurationTimeWarningSensor;

    Save_Array(ADDR_TIME_WARNING_SENSOR, aData, length);
#endif
}

void Init_TimeWarningSensor(void)
{
#ifdef USING_INTERNAL_MEM
    if(*(__IO uint8_t*)(ADDR_TIME_WARNING_SENSOR) != FLASH_BYTE_EMPTY)
    {
        DurationTimeWarningSensor = *(__IO uint8_t*)(ADDR_TIME_WARNING_SENSOR+2);
    }
    else
    {
        DurationTimeWarningSensor = TIME_RESEND_WARNING;
    }
#endif    
}
/*=====================Save and Init State Calib====================*/
void Save_StateCalibSensor(uint8_t State)
{
#ifdef USING_INTERNAL_MEM
    uint8_t aData[8] = {0};
    uint8_t length = 0;
    
    StateCalibSensor = State;
    
    aData[length++] = State;

    Save_Array(ADDR_STATE_CALIB_SENSOR, aData, length);
#endif
}

void Init_StateCalibSensor(void)
{
#ifdef USING_INTERNAL_MEM
    if(*(__IO uint8_t*)(ADDR_STATE_CALIB_SENSOR) != FLASH_BYTE_EMPTY)
    {
        StateCalibSensor = *(__IO uint8_t*)(ADDR_STATE_CALIB_SENSOR+2);
    }
    else
    {
        StateCalibSensor = _E_DANG_DO;
    }
#endif    
}
/*================Save and Init Value Average One Hour==============*/
void Save_AverageOneHour(void)
{
#ifdef USING_INTERNAL_MEM
    uint8_t aData[200] = {0};
    uint16_t length = 0;
    uint32_t Stamp_Hex = 0;
    
    for(uint8_t i = 0; i < _END_SENSOR; i++)
    {
        Stamp_Hex = Handle_Float_To_hexUint32(sAverageOneHour[i].Sum_Value_f);
        
        aData[length++] = sAverageOneHour[i].Year_Before;
        aData[length++] = sAverageOneHour[i].Month_Before;
        aData[length++] = sAverageOneHour[i].Date_Before;
        aData[length++] = sAverageOneHour[i].Hour_Before;
        aData[length++] = sAverageOneHour[i].Num_Success;
        aData[length++] = sAverageOneHour[i].Num_Error;
        
        aData[length++] = Stamp_Hex >> 24;
        aData[length++] = Stamp_Hex >> 16;
        aData[length++] = Stamp_Hex >> 8;
        aData[length++] = Stamp_Hex ;
    }

    Save_Array(ADDR_VALUE_AVG_ONE_HOUR, aData, length);
#endif
}

void Init_AverageOneHour(void)
{
#ifdef USING_INTERNAL_MEM
    uint32_t Stamp_Hex = 0;
    
    if(*(__IO uint8_t*)(ADDR_VALUE_AVG_ONE_HOUR) != FLASH_BYTE_EMPTY)
    {
        for(uint8_t i = 0; i < _END_SENSOR; i++)
        {
            sAverageOneHour[i].Year_Before = *(__IO uint8_t*)(ADDR_VALUE_AVG_ONE_HOUR+2+i*10);
            sAverageOneHour[i].Month_Before = *(__IO uint8_t*)(ADDR_VALUE_AVG_ONE_HOUR+3+i*10);
            sAverageOneHour[i].Date_Before = *(__IO uint8_t*)(ADDR_VALUE_AVG_ONE_HOUR+4+i*10);
            sAverageOneHour[i].Hour_Before = *(__IO uint8_t*)(ADDR_VALUE_AVG_ONE_HOUR+5+i*10);
            sAverageOneHour[i].Num_Success = *(__IO uint8_t*)(ADDR_VALUE_AVG_ONE_HOUR+6+i*10);
            sAverageOneHour[i].Num_Error = *(__IO uint8_t*)(ADDR_VALUE_AVG_ONE_HOUR+7+i*10);
              
            Stamp_Hex   = *(__IO uint8_t*)(ADDR_VALUE_AVG_ONE_HOUR+8 + i*10) << 24;
            Stamp_Hex  |= *(__IO uint8_t*)(ADDR_VALUE_AVG_ONE_HOUR+9 + i*10) << 16;
            Stamp_Hex  |= *(__IO uint8_t*)(ADDR_VALUE_AVG_ONE_HOUR+10 + i*10) << 8;
            Stamp_Hex  |= *(__IO uint8_t*)(ADDR_VALUE_AVG_ONE_HOUR+11 + i*10);
            
            if(Stamp_Hex != 0xFFFFFFFF)
                Convert_uint32Hex_To_Float(Stamp_Hex,  &sAverageOneHour[i].Sum_Value_f);
            else
                sAverageOneHour[i].Sum_Value_f = 0;
        }
    }
    else
    {
        for(uint8_t i = 0; i < _END_SENSOR; i++)
        {
            UTIL_MEM_set(&sAverageOneHour[i], 0, sizeof(Struct_AverageOneHour));
        }
    }
#endif    
}

uint8_t IsLeapYear(uint16_t year_full)
{
    if ((year_full % 4 == 0 && year_full % 100 != 0) || (year_full % 400 == 0))
        return 1;
    return 0;
}

uint32_t ConvertToHours(uint8_t year, uint8_t month, uint8_t day, uint8_t hour)
{
    static const uint8_t daysInMonth[12] =
        {31,28,31,30,31,30,31,31,30,31,30,31};

    uint32_t days = 0;
    uint16_t year_full = 2000 + year;

    // Cong so ngay cua cac nam truoc
    for(uint16_t y = 2000; y < year_full; y++)
        days += IsLeapYear(y) ? 366 : 365;

    // Cong so ngay cua cac thang truoc
    for(uint8_t m = 1; m < month; m++)
    {
        if(m == 2 && IsLeapYear(year_full))
            days += 29;
        else
            days += daysInMonth[m-1];
    }

    days += (day - 1);

    return days * 24UL + hour;
}

void Average_One_Hour(void)
{
    float temp_f = 0;
    uint8_t Scale = 0;
    uint32_t Count_Time_Current = 0;
    uint32_t Count_Time_Before = 0;
    uint8_t Count_Handle = 0;           //Tinh tong so lan xu ly
    
    Count_Handle++;
    Count_Time_Current = ConvertToHours(sRTC.year, sRTC.month, sRTC.date, sRTC.hour);
    Count_Time_Before = ConvertToHours(sAverageOneHour[0].Year_Before, sAverageOneHour[0].Month_Before, 
                                       sAverageOneHour[0].Date_Before, sAverageOneHour[0].Hour_Before);
    
    if(Count_Time_Current - Count_Time_Before > 1)
    {
        for(uint8_t i = 0; i < _END_SENSOR; i++)
        {
            UTIL_MEM_set(&sAverageOneHour[i], 0, sizeof(Struct_AverageOneHour));
        }
        OnchipFlashPageErase(ADDR_VALUE_AVG_ONE_HOUR);
    }

    if(StateCalibSensor == _E_DANG_DO)
    {
        if(sDataSensorMeasure.sTemp.State_u8 == 1)
        {
            temp_f = (float)sDataSensorMeasure.sTemp.Value_i32;
            Scale = sDataSensorMeasure.sTemp.Scale_u8;
            sAverageOneHour[0].Sum_Value_f += temp_f/(float)Calculator_Scale(Scale);
            sAverageOneHour[0].Num_Success++;
        }
        else
        {
            sAverageOneHour[0].Num_Error++;
        }
        sAverageOneHour[0].Year_Before = sRTC.year;
        sAverageOneHour[0].Month_Before = sRTC.month;
        sAverageOneHour[0].Date_Before = sRTC.date;
        sAverageOneHour[0].Hour_Before = sRTC.hour;
        
        for(uint8_t i = 1; i< _END_SENSOR; i++)
        {
            if(sMeasureHandle[i].sMeasureHanlde->State_u8 == 1)
            {
                temp_f = (float)sMeasureHandle[i].sMeasureHanlde->Value_i32;
                Scale = sMeasureHandle[i].sMeasureHanlde->Scale_u8;
                sAverageOneHour[i].Sum_Value_f += temp_f/(float)Calculator_Scale(Scale);
                sAverageOneHour[i].Num_Success++;
            }
            else
            {
                sAverageOneHour[i].Num_Error++;
            }
            sAverageOneHour[i].Year_Before = sRTC.year;
            sAverageOneHour[i].Month_Before = sRTC.month;
            sAverageOneHour[i].Date_Before = sRTC.date;
            sAverageOneHour[i].Hour_Before = sRTC.hour;
        }
        Calculator_Average_One_Hour();
    }
    else
    {
        for(uint8_t i = 0; i < _END_SENSOR; i++)
            sAverageMeasure[i].State = StateCalibSensor;
    }
    
    if(sRTC.min == 0 || Count_Handle > 120)
    {
        for(uint8_t i = 0; i < _END_SENSOR; i++)
        {
            UTIL_MEM_set(&sAverageOneHour[i], 0, sizeof(Struct_AverageOneHour));
        }
        OnchipFlashPageErase(ADDR_VALUE_AVG_ONE_HOUR);
        Count_Handle = 0;
    }
       
    Save_AverageOneHour();
}

void Calculator_Average_One_Hour(void)
{
    for(uint8_t i = 0; i < _END_SENSOR; i++)
    {
        if(sAverageOneHour[i].Num_Success > 0 && sAverageOneHour[i].Num_Success >= sAverageOneHour[i].Num_Error)
        {
            sAverageMeasure[i].State = _E_DANG_DO;
            sAverageMeasure[i].Value_f = sAverageOneHour[i].Sum_Value_f/sAverageOneHour[i].Num_Success;
        }
        else
        {
            sAverageMeasure[i].State = _E_BAO_LOI_THIET_BI;
            sAverageMeasure[i].Value_f = 0;
        }
    }
}

/*=====================Handle Task and Init app===================*/
void Init_AppSensor(void)
{
    Init_StateCalibSensor();
    Init_AverageOneHour();
    Init_TimeWarningSensor();
    Init_OffsetMeasure();
    Init_UserSensor();
#ifdef USING_AT_CONFIG
    /* regis cb serial */
    sATCmdList[_GET_FREQ_WARNING_SENSOR].CallBack = AT_CMD_Get_Time_Warning_Sensor;
    sATCmdList[_SET_FREQ_WARNING_SENSOR].CallBack = AT_CMD_Set_Time_Warning_Sensor;
    
    sATCmdList[_GET_STATE_SENSOR].CallBack = AT_CMD_Get_State_Sensor;
    
    sATCmdList[_GET_MEASURE_VALUE].CallBack = AT_CMD_Get_Measure_Value;
    sATCmdList[_GET_MEASURE_FILTER].CallBack = AT_CMD_Get_Measure_Filter;
    sATCmdList[_GET_MEASURE_AVERAGE].CallBack = AT_CMD_Get_Measure_Average;
    sATCmdList[_SET_RESET_AVERAGE].CallBack = AT_CMD_Set_Reset_Average,
    
    sATCmdList[_GET_USER_SENSOR].CallBack = AT_CMD_Get_User_Sensor;
    sATCmdList[_SET_USER_SENSOR].CallBack = AT_CMD_Set_User_Sensor;
    
    sATCmdList[_GET_OFFSET_SENSOR].CallBack = AT_CMD_Get_Offset_Sensor;
    sATCmdList[_SET_OFFSET_SENSOR].CallBack = AT_CMD_Set_Offset_Sensor;
    
    sATCmdList[_GET_STATE_CALIB_SS].CallBack = AT_CMD_Get_State_Calib_SS;
    sATCmdList[_SET_STATE_CALIB_SS].CallBack = AT_CMD_Set_State_Calib_SS;
    
#endif
}

uint8_t AppSensor_Task(void)
{
    uint8_t i = 0;
    uint8_t Result = false;
    for( i = 0; i < _EVENT_SENSOR_END; i++)
    {
        if(sEventAppSensor[i].e_status == 1)
        {
            Result = true;
            if((sEventAppSensor[i].e_systick == 0) ||
                ((HAL_GetTick() - sEventAppSensor[i].e_systick) >= sEventAppSensor[i].e_period))
            {
                sEventAppSensor[i].e_status = 0; //Disable event
                sEventAppSensor[i].e_systick= HAL_GetTick();
                sEventAppSensor[i].e_function_handler(i);
            }
        }
    }
    
    return Result;
}
