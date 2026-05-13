
#ifndef USER_APP_RS485_H__
#define USER_APP_RS485_H__

#define USING_APP_RS485

#include "user_util.h"
#include "event_driven.h"

#define ID_DEFAULT_OXY          5
#define ID_DEFAULT_PH           3

#define ID_DEFAULT_SS_PH        1
#define ID_DEFAULT_SS_CLO       2
#define ID_DEFAULT_SS_EC        3
#define ID_DEFAULT_SS_TURB      4

#define ID_SS_PH        1
#define ID_SS_CLO       2
#define ID_SS_EC        3
#define ID_SS_TURB      4
#define ID_SS_COD       5
#define ID_SS_TSS       6
#define ID_SS_NH4       7
#define ID_SS_DO        8
#define ID_SS_NO3       9

#define DEFAULT_SCALE_CLO           0xFE
#define DEFAULT_SCALE_PH            0xFE
#define DEFAULT_SCALE_NTU           0xFE
#define DEFAULT_SCALE_SALINITY      0xFE
#define DEFAULT_SCALE_TEMPERATURE   0xFE
#define DEFAULT_SCALE_EC            0x00

typedef enum
{
    _EVENT_RS485_ENTRY,

    _EVENT_RS485_WAIT_CALIB,
    _EVENT_RS485_REFRESH,
    
    _EVENT_RS485_HANDLE_SUBREG,
    
    _EVENT_RS485_TRANSMIT_DATA,
    _EVENT_RS485_RECEIVE_DATA,
    
    _EVENT_RS485_END,
}eKindEventRs485;

typedef enum
{
    _SENSOR_DISCONNECT = 0,
    _SENSOR_CONNECT,
}eKindStateSensor;

typedef enum
{
    _RS485_UNRESPOND = 0,
    _RS485_RESPOND,
}eKindStateRs485Respond;

typedef enum
{
    _STATE_CALIB_FREE = 0,
    _STATE_CALIB_ENTER,
    _STATE_CALIB_WAIT,
    _STATE_CALIB_DONE,
    _STATE_CALIB_ERROR,
}eKindStateSendCalib;

typedef enum
{
    _E_PH_VALUE,
    _E_PH_TEMP,
    
    _E_CLO_SEND_PH,
    _E_CLO_VALUE,
    _E_CLO_TEMP,
    
    _E_EC_VALUE,
    _E_EC_TEMP,
    
    _E_TURB_VALUE,
    _E_TURB_TEMP,
    
    _E_COD_VALUE,
    _E_COD_TEMP,
    
    _E_TSS_VALUE,
    _E_TSS_TEMP,
    
    _E_NH4_VALUE,
    _E_NH4_TEMP,
    
    _E_DO_SALT,
    _E_DO_VALUE,
    _E_DO_TEMP,
    
    _E_SALT_VALUE,
    _E_SALT_TEMP,
    
    _E_TDS_VALUE,
    _E_TDS_TEMP,
    
    _E_NO3_VALUE,
    _E_NO3_TEMP,

    _E_RS485_SS_END,
}eKindStateRs485Reg;

typedef enum
{
    _E_BE,              //Big Endian
    _E_LE,              //Little Endian
    _E_BS,              //Big Endian Swap (Byte Swap)
    _E_WS,              //Little Endian Word Swap (Word Swap)
}eKindEndianFormat;

typedef enum
{
    _ETYPE_F,           //Kieu du lieu float
    _ETYPE_U32,         //Kieu du lieu uint32_t
    _ETYPE_I32,         //Kieu du lieu int32_t
    _ETYPE_U16,         //Kieu du lieu uint16_t
    _ETYPE_I16,         //Kieu du lieu int16_t
}eKindTypeValue;


typedef struct
{   
    uint8_t State_u8;
    uint8_t Scale_u8;
    int32_t Value_i32;
}Struct_SS_Value;

typedef struct
{
    Struct_SS_Value sClo;
    Struct_SS_Value spH;
    Struct_SS_Value sTurb;
    Struct_SS_Value sEC;
    Struct_SS_Value sSal;
    Struct_SS_Value sCOD;
    Struct_SS_Value sTSS;
    Struct_SS_Value sNH4;
    Struct_SS_Value sDO;
    Struct_SS_Value sTemp;
    Struct_SS_Value sTDS;
    Struct_SS_Value sNO3;
}Struct_Data_Sensor_Measure;

typedef struct 
{
    uint8_t CountDisconnectRS485_1;
    uint8_t CountDisconnectRS485_2;
  
    uint8_t State_Wait_Calib;
}Struct_Hanlde_RS485;

typedef struct
{
    uint8_t  eKind;                  //Name Register
    uint8_t  *State;                 //Trang thai su dung (On/Off)
    uint8_t  cmdRW;                  //0: Read, 1: Write
    uint8_t  idDev;                  //ID cua thiet bi
    uint8_t  cmdLen;                 //So thanh ghi can doc
    uint16_t cmdAddr;                //Addr thanh ghi 1234(Dec)
    uint8_t  vFormat;                //Kieu du lieu(float, uint32_t, int32_t, uint16_t, int16_t)
    uint8_t  vBeLe;                  //Kieu giai ma
    float    vScale;                 //Scale du lieu 0.01, 0.1, 1, 10 (ket qua do nhan voi scale)
    float    *subReg;                //Gia tri viet vao thanh ghi lay tu thiet bi so 1|2|3...
    uint8_t  nPort;                  //Modbus port: 1
    float    *vReturn;                  
    uint8_t  *nConnect;
}Struct_RegSensor;

typedef struct
{
    uint8_t eKind;
    uint8_t sUser;                  //Trang thai su dung hay khong
    uint8_t nConnect_u8;            //Dem so lan connect
    uint8_t sConnect_u8;            //Trang thai connect
    float   Value_f;                //Gia tri do cam bien
    float   Temp_f;                 //Nhiet do
    float   Offset_f;                 //Offset gia tri do
}Struct_RS485_Measure;

typedef struct
{
    float pH;
    float Salt_PSU;
}Struct_Rs485_SubReg;

extern sEvent_struct        sEventAppRs485[];

extern Struct_Data_Sensor_Measure  sDataSensorMeasure;
extern Struct_Hanlde_RS485         sHandleRs485;
/*====================Function Handle====================*/

uint8_t    AppRs485_Task(void);
void       Init_AppRs485(void);

void       RS485_Para_Init(void);

void       Init_Parameter_Sensor(void);

void       Init_UartRs485(void);
void       Init_UartRs485_2(void);
void       Send_RS458_Sensor(uint8_t *aData, uint16_t Length_u16);

uint32_t   Read_Register_Rs485(uint8_t aData[], uint16_t *pos, uint8_t LengthData);

uint32_t   Endian_Format(uint32_t Hex_Data, uint8_t length, uint8_t Type);
float      Decode_Data_Type_u32(uint32_t Hex_Data, uint8_t Type);
uint32_t   Decode_Data_Type_f(float Data_f, uint8_t Type);
#endif

