
#ifndef USER_APP_MODB_H__
#define USER_APP_MODB_H__

#define USING_APP_MODB

#include "user_util.h"
#include "event_driven.h"

#define MODBUS_SENSOR_SAOVIET

#define ID_DEFAULT_OXY          5
#define ID_DEFAULT_PH           3

#define ID_DEFAULT_SS_PH        1
#define ID_DEFAULT_SS_CLO       2
#define ID_DEFAULT_SS_EC        3
#define ID_DEFAULT_SS_TURB      4

#define TIMEOUT_MODB_RTU        200
#define TIMEOUT_MODB_TCP        900

#define ID_SS_TEMP      0
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

#ifdef MODBUS_SENSOR_SAOVIET
    #define MAX_COUNT_DISCONNECT    9
#else
    #define MAX_COUNT_DISCONNECT    8
#endif
    
typedef enum
{
    _EVENT_MODB_ENTRY,

    _EVENT_MODB_WAIT_CALIB,
    _EVENT_MODB_RS485_1_REFRESH,
    _EVENT_MODB_RS485_2_REFRESH,
    
    _EVENT_MODB_HANDLE_SUBREG,

    _EVENT_MODB_RS485_1_HANDLE,
    _EVENT_MODB_RS485_2_HANDLE,
    _EVENT_MODB_TCP_HANDLE,
    
    _EVENT_PTR_TEMP,
    
    _EVENT_MODB_END,
}eKindEventModb;

typedef enum
{
    _PORT_ETH_TCP,  
    _PORT_RS485_1,   
    _PORT_RS485_2,    
}eKindEPortModbus;

typedef enum
{
    _SENSOR_DISCONNECT = 0,
    _SENSOR_CONNECT,
}eKindStateSensor;

typedef enum
{
    _MODB_UNRESPOND = 0,
    _MODB_RESPOND,
}eKindStateModbRespond;

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
#ifdef MODBUS_SENSOR_SAOVIET
    _E_PH_VALUE=0,
    _E_PH_S_SENSOR,
    _E_PH_S_VALUE,
    
    _E_CLO_SEND_PH,
    _E_CLO_VALUE,
    _E_CLO_S_SENSOR,
    _E_CLO_S_VALUE,
    
    _E_EC_VALUE,
    _E_EC_S_SENSOR,
    _E_EC_S_VALUE,
    
    _E_TURB_VALUE,
    _E_TURB_S_SENSOR,
    _E_TURB_S_VALUE,
    
    _E_COD_VALUE,
    _E_COD_S_SENSOR,
    _E_COD_S_VALUE,
    
    _E_TSS_VALUE,
    _E_TSS_S_SENSOR,
    _E_TSS_S_VALUE,
    
    _E_NH4_VALUE,
    _E_NH4_S_SENSOR,
    _E_NH4_S_VALUE,
    
    _E_DO_SALT,
    _E_DO_VALUE,
    _E_DO_S_SENSOR,
    _E_DO_S_VALUE,
    
    _E_SALT_VALUE,
    _E_SALT_S_SENSOR,
    _E_SALT_S_VALUE,
    
    _E_TDS_VALUE,
    _E_TDS_S_SENSOR,
    _E_TDS_S_VALUE,
    
    _E_NO3_VALUE,
    _E_NO3_S_SENSOR,
    _E_NO3_S_VALUE,
    
    _E_TEMP_VALUE,
    _E_TEMP_S_SENSOR,
    _E_TEMP_S_VALUE,
    
    _E_PH1_WRITE,
    _E_PH2_WRITE,
    _E_TURB1_WRITE,
    _E_TURB2_WRITE,
#else
    _E_PH_S_SENSOR,
    _E_TEMP_S_SENSOR,
    _E_PH_S_VALUE,
    _E_TEMP_S_VALUE,
    _E_PH_VALUE,
    _E_TEMP_VALUE,

    _E_DO_S_SENSOR,
    _E_DO_S_VALUE,
    _E_DO_VALUE,
    
    _E_NH4_S_SENSOR,
    _E_NH4_S_VALUE,
    _E_NH4_VALUE,
    
    _E_TSS_S_SENSOR,
    _E_TSS_S_VALUE,
    _E_TSS_VALUE,
    
    _E_COD_S_SENSOR,
    _E_COD_S_VALUE,
    _E_COD_VALUE,
#endif
    _E_MODB_SS_END,
}eKindStateModbReg;

typedef struct
{   
    uint8_t State_u8;   //Mat ket noi hay khong
    uint8_t Scale_u8;
    int32_t Value_i32;
}Struct_SS_Value;

typedef struct 
{
    uint8_t CountDisconnectModb_1;
    uint8_t CountDisconnectModb_2;
  
    uint8_t State_Wait_Calib;
}Struct_Hanlde_Modb;

typedef struct
{
    uint8_t  eKind;                  //Name Register
    uint8_t  Block;
    uint8_t  *State;                 //Trang thai su dung (On/Off)
    uint8_t  cmdRW;                  //0: Read, 1: Write
    uint8_t  *idDev;                  //ID cua thiet bi
    uint8_t  cmdLen;                 //So thanh ghi can doc
    uint16_t cmdAddr;                //Addr thanh ghi 1234(Dec)
    uint8_t  vFormat;                //Kieu du lieu(float, uint32_t, int32_t, uint16_t, int16_t)
    uint8_t  vBeLe;                  //Kieu giai ma
    float    vScale;                 //Scale du lieu 0.01, 0.1, 1, 10 (ket qua do nhan voi scale)
    void    *subReg;                //Gia tri viet vao thanh ghi lay tu thiet bi so 1|2|3...
    uint8_t  nPort;                  //Modbus port: 1
    void    *vReturn;                  
    uint8_t  *nConnect;
}Struct_RegSensor;

typedef struct
{
    uint8_t eKind;
    uint8_t ID_Modbus;              // ID su dung trong giao thuc Modbus
    char    Name[10];
    char    Unit[10];
    
    uint8_t sUser;                  //Trang thai su dung hay khong
    uint8_t nConnect_u8;            //Dem so lan connect
    uint8_t sConnect_u8;            //Trang thai connect
    
    uint8_t stateSensor;            //Dung cho cam bien Chau Au
    uint8_t stateValue;             //Dung cho cam bien Chau Au
    uint8_t sUnit;                  //Dung cho cam bien Chau Au
    
    float   Value_f;                //Gia tri do cam bien
    float   Temp_f;                 //Nhiet do
    float   Offset_f;               //Offset gia tri do
}Struct_Modb_Measure;

typedef struct
{
    float pH;
    float Salt_PSU;
}Struct_Modb_SubReg;

typedef struct
{
    uint8_t Flag;
    uint8_t aData[50];
    uint8_t length;
}Struct_TransModbusTCP;

typedef struct
{
    uint8_t iHandle; 
    uint8_t iStartBlock;
    uint8_t iEndBlock;
    uint8_t iReg;
    uint16_t iAddr;
    
    uint16_t Transaction_TCP;
}Struct_CtrlModbM;

extern sEvent_struct        sEventAppModb[];

extern Struct_Hanlde_Modb          sHandleModb;
extern sData                       sDataRecvTCP;
extern Struct_TransModbusTCP       sTransModTCP;
/*====================Function Handle====================*/

uint8_t    AppModb_Task(void);
void       Init_AppModb(void);

void       Modb_Para_Init(void);

void       Init_Parameter_Sensor(void);

void       Init_UartRs485(void);
void       Init_UartRs485_2(void);

uint8_t    ModbusTCP_Check_Format(uint8_t SlaveID, uint16_t nRegis,
                                  sData *pSource, sData *Content);
uint8_t    Modbus_RTU_Check_Format(uint8_t SlaveID, uint16_t nRegis,
                                  sData *pSource, sData *Content);
uint8_t    Handle_Trans_Modb(uint8_t Port, Struct_RegSensor  sReg[], Struct_CtrlModbM  *sCtrl, sData *sFrame);
uint8_t    Handle_Recv_Modb(Struct_RegSensor  sReg[], Struct_CtrlModbM  *sCtrl, sData sRecv);
void       Config_RegSen_Read(uint8_t Kind,uint8_t *ID, uint8_t *User, void *Return, uint8_t *nConnect);
void       Config_RegSen_Write(uint8_t Kind,uint8_t *ID, uint8_t *User, void *subReg, uint8_t *nConnect);
#endif

