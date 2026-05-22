#include "user_app_modb_m.h"
#include "user_modbus_rtu.h"
#include "user_internal_mem.h"
#include "user_rs485.h"
#include "user_app_wm.h"
#include "user_define.h"
#include "user_modem.h"
/*=========================Fucntion Static=========================*/
static uint8_t fevent_modb_entry(uint8_t event);

static uint8_t fevent_modb_wait_calib(uint8_t event);
static uint8_t fevent_modb_refresh(uint8_t event);

static uint8_t fevent_modb_handle_subreg(uint8_t event);
static uint8_t fevent_modb_transmit_data(uint8_t event);
static uint8_t fevent_modbus_receive_data(uint8_t event);
static uint8_t fevent_ptr_temp(uint8_t event);
/*==============================Struct=============================*/
sEvent_struct               sEventAppModb[]=
{
  {_EVENT_MODB_ENTRY,              1, 5, 30000,            fevent_modb_entry},            //Doi slave khoi dong moi truyen opera
  
  {_EVENT_MODB_WAIT_CALIB,         0, 5, 5000,             fevent_modb_wait_calib},
  {_EVENT_MODB_REFRESH,            0, 5, 60000,            fevent_modb_refresh},
  
  {_EVENT_MODB_HANDLE_SUBREG,      1, 5, 500,              fevent_modb_handle_subreg},
  
  {_EVENT_MODB_TRANSMIT_DATA,      1, 5, 100,              fevent_modb_transmit_data},
  {_EVENT_MODBUS_RECEIVE_DATA,     0, 5, 900,              fevent_modbus_receive_data},
  
  {_EVENT_PTR_TEMP,                1, 5, 100,              fevent_ptr_temp},
};
uint16_t CountBufferHandleRecv = 0;

static uint8_t aDATA_RECV_MODBUS_TCP [DATA_BUF_SIZE];
sData   sDataRecvTCP = {(uint8_t *) &aDATA_RECV_MODBUS_TCP[0], 0};

extern sData sUart485;
extern sData sUart485_2;

Struct_TransModbusTCP       sTransModTCP = {0};

Struct_Modb_SubReg          sModbSubReg = {0};
Struct_Data_Sensor_Measure  sDataSensorMeasure = {0};
Struct_Hanlde_Modb          sHandleModb = {0};

Struct_RegSensor            sRegSensor[] =
{
#ifdef MODBUS_SENSOR_SAOVIET
    //eKind         //State  //cmdRW //idDev     //cmdLen  //Addr   //vFormat  //vBeLe  //vScale  //subReg  //nPort         //vReturn  //nConnect
  {_E_PH_VALUE,     NULL,    0,      ID_SS_PH,   2,        0x0002,  _ETYPE_F,  _E_WS,   1,        NULL,     PORT_RS485_1,       NULL,      NULL},
  {_E_PH_S_SENSOR,  NULL,    0,      ID_SS_PH,   1,        0x000A,  _ETYPE_U8, _E_BE,   1,        NULL,     PORT_RS485_1,       NULL,      NULL},
  {_E_PH_S_VALUE,   NULL,    0,      ID_SS_PH,   1,        0x000B,  _ETYPE_U8, _E_BE,   1,        NULL,     PORT_RS485_1,       NULL,      NULL},
  
  {_E_CLO_SEND_PH,  NULL,    1,      ID_SS_CLO,  2,        0x0006,  _ETYPE_F,  _E_WS,   1,        NULL,     PORT_RS485_1,       NULL,      NULL},
  {_E_CLO_VALUE,    NULL,    0,      ID_SS_CLO,  2,        0x0002,  _ETYPE_F,  _E_WS,   1,        NULL,     PORT_RS485_1,       NULL,      NULL},
  {_E_CLO_S_SENSOR, NULL,    0,      ID_SS_CLO,  1,        0x000A,  _ETYPE_U8, _E_BE,   1,        NULL,     PORT_RS485_1,       NULL,      NULL},
  {_E_CLO_S_VALUE,  NULL,    0,      ID_SS_CLO,  1,        0x000B,  _ETYPE_U8, _E_BE,   1,        NULL,     PORT_RS485_1,       NULL,      NULL},
  
  {_E_EC_VALUE,     NULL,    0,      ID_SS_EC,   2,        0x0002,  _ETYPE_F,  _E_WS,   1,        NULL,     PORT_RS485_1,       NULL,      NULL},
  {_E_EC_S_SENSOR,  NULL,    0,      ID_SS_EC,   1,        0x000A,  _ETYPE_U8, _E_BE,   1,        NULL,     PORT_RS485_1,       NULL,      NULL},
  {_E_EC_S_VALUE,   NULL,    0,      ID_SS_EC,   1,        0x000B,  _ETYPE_U8, _E_BE,   1,        NULL,     PORT_RS485_1,       NULL,      NULL},
  
  {_E_TURB_VALUE,   NULL,    0,      ID_SS_TURB, 2,        0x0002,  _ETYPE_F,  _E_WS,   1,        NULL,     PORT_RS485_1,       NULL,      NULL},
  {_E_TURB_S_SENSOR,NULL,    0,      ID_SS_TURB, 1,        0x000A,  _ETYPE_U8, _E_BE,   1,        NULL,     PORT_RS485_1,       NULL,      NULL},
  {_E_TURB_S_VALUE, NULL,    0,      ID_SS_TURB, 1,        0x000B,  _ETYPE_U8, _E_BE,   1,        NULL,     PORT_RS485_1,       NULL,      NULL},
  
  {_E_COD_VALUE,    NULL,    0,      ID_SS_COD,  2,        0x0002,  _ETYPE_F,  _E_WS,   1,        NULL,     PORT_RS485_1,       NULL,      NULL},
  {_E_COD_S_SENSOR, NULL,    0,      ID_SS_COD,  1,        0x000A,  _ETYPE_U8, _E_BE,   1,        NULL,     PORT_RS485_1,       NULL,      NULL},
  {_E_COD_S_VALUE,  NULL,    0,      ID_SS_COD,  1,        0x000B,  _ETYPE_U8, _E_BE,   1,        NULL,     PORT_RS485_1,       NULL,      NULL},
  
  {_E_TSS_VALUE,    NULL,    0,      ID_SS_TSS,  2,        0x0002,  _ETYPE_F,  _E_WS,   1,        NULL,     PORT_RS485_1,       NULL,      NULL},
  {_E_TSS_S_SENSOR, NULL,    0,      ID_SS_TSS,  1,        0x000A,  _ETYPE_U8, _E_BE,   1,        NULL,     PORT_RS485_1,       NULL,      NULL},
  {_E_TSS_S_VALUE,  NULL,    0,      ID_SS_TSS,  1,        0x000B,  _ETYPE_U8, _E_BE,   1,        NULL,     PORT_RS485_1,       NULL,      NULL},
  
  {_E_NH4_VALUE,    NULL,    0,      ID_SS_NH4,  2,        0x0002,  _ETYPE_F,  _E_WS,   1,        NULL,     PORT_RS485_1,       NULL,      NULL},
  {_E_NH4_S_SENSOR, NULL,    0,      ID_SS_NH4,  1,        0x000A,  _ETYPE_U8, _E_BE,   1,        NULL,     PORT_RS485_1,       NULL,      NULL},
  {_E_NH4_S_VALUE,  NULL,    0,      ID_SS_NH4,  1,        0x000B,  _ETYPE_U8, _E_BE,   1,        NULL,     PORT_RS485_1,       NULL,      NULL},
  
  {_E_DO_SALT,      NULL,    1,      ID_SS_DO,   2,        0x0008,  _ETYPE_F,  _E_WS,   1,        NULL,     PORT_RS485_1,       NULL,      NULL},
  {_E_DO_VALUE,     NULL,    0,      ID_SS_DO,   2,        0x0002,  _ETYPE_F,  _E_WS,   1,        NULL,     PORT_RS485_1,       NULL,      NULL},
  {_E_DO_S_SENSOR,  NULL,    0,      ID_SS_DO,   1,        0x000A,  _ETYPE_U8, _E_BE,   1,        NULL,     PORT_RS485_1,       NULL,      NULL},
  {_E_DO_S_VALUE,   NULL,    0,      ID_SS_DO,   1,        0x000B,  _ETYPE_U8, _E_BE,   1,        NULL,     PORT_RS485_1,       NULL,      NULL},
  
  {_E_SALT_VALUE,   NULL,    0,      ID_SS_EC,   2,        0x0008,  _ETYPE_F,  _E_WS,   1,        NULL,     PORT_RS485_1,       NULL,      NULL},
  {_E_SALT_S_SENSOR,NULL,    0,      ID_SS_EC,   1,        0x000A,  _ETYPE_U8, _E_BE,   1,        NULL,     PORT_RS485_1,       NULL,      NULL},
  {_E_SALT_S_VALUE, NULL,    0,      ID_SS_EC,   1,        0x000B,  _ETYPE_U8, _E_BE,   1,        NULL,     PORT_RS485_1,       NULL,      NULL},
  
  {_E_TDS_VALUE,    NULL,    0,      ID_SS_EC,   2,        0x0006,  _ETYPE_F,  _E_WS,   1,        NULL,     PORT_RS485_1,       NULL,      NULL},
  {_E_TDS_S_SENSOR, NULL,    0,      ID_SS_EC,   1,        0x000A,  _ETYPE_U8, _E_BE,   1,        NULL,     PORT_RS485_1,       NULL,      NULL},
  {_E_TDS_S_VALUE,  NULL,    0,      ID_SS_EC,   1,        0x000B,  _ETYPE_U8, _E_BE,   1,        NULL,     PORT_RS485_1,       NULL,      NULL},
  
  {_E_NO3_VALUE,    NULL,    0,      ID_SS_NO3,   2,       0x0002,  _ETYPE_F,  _E_WS,   1,        NULL,     PORT_RS485_1,       NULL,      NULL},
  {_E_NO3_S_SENSOR, NULL,    0,      ID_SS_NO3,   1,       0x000A,  _ETYPE_U8, _E_BE,   1,        NULL,     PORT_RS485_1,       NULL,      NULL},
  {_E_NO3_S_VALUE,  NULL,    0,      ID_SS_NO3,   1,       0x000B,  _ETYPE_U8, _E_BE,   1,        NULL,     PORT_RS485_1,       NULL,      NULL},
  
  {_E_TEMP_VALUE,    NULL,    0,     ID_SS_TEMP,   2,      0x0004,  _ETYPE_F,  _E_WS,   1,        NULL,     PORT_RS485_1,       NULL,      NULL},
  {_E_TEMP_S_SENSOR, NULL,    0,     ID_SS_TEMP,   1,      0x000A,  _ETYPE_U8, _E_BE,   1,        NULL,     PORT_RS485_1,       NULL,      NULL},
  {_E_TEMP_S_VALUE,  NULL,    0,     ID_SS_TEMP,   1,      0x000C,  _ETYPE_U8, _E_BE,   1,        NULL,     PORT_RS485_1,       NULL,      NULL},
#else 
  {_E_PH_VALUE,     NULL,    0,      ID_SS_PH,   2,        4,      _ETYPE_F,  _E_BE,   1,        NULL,     PORT_MODB_TCP,       NULL,      NULL},
  {_E_PH_S_SENSOR,  NULL,    0,      ID_SS_PH,   1,        0,      _ETYPE_U8, _E_BE,   1,        NULL,     PORT_MODB_TCP,       NULL,      NULL},
  {_E_PH_S_VALUE,   NULL,    0,      ID_SS_PH,   1,        3,      _ETYPE_U8, _E_BE,   1,        NULL,     PORT_MODB_TCP,       NULL,      NULL},
 
  {_E_CLO_VALUE,    NULL,    0,      ID_SS_CLO,  2,        0x0002,  _ETYPE_F,  _E_WS,   1,        NULL,     PORT_RS485_1,       NULL,      NULL},
  {_E_CLO_TEMP,     NULL,    0,      ID_SS_CLO,  2,        0x0004,  _ETYPE_F,  _E_WS,   1,        NULL,     PORT_RS485_1,       NULL,      NULL},
    
  {_E_EC_VALUE,     NULL,    0,      ID_SS_EC,   2,        0x0002,  _ETYPE_F,  _E_WS,   1,        NULL,     PORT_RS485_1,       NULL,      NULL},
  {_E_EC_TEMP,      NULL,    0,      ID_SS_EC,   2,        0x0004,  _ETYPE_F,  _E_WS,   1,        NULL,     PORT_RS485_1,       NULL,      NULL},
    
  {_E_TURB_VALUE,   NULL,    0,      ID_SS_TURB, 2,        0x0002,  _ETYPE_F,  _E_WS,   1,        NULL,     PORT_RS485_1,       NULL,      NULL},
  {_E_TURB_TEMP,    NULL,    0,      ID_SS_TURB, 2,        0x0004,  _ETYPE_F,  _E_WS,   1,        NULL,     PORT_RS485_1,       NULL,      NULL},
    
  {_E_COD_VALUE,    NULL,    0,      ID_SS_COD,  2,        36,      _ETYPE_F,  _E_BE,   1,        NULL,     PORT_MODB_TCP,       NULL,      NULL},
  {_E_COD_S_SENSOR, NULL,    0,      ID_SS_COD,  1,        32,      _ETYPE_U8, _E_BE,   1,        NULL,     PORT_MODB_TCP,       NULL,      NULL},
  {_E_COD_S_VALUE,  NULL,    0,      ID_SS_COD,  1,        35,      _ETYPE_U8, _E_BE,   1,        NULL,     PORT_MODB_TCP,       NULL,      NULL},
  
  {_E_TSS_VALUE,    NULL,    0,      ID_SS_TSS,  2,        28,      _ETYPE_F,  _E_BE,   1000,     NULL,     PORT_MODB_TCP,       NULL,      NULL},
  {_E_TSS_S_SENSOR, NULL,    0,      ID_SS_TSS,  1,        24,      _ETYPE_U8, _E_BE,   1,        NULL,     PORT_MODB_TCP,       NULL,      NULL},
  {_E_TSS_S_VALUE,  NULL,    0,      ID_SS_TSS,  1,        27,      _ETYPE_U8, _E_BE,   1,        NULL,     PORT_MODB_TCP,       NULL,      NULL},
  
  {_E_NH4_VALUE,    NULL,    0,      ID_SS_NH4,  2,        20,      _ETYPE_F,  _E_BE,   1,        NULL,     PORT_MODB_TCP,       NULL,      NULL},
  {_E_NH4_S_SENSOR, NULL,    0,      ID_SS_NH4,  1,        16,      _ETYPE_U8, _E_BE,   1,        NULL,     PORT_MODB_TCP,       NULL,      NULL},
  {_E_NH4_S_VALUE,  NULL,    0,      ID_SS_NH4,  1,        19,      _ETYPE_U8, _E_BE,   1,        NULL,     PORT_MODB_TCP,       NULL,      NULL},
    
  {_E_DO_VALUE,     NULL,    0,      ID_SS_DO,   2,        12,      _ETYPE_F,  _E_BE,   1,        NULL,     PORT_MODB_TCP,       NULL,      NULL},
  {_E_DO_S_SENSOR,  NULL,    0,      ID_SS_DO,   1,        8,      _ETYPE_U8, _E_BE,   1,        NULL,      PORT_MODB_TCP,       NULL,      NULL},
  {_E_DO_S_VALUE,   NULL,    0,      ID_SS_DO,   1,        11,      _ETYPE_U8, _E_BE,   1,        NULL,     PORT_MODB_TCP,       NULL,      NULL},
  
  {_E_SALT_VALUE,   NULL,    0,      ID_SS_EC,   2,        0x0008,  _ETYPE_F,  _E_WS,   1,        NULL,     PORT_RS485_1,       NULL,      NULL},
  {_E_SALT_TEMP,    NULL,    0,      ID_SS_EC,   2,        0x0004,  _ETYPE_F,  _E_WS,   1,        NULL,     PORT_RS485_1,       NULL,      NULL},

  {_E_TDS_VALUE,    NULL,    0,      ID_SS_EC,   2,        0x0006,  _ETYPE_F,  _E_WS,   1,        NULL,     PORT_RS485_1,       NULL,      NULL},
  {_E_TDS_TEMP,     NULL,    0,      ID_SS_EC,   2,        0x0004,  _ETYPE_F,  _E_WS,   1,        NULL,     PORT_RS485_1,       NULL,      NULL},

  {_E_NO3_VALUE,    NULL,    0,      ID_SS_NO3,  2,        0x0002,  _ETYPE_F,  _E_WS,   1,        NULL,     PORT_RS485_1,       NULL,      NULL},
  {_E_NO3_TEMP,     NULL,    0,      ID_SS_NO3,  2,        0x0004,  _ETYPE_F,  _E_WS,   1,        NULL,     PORT_RS485_1,       NULL,      NULL},

  {_E_TEMP_VALUE,    NULL,    0,      ID_SS_TEMP,  2,        6,    _ETYPE_F,  _E_BE,   1,        NULL,     PORT_MODB_TCP,       NULL,      NULL},
  {_E_TEMP_S_SENSOR, NULL,    0,      ID_SS_TEMP,  1,        0,    _ETYPE_U8, _E_BE,   1,        NULL,     PORT_MODB_TCP,       NULL,      NULL},
  {_E_TEMP_S_VALUE,  NULL,    0,      ID_SS_TEMP,  1,        3,    _ETYPE_U8, _E_BS,   1,        NULL,     PORT_MODB_TCP,       NULL,      NULL},

#endif
};
/*========================Function Handle========================*/
void       RS485_Para_Init(void)
{
#ifdef MODBUS_SENSOR_SAOVIET
    //PH
    sRegSensor[_E_PH_VALUE].State = &sModbMeasure[_SS_PH].sUser;
    sRegSensor[_E_PH_VALUE].vReturn = &sModbMeasure[_SS_PH].Value_f;
    sRegSensor[_E_PH_VALUE].nConnect = &sModbMeasure[_SS_PH].nConnect_u8;
    
    sRegSensor[_E_PH_S_SENSOR].State = &sModbMeasure[_SS_PH].sUser;
    sRegSensor[_E_PH_S_SENSOR].vReturn = &sModbMeasure[_SS_PH].stateSensor;
    sRegSensor[_E_PH_S_SENSOR].nConnect = &sModbMeasure[_SS_PH].nConnect_u8;
    
    sRegSensor[_E_PH_S_VALUE].State = &sModbMeasure[_SS_PH].sUser;
    sRegSensor[_E_PH_S_VALUE].vReturn = &sModbMeasure[_SS_PH].stateValue;
    sRegSensor[_E_PH_S_VALUE].nConnect = &sModbMeasure[_SS_PH].nConnect_u8;
    
    //CLO
    sRegSensor[_E_CLO_SEND_PH].State = &sModbMeasure[_SS_CLO].sUser;
    sRegSensor[_E_CLO_SEND_PH].subReg = &sModbSubReg.pH;
    sRegSensor[_E_CLO_SEND_PH].nConnect = &sModbMeasure[_SS_CLO].nConnect_u8;
    
    sRegSensor[_E_CLO_VALUE].State = &sModbMeasure[_SS_CLO].sUser;
    sRegSensor[_E_CLO_VALUE].vReturn = &sModbMeasure[_SS_CLO].Value_f;
    sRegSensor[_E_CLO_VALUE].nConnect = &sModbMeasure[_SS_CLO].nConnect_u8;
    
    sRegSensor[_E_CLO_S_SENSOR].State = &sModbMeasure[_SS_CLO].sUser;
    sRegSensor[_E_CLO_S_SENSOR].vReturn = &sModbMeasure[_SS_CLO].stateSensor;
    sRegSensor[_E_CLO_S_SENSOR].nConnect = &sModbMeasure[_SS_CLO].nConnect_u8;
    
    sRegSensor[_E_CLO_S_VALUE].State = &sModbMeasure[_SS_CLO].sUser;
    sRegSensor[_E_CLO_S_VALUE].vReturn = &sModbMeasure[_SS_CLO].stateValue;
    sRegSensor[_E_CLO_S_VALUE].nConnect = &sModbMeasure[_SS_CLO].nConnect_u8;
    
    //EC
    sRegSensor[_E_EC_VALUE].State = &sModbMeasure[_SS_EC].sUser;
    sRegSensor[_E_EC_VALUE].vReturn = &sModbMeasure[_SS_EC].Value_f;
    sRegSensor[_E_EC_VALUE].nConnect = &sModbMeasure[_SS_EC].nConnect_u8;
    
    sRegSensor[_E_EC_S_SENSOR].State = &sModbMeasure[_SS_EC].sUser;
    sRegSensor[_E_EC_S_SENSOR].vReturn = &sModbMeasure[_SS_EC].stateSensor;
    sRegSensor[_E_EC_S_SENSOR].nConnect = &sModbMeasure[_SS_EC].nConnect_u8;
    
    sRegSensor[_E_EC_S_VALUE].State = &sModbMeasure[_SS_EC].sUser;
    sRegSensor[_E_EC_S_VALUE].vReturn = &sModbMeasure[_SS_EC].stateValue;
    sRegSensor[_E_EC_S_VALUE].nConnect = &sModbMeasure[_SS_EC].nConnect_u8;

    //TURB
    sRegSensor[_E_TURB_VALUE].State = &sModbMeasure[_SS_TURB].sUser;
    sRegSensor[_E_TURB_VALUE].vReturn = &sModbMeasure[_SS_TURB].Value_f;
    sRegSensor[_E_TURB_VALUE].nConnect = &sModbMeasure[_SS_TURB].nConnect_u8;
    
    sRegSensor[_E_TURB_S_SENSOR].State = &sModbMeasure[_SS_TURB].sUser;
    sRegSensor[_E_TURB_S_SENSOR].vReturn = &sModbMeasure[_SS_TURB].stateSensor;
    sRegSensor[_E_TURB_S_SENSOR].nConnect = &sModbMeasure[_SS_TURB].nConnect_u8;
    
    sRegSensor[_E_TURB_S_VALUE].State = &sModbMeasure[_SS_TURB].sUser;
    sRegSensor[_E_TURB_S_VALUE].vReturn = &sModbMeasure[_SS_TURB].stateValue;
    sRegSensor[_E_TURB_S_VALUE].nConnect = &sModbMeasure[_SS_TURB].nConnect_u8;
    
    //COD
    sRegSensor[_E_COD_VALUE].State = &sModbMeasure[_SS_COD].sUser;
    sRegSensor[_E_COD_VALUE].vReturn = &sModbMeasure[_SS_COD].Value_f;
    sRegSensor[_E_COD_VALUE].nConnect = &sModbMeasure[_SS_COD].nConnect_u8;
    
    sRegSensor[_E_COD_S_SENSOR].State = &sModbMeasure[_SS_COD].sUser;
    sRegSensor[_E_COD_S_SENSOR].vReturn = &sModbMeasure[_SS_COD].stateSensor;
    sRegSensor[_E_COD_S_SENSOR].nConnect = &sModbMeasure[_SS_COD].nConnect_u8;
    
    sRegSensor[_E_COD_S_VALUE].State = &sModbMeasure[_SS_COD].sUser;
    sRegSensor[_E_COD_S_VALUE].vReturn = &sModbMeasure[_SS_COD].stateValue;
    sRegSensor[_E_COD_S_VALUE].nConnect = &sModbMeasure[_SS_COD].nConnect_u8;
    
    //TSS
    sRegSensor[_E_TSS_VALUE].State = &sModbMeasure[_SS_TSS].sUser;
    sRegSensor[_E_TSS_VALUE].vReturn = &sModbMeasure[_SS_TSS].Value_f;
    sRegSensor[_E_TSS_VALUE].nConnect = &sModbMeasure[_SS_TSS].nConnect_u8;
    
    sRegSensor[_E_TSS_S_SENSOR].State = &sModbMeasure[_SS_TSS].sUser;
    sRegSensor[_E_TSS_S_SENSOR].vReturn = &sModbMeasure[_SS_TSS].stateSensor;
    sRegSensor[_E_TSS_S_SENSOR].nConnect = &sModbMeasure[_SS_TSS].nConnect_u8;
    
    sRegSensor[_E_TSS_S_VALUE].State = &sModbMeasure[_SS_TSS].sUser;
    sRegSensor[_E_TSS_S_VALUE].vReturn = &sModbMeasure[_SS_TSS].stateValue;
    sRegSensor[_E_TSS_S_VALUE].nConnect = &sModbMeasure[_SS_TSS].nConnect_u8;

    //NH4
    sRegSensor[_E_NH4_VALUE].State = &sModbMeasure[_SS_NH4].sUser;
    sRegSensor[_E_NH4_VALUE].vReturn = &sModbMeasure[_SS_NH4].Value_f;
    sRegSensor[_E_NH4_VALUE].nConnect = &sModbMeasure[_SS_NH4].nConnect_u8;
    
    sRegSensor[_E_NH4_S_SENSOR].State = &sModbMeasure[_SS_NH4].sUser;
    sRegSensor[_E_NH4_S_SENSOR].vReturn = &sModbMeasure[_SS_NH4].stateSensor;
    sRegSensor[_E_NH4_S_SENSOR].nConnect = &sModbMeasure[_SS_NH4].nConnect_u8;
    
    sRegSensor[_E_NH4_S_VALUE].State = &sModbMeasure[_SS_NH4].sUser;
    sRegSensor[_E_NH4_S_VALUE].vReturn = &sModbMeasure[_SS_NH4].stateValue;
    sRegSensor[_E_NH4_S_VALUE].nConnect = &sModbMeasure[_SS_NH4].nConnect_u8;
    
    //DO
    sRegSensor[_E_DO_SALT].State    = &sModbMeasure[_SS_DO].sUser;
    sRegSensor[_E_DO_SALT].subReg   = &sModbSubReg.Salt_PSU;
    sRegSensor[_E_DO_SALT].nConnect = &sModbMeasure[_SS_DO].nConnect_u8;
    
    sRegSensor[_E_DO_VALUE].State = &sModbMeasure[_SS_DO].sUser;
    sRegSensor[_E_DO_VALUE].vReturn = &sModbMeasure[_SS_DO].Value_f;
    sRegSensor[_E_DO_VALUE].nConnect = &sModbMeasure[_SS_DO].nConnect_u8;
    
    sRegSensor[_E_DO_S_SENSOR].State = &sModbMeasure[_SS_DO].sUser;
    sRegSensor[_E_DO_S_SENSOR].vReturn = &sModbMeasure[_SS_DO].stateSensor;
    sRegSensor[_E_DO_S_SENSOR].nConnect = &sModbMeasure[_SS_DO].nConnect_u8;
    
    sRegSensor[_E_DO_S_VALUE].State = &sModbMeasure[_SS_DO].sUser;
    sRegSensor[_E_DO_S_VALUE].vReturn = &sModbMeasure[_SS_DO].stateValue;
    sRegSensor[_E_DO_S_VALUE].nConnect = &sModbMeasure[_SS_DO].nConnect_u8;

    //SALT
    sRegSensor[_E_SALT_VALUE].State = &sModbMeasure[_SS_SALT].sUser;
    sRegSensor[_E_SALT_VALUE].vReturn = &sModbMeasure[_SS_SALT].Value_f;
    sRegSensor[_E_SALT_VALUE].nConnect = &sModbMeasure[_SS_SALT].nConnect_u8;
    
    sRegSensor[_E_SALT_S_SENSOR].State = &sModbMeasure[_SS_SALT].sUser;
    sRegSensor[_E_SALT_S_SENSOR].vReturn = &sModbMeasure[_SS_SALT].stateSensor;
    sRegSensor[_E_SALT_S_SENSOR].nConnect = &sModbMeasure[_SS_SALT].nConnect_u8;
    
    sRegSensor[_E_SALT_S_VALUE].State = &sModbMeasure[_SS_SALT].sUser;
    sRegSensor[_E_SALT_S_VALUE].vReturn = &sModbMeasure[_SS_SALT].stateValue;
    sRegSensor[_E_SALT_S_VALUE].nConnect = &sModbMeasure[_SS_SALT].nConnect_u8;

    //TDS
    sRegSensor[_E_TDS_VALUE].State = &sModbMeasure[_SS_TDS].sUser;
    sRegSensor[_E_TDS_VALUE].vReturn = &sModbMeasure[_SS_TDS].Value_f;
    sRegSensor[_E_TDS_VALUE].nConnect = &sModbMeasure[_SS_TDS].nConnect_u8;
    
    sRegSensor[_E_TDS_S_SENSOR].State = &sModbMeasure[_SS_TDS].sUser;
    sRegSensor[_E_TDS_S_SENSOR].vReturn = &sModbMeasure[_SS_TDS].stateSensor;
    sRegSensor[_E_TDS_S_SENSOR].nConnect = &sModbMeasure[_SS_TDS].nConnect_u8;
    
    sRegSensor[_E_TDS_S_VALUE].State = &sModbMeasure[_SS_TDS].sUser;
    sRegSensor[_E_TDS_S_VALUE].vReturn = &sModbMeasure[_SS_TDS].stateValue;
    sRegSensor[_E_TDS_S_VALUE].nConnect = &sModbMeasure[_SS_TDS].nConnect_u8;
    
    //NO3
    sRegSensor[_E_NO3_VALUE].State = &sModbMeasure[_SS_NO3].sUser;
    sRegSensor[_E_NO3_VALUE].vReturn = &sModbMeasure[_SS_NO3].Value_f;
    sRegSensor[_E_NO3_VALUE].nConnect = &sModbMeasure[_SS_NO3].nConnect_u8;
    
    sRegSensor[_E_NO3_S_SENSOR].State = &sModbMeasure[_SS_NO3].sUser;
    sRegSensor[_E_NO3_S_SENSOR].vReturn = &sModbMeasure[_SS_NO3].stateSensor;
    sRegSensor[_E_NO3_S_SENSOR].nConnect = &sModbMeasure[_SS_NO3].nConnect_u8;
    
    sRegSensor[_E_NO3_S_VALUE].State = &sModbMeasure[_SS_NO3].sUser;
    sRegSensor[_E_NO3_S_VALUE].vReturn = &sModbMeasure[_SS_NO3].stateValue;
    sRegSensor[_E_NO3_S_VALUE].nConnect = &sModbMeasure[_SS_NO3].nConnect_u8;

    //TEMP
    sRegSensor[_E_TEMP_VALUE].State = &sModbMeasure[_SS_TEMP].sUser;
    sRegSensor[_E_TEMP_VALUE].vReturn = &sModbMeasure[_SS_TEMP].Value_f;
    sRegSensor[_E_TEMP_VALUE].nConnect = &sModbMeasure[_SS_TEMP].nConnect_u8;
    
    sRegSensor[_E_TEMP_S_SENSOR].State = &sModbMeasure[_SS_TEMP].sUser;
    sRegSensor[_E_TEMP_S_SENSOR].vReturn = &sModbMeasure[_SS_TEMP].stateSensor;
    sRegSensor[_E_TEMP_S_SENSOR].nConnect = &sModbMeasure[_SS_TEMP].nConnect_u8;
    
    sRegSensor[_E_TEMP_S_VALUE].State = &sModbMeasure[_SS_TEMP].sUser;
    sRegSensor[_E_TEMP_S_VALUE].vReturn = &sModbMeasure[_SS_TEMP].stateValue;
    sRegSensor[_E_TEMP_S_VALUE].nConnect = &sModbMeasure[_SS_TEMP].nConnect_u8;
#else
    //----------------------------Ph------------------------------
    sRegSensor[_E_PH_VALUE].State = &sModbMeasure[_SS_PH].sUser;
    sRegSensor[_E_PH_VALUE].vReturn = &sModbMeasure[_SS_PH].Value_f;
    sRegSensor[_E_PH_VALUE].nConnect = &sModbMeasure[_SS_PH].nConnect_u8;
    
    sRegSensor[_E_PH_S_SENSOR].State = &sModbMeasure[_SS_PH].sUser;
    sRegSensor[_E_PH_S_SENSOR].vReturn = &sModbMeasure[_SS_PH].stateSensor;
    sRegSensor[_E_PH_S_SENSOR].nConnect = &sModbMeasure[_SS_PH].nConnect_u8;
    
    sRegSensor[_E_PH_S_VALUE].State = &sModbMeasure[_SS_PH].sUser;
    sRegSensor[_E_PH_S_VALUE].vReturn = &sModbMeasure[_SS_PH].stateValue;
    sRegSensor[_E_PH_S_VALUE].nConnect = &sModbMeasure[_SS_PH].nConnect_u8;
    
    //----------------------------Clo-----------------------------
    sRegSensor[_E_CLO_VALUE].State = &sModbMeasure[_SS_CLO].sUser;
    sRegSensor[_E_CLO_VALUE].vReturn = &sModbMeasure[_SS_CLO].Value_f;
    sRegSensor[_E_CLO_VALUE].nConnect = &sModbMeasure[_SS_CLO].nConnect_u8;

    sRegSensor[_E_CLO_TEMP].State = &sModbMeasure[_SS_CLO].sUser;
    sRegSensor[_E_CLO_TEMP].vReturn = &sModbMeasure[_SS_CLO].Temp_f;
    sRegSensor[_E_CLO_TEMP].nConnect = &sModbMeasure[_SS_CLO].nConnect_u8;

    //----------------------------EC------------------------------
    sRegSensor[_E_EC_VALUE].State = &sModbMeasure[_SS_EC].sUser;
    sRegSensor[_E_EC_VALUE].vReturn = &sModbMeasure[_SS_EC].Value_f;
    sRegSensor[_E_EC_VALUE].nConnect = &sModbMeasure[_SS_EC].nConnect_u8;

    sRegSensor[_E_EC_TEMP].State = &sModbMeasure[_SS_EC].sUser;
    sRegSensor[_E_EC_TEMP].vReturn = &sModbMeasure[_SS_EC].Temp_f;
    sRegSensor[_E_EC_TEMP].nConnect = &sModbMeasure[_SS_EC].nConnect_u8;
    
    //---------------------------Turb-----------------------------
    sRegSensor[_E_TURB_VALUE].State = &sModbMeasure[_SS_TURB].sUser;
    sRegSensor[_E_TURB_VALUE].vReturn = &sModbMeasure[_SS_TURB].Value_f;
    sRegSensor[_E_TURB_VALUE].nConnect = &sModbMeasure[_SS_TURB].nConnect_u8;
    
    sRegSensor[_E_TURB_TEMP].State = &sModbMeasure[_SS_TURB].sUser;
    sRegSensor[_E_TURB_TEMP].vReturn = &sModbMeasure[_SS_TURB].Temp_f;
    sRegSensor[_E_TURB_TEMP].nConnect = &sModbMeasure[_SS_TURB].nConnect_u8;
    
    //----------------------------COD------------------------------
    sRegSensor[_E_COD_VALUE].State = &sModbMeasure[_SS_COD].sUser;
    sRegSensor[_E_COD_VALUE].vReturn = &sModbMeasure[_SS_COD].Value_f;
    sRegSensor[_E_COD_VALUE].nConnect = &sModbMeasure[_SS_COD].nConnect_u8;
    
    sRegSensor[_E_COD_S_SENSOR].State = &sModbMeasure[_SS_COD].sUser;
    sRegSensor[_E_COD_S_SENSOR].vReturn = &sModbMeasure[_SS_COD].stateSensor;
    sRegSensor[_E_COD_S_SENSOR].nConnect = &sModbMeasure[_SS_COD].nConnect_u8;
    
    sRegSensor[_E_COD_S_VALUE].State = &sModbMeasure[_SS_COD].sUser;
    sRegSensor[_E_COD_S_VALUE].vReturn = &sModbMeasure[_SS_COD].stateValue;
    sRegSensor[_E_COD_S_VALUE].nConnect = &sModbMeasure[_SS_COD].nConnect_u8;
    
    //----------------------------TSS-----------------------------
    sRegSensor[_E_TSS_VALUE].State = &sModbMeasure[_SS_TSS].sUser;
    sRegSensor[_E_TSS_VALUE].vReturn = &sModbMeasure[_SS_TSS].Value_f;
    sRegSensor[_E_TSS_VALUE].nConnect = &sModbMeasure[_SS_TSS].nConnect_u8;
    
    sRegSensor[_E_TSS_S_SENSOR].State = &sModbMeasure[_SS_TSS].sUser;
    sRegSensor[_E_TSS_S_SENSOR].vReturn = &sModbMeasure[_SS_TSS].stateSensor;
    sRegSensor[_E_TSS_S_SENSOR].nConnect = &sModbMeasure[_SS_TSS].nConnect_u8;
    
    sRegSensor[_E_TSS_S_VALUE].State = &sModbMeasure[_SS_TSS].sUser;
    sRegSensor[_E_TSS_S_VALUE].vReturn = &sModbMeasure[_SS_TSS].stateValue;
    sRegSensor[_E_TSS_S_VALUE].nConnect = &sModbMeasure[_SS_TSS].nConnect_u8;

    //----------------------------NH4-----------------------------
    sRegSensor[_E_NH4_VALUE].State = &sModbMeasure[_SS_NH4].sUser;
    sRegSensor[_E_NH4_VALUE].vReturn = &sModbMeasure[_SS_NH4].Value_f;
    sRegSensor[_E_NH4_VALUE].nConnect = &sModbMeasure[_SS_NH4].nConnect_u8;
    
    sRegSensor[_E_NH4_S_SENSOR].State = &sModbMeasure[_SS_NH4].sUser;
    sRegSensor[_E_NH4_S_SENSOR].vReturn = &sModbMeasure[_SS_NH4].stateSensor;
    sRegSensor[_E_NH4_S_SENSOR].nConnect = &sModbMeasure[_SS_NH4].nConnect_u8;
    
    sRegSensor[_E_NH4_S_VALUE].State = &sModbMeasure[_SS_NH4].sUser;
    sRegSensor[_E_NH4_S_VALUE].vReturn = &sModbMeasure[_SS_NH4].stateValue;
    sRegSensor[_E_NH4_S_VALUE].nConnect = &sModbMeasure[_SS_NH4].nConnect_u8;
    
    //----------------------------DO------------------------------
    sRegSensor[_E_DO_VALUE].State = &sModbMeasure[_SS_DO].sUser;
    sRegSensor[_E_DO_VALUE].vReturn = &sModbMeasure[_SS_DO].Value_f;
    sRegSensor[_E_DO_VALUE].nConnect = &sModbMeasure[_SS_DO].nConnect_u8;

    sRegSensor[_E_DO_S_SENSOR].State = &sModbMeasure[_SS_DO].sUser;
    sRegSensor[_E_DO_S_SENSOR].vReturn = &sModbMeasure[_SS_DO].stateSensor;
    sRegSensor[_E_DO_S_SENSOR].nConnect = &sModbMeasure[_SS_DO].nConnect_u8;
    
    sRegSensor[_E_DO_S_VALUE].State = &sModbMeasure[_SS_DO].sUser;
    sRegSensor[_E_DO_S_VALUE].vReturn = &sModbMeasure[_SS_DO].stateValue;
    sRegSensor[_E_DO_S_VALUE].nConnect = &sModbMeasure[_SS_DO].nConnect_u8;
    
    //---------------------------Salt-----------------------------
    sRegSensor[_E_SALT_VALUE].State = &sModbMeasure[_SS_SALT].sUser;
    sRegSensor[_E_SALT_VALUE].vReturn = &sModbMeasure[_SS_SALT].Value_f;
    sRegSensor[_E_SALT_VALUE].nConnect = &sModbMeasure[_SS_SALT].nConnect_u8;

    sRegSensor[_E_SALT_TEMP].State = &sModbMeasure[_SS_SALT].sUser;
    sRegSensor[_E_SALT_TEMP].vReturn = &sModbMeasure[_SS_SALT].Temp_f;
    sRegSensor[_E_SALT_TEMP].nConnect = &sModbMeasure[_SS_SALT].nConnect_u8;
    
    //---------------------------TDS------------------------------
    sRegSensor[_E_TDS_VALUE].State = &sModbMeasure[_SS_TDS].sUser;
    sRegSensor[_E_TDS_VALUE].vReturn = &sModbMeasure[_SS_TDS].Value_f;
    sRegSensor[_E_TDS_VALUE].nConnect = &sModbMeasure[_SS_TDS].nConnect_u8;

    sRegSensor[_E_TDS_TEMP].State   = &sModbMeasure[_SS_TDS].sUser;
    sRegSensor[_E_TDS_TEMP].vReturn = &sModbMeasure[_SS_TDS].Temp_f;
    sRegSensor[_E_TDS_TEMP].nConnect = &sModbMeasure[_SS_TDS].nConnect_u8;
    
    //---------------------------NO3------------------------------
    sRegSensor[_E_NO3_VALUE].State = &sModbMeasure[_SS_NO3].sUser;
    sRegSensor[_E_NO3_VALUE].vReturn = &sModbMeasure[_SS_NO3].Value_f;
    sRegSensor[_E_NO3_VALUE].nConnect = &sModbMeasure[_SS_NO3].nConnect_u8;

    sRegSensor[_E_NO3_TEMP].State   = &sModbMeasure[_SS_NO3].sUser;
    sRegSensor[_E_NO3_TEMP].vReturn = &sModbMeasure[_SS_NO3].Temp_f;
    sRegSensor[_E_NO3_TEMP].nConnect = &sModbMeasure[_SS_NO3].nConnect_u8;
    
    //---------------------------TEMP-----------------------------
    sRegSensor[_E_TEMP_VALUE].State = &sModbMeasure[_SS_TEMP].sUser;
    sRegSensor[_E_TEMP_VALUE].vReturn = &sModbMeasure[_SS_TEMP].Value_f;
    sRegSensor[_E_TEMP_VALUE].nConnect = &sModbMeasure[_SS_TEMP].nConnect_u8;
    
    sRegSensor[_E_TEMP_S_SENSOR].State = &sModbMeasure[_SS_TEMP].sUser;
    sRegSensor[_E_TEMP_S_SENSOR].vReturn = &sModbMeasure[_SS_TEMP].stateSensor;
    sRegSensor[_E_TEMP_S_SENSOR].nConnect = &sModbMeasure[_SS_TEMP].nConnect_u8;
    
    sRegSensor[_E_TEMP_S_VALUE].State = &sModbMeasure[_SS_TEMP].sUser;
    sRegSensor[_E_TEMP_S_VALUE].vReturn = &sModbMeasure[_SS_TEMP].stateValue;
    sRegSensor[_E_TEMP_S_VALUE].nConnect = &sModbMeasure[_SS_TEMP].nConnect_u8;
#endif
}

static uint8_t fevent_modb_entry(uint8_t event)
{
//    fevent_enable(sEventAppRs485, _EVENT_RS485_TRANSMIT);
    fevent_enable(sEventAppModb, _EVENT_MODB_REFRESH);
    return 1;
}

static uint8_t fevent_modb_wait_calib(uint8_t event)
{
    if(sHandleModb.State_Wait_Calib != _STATE_CALIB_DONE)
    {
        sHandleModb.State_Wait_Calib = _STATE_CALIB_ERROR;
    }
    return 1;
}

static uint8_t fevent_modb_refresh(uint8_t event)
{
    Init_UartRs485();
    Init_UartRs485_2();
    fevent_enable(sEventAppModb, event);
    return 1;
}

static uint8_t fevent_modb_handle_subreg(uint8_t event)
{
    //Handle SubReg pH
    if(sModbMeasure[_SS_PH].Value_f == 0)
      sModbSubReg.pH = 7;
    else if(sModbMeasure[_SS_PH].Value_f < 5)
      sModbSubReg.pH = 5;
    else if(sModbMeasure[_SS_PH].Value_f > 9)
      sModbSubReg.pH = 9;
    else
      sModbSubReg.pH = sModbMeasure[_SS_PH].Value_f;
    
    //Handle SubReg PSU
    sModbSubReg.Salt_PSU = sModbMeasure[_SS_SALT].Value_f * 10;
       
    fevent_enable(sEventAppModb, event);
    return 1;
}

uint8_t Modb_KindHandle = _E_PH_VALUE;
uint16_t Transaction = 0;
static uint8_t fevent_modb_transmit_data(uint8_t event)
{
    uint8_t aData[4] = {0};
    uint32_t hex_Data = 0;
    
    if(Modb_KindHandle == _E_MODB_SS_END)
      Modb_KindHandle = _E_PH_VALUE;
    
    while(*sRegSensor[Modb_KindHandle].State != 1)
    {
        if(sRegSensor[Modb_KindHandle].nConnect != NULL)
            *sRegSensor[Modb_KindHandle].nConnect = 0;
        
        Modb_KindHandle++;
        if(Modb_KindHandle == _E_MODB_SS_END) 
        {
            fevent_enable(sEventAppModb, event);
            return 1;
        }
    }
    
    sTransModTCP.Flag = 0;
    
#ifdef MODBUS_SENSOR_SAOVIET
    if (sRegSensor[Modb_KindHandle].nPort > _PORT_MODB_TCP)
    {
        if (sRegSensor[Modb_KindHandle].cmdRW == 1)
        {
            if(sRegSensor[Modb_KindHandle].vFormat == _ETYPE_F)
                hex_Data = Decode_Data_Type_f(*sRegSensor[Modb_KindHandle].subReg, sRegSensor[Modb_KindHandle].vFormat);
            else 
                hex_Data = (uint32_t)sRegSensor[Modb_KindHandle].subReg;

            hex_Data = Endian_Format(hex_Data, sRegSensor[Modb_KindHandle].cmdLen * 2, sRegSensor[Modb_KindHandle].vBeLe);

            for (uint8_t i = 0; i < sRegSensor[Modb_KindHandle].cmdLen * 2; i++)
                aData[i] = (uint8_t)(hex_Data >> (8 * ((sRegSensor[Modb_KindHandle].cmdLen * 2 - 1) - i)));

            if (sRegSensor[Modb_KindHandle].nPort == _PORT_RS485_1)
                RS485_Modbus_Write_Value(sRegSensor[Modb_KindHandle].idDev, sRegSensor[Modb_KindHandle].cmdAddr, sRegSensor[Modb_KindHandle].cmdLen, aData);
            else
                RS485_2_Modbus_Write_Value(sRegSensor[Modb_KindHandle].idDev, sRegSensor[Modb_KindHandle].cmdAddr, sRegSensor[Modb_KindHandle].cmdLen, aData);
        }
        else
        {
            if (sRegSensor[Modb_KindHandle].nPort == _PORT_RS485_1)
                RS485_Modbus_Read_Value(sRegSensor[Modb_KindHandle].idDev, sRegSensor[Modb_KindHandle].cmdAddr, sRegSensor[Modb_KindHandle].cmdLen);
            else
                RS485_2_Modbus_Read_Value(sRegSensor[Modb_KindHandle].idDev, sRegSensor[Modb_KindHandle].cmdAddr, sRegSensor[Modb_KindHandle].cmdLen);
        }
    }
    else
    {
        sTransModTCP.length = 0;
        sTransModTCP.aData[sTransModTCP.length++] = sRegSensor[Modb_KindHandle].idDev >> 8;
        sTransModTCP.aData[sTransModTCP.length++] = sRegSensor[Modb_KindHandle].idDev;
        sTransModTCP.aData[sTransModTCP.length++] = 0x00;
        sTransModTCP.aData[sTransModTCP.length++] = 0x00;
        sTransModTCP.aData[sTransModTCP.length++] = 0x00;
        sTransModTCP.aData[sTransModTCP.length++] = 0x06;
        sTransModTCP.aData[sTransModTCP.length++] = sRegSensor[Modb_KindHandle].idDev;
        sTransModTCP.aData[sTransModTCP.length++] = 0x04;
        sTransModTCP.aData[sTransModTCP.length++] = sRegSensor[Modb_KindHandle].cmdAddr >>8;
        sTransModTCP.aData[sTransModTCP.length++] = sRegSensor[Modb_KindHandle].cmdAddr;
        sTransModTCP.aData[sTransModTCP.length++] = sRegSensor[Modb_KindHandle].cmdLen >>8;
        sTransModTCP.aData[sTransModTCP.length++] = sRegSensor[Modb_KindHandle].cmdLen;
        sTransModTCP.Flag = 1;
        
        Reset_Buff(&sDataRecvTCP);
    }
    if(sRegSensor[Modb_KindHandle].nPort == _PORT_MODB_TCP)
        sEventAppModb[_EVENT_MODBUS_RECEIVE_DATA].e_period = TIMEOUT_MODBUS_TCP;
    else
        sEventAppModb[_EVENT_MODBUS_RECEIVE_DATA].e_period = TIMEOUT_MODBUS_RTU;
#else
        sTransModTCP.length = 0;
        sTransModTCP.aData[sTransModTCP.length++] = Transaction >> 8;
        sTransModTCP.aData[sTransModTCP.length++] = Transaction;
        sTransModTCP.aData[sTransModTCP.length++] = 0x00;
        sTransModTCP.aData[sTransModTCP.length++] = 0x00;
        sTransModTCP.aData[sTransModTCP.length++] = 0x00;
        sTransModTCP.aData[sTransModTCP.length++] = 0x06;
        sTransModTCP.aData[sTransModTCP.length++] = 0x01;
        sTransModTCP.aData[sTransModTCP.length++] = 0x04;
        sTransModTCP.aData[sTransModTCP.length++] = 0x00;
        sTransModTCP.aData[sTransModTCP.length++] = 0x00;
        sTransModTCP.aData[sTransModTCP.length++] = 0x00;
        sTransModTCP.aData[sTransModTCP.length++] = 0x28;
        sTransModTCP.Flag = 1;
        
        Reset_Buff(&sDataRecvTCP);
        sEventAppModb[_EVENT_MODBUS_RECEIVE_DATA].e_period = TIMEOUT_MODBUS_TCP;
#endif
    
    fevent_enable(sEventAppModb, _EVENT_MODBUS_RECEIVE_DATA);
    return 1;
}

uint32_t hex_Recv = 0;
static uint8_t fevent_modbus_receive_data(uint8_t event)
{
    sData *sDataReg = (sRegSensor[Modb_KindHandle].nPort == _PORT_RS485_1) ? &sUart485 : &sUart485_2;
    uint8_t RS485Status = (sRegSensor[Modb_KindHandle].nPort == _PORT_RS485_1) ? Rs485Status_u8 : Rs485_2Status_u8;
    sData ModContent;
    hex_Recv = 0;
    uint8_t Result_Recv = false;
    
#ifdef MODBUS_SENSOR_SAOVIET
    if (sRegSensor[Modb_KindHandle].nPort > _PORT_MODB_TCP)
    {
        if ((RS485Status == TRUE) && (RS485_Modbus_Check_Format(sRegSensor[Modb_KindHandle].idDev, sRegSensor[Modb_KindHandle].cmdLen, sDataReg, &ModContent) == true))
        {
          Result_Recv = true;
          fevent_enable(sEventAppModb, _EVENT_MODB_REFRESH);
        }
    }
    else
    {
        if (ModbusTCP_Check_Format(sRegSensor[Modb_KindHandle].idDev, sRegSensor[Modb_KindHandle].cmdLen, &sDataRecvTCP, &ModContent) == true)
            Result_Recv = true;
    }
    
    if(Result_Recv == true)
    {     
        if(ModContent.Length_u16 == 4)
        {
            hex_Recv =  ((uint32_t)ModContent.Data_a8[0] << 24) | 
                        ((uint32_t)ModContent.Data_a8[1] << 16) | 
                        ((uint32_t)ModContent.Data_a8[2] << 8)  | 
                        (uint32_t)ModContent.Data_a8[3];
        }
        else
            hex_Recv =  ((uint32_t)ModContent.Data_a8[0] << 8) | (uint32_t)ModContent.Data_a8[1];
        
        hex_Recv = Endian_Format(hex_Recv, ModContent.Length_u16, sRegSensor[Modb_KindHandle].vBeLe);
        
        if(sRegSensor[Modb_KindHandle].vReturn != NULL)
        {
            switch(sRegSensor[Modb_KindHandle].vFormat)
            {
                case _ETYPE_F:  
                    *(float*)sRegSensor[Modb_KindHandle].vReturn = Decode_Data_Type_u32(hex_Recv, sRegSensor[Modb_KindHandle].vFormat);
                    *(float*)sRegSensor[Modb_KindHandle].vReturn *= sRegSensor[Modb_KindHandle].vScale;
                    break;
                    
                case _ETYPE_U32:  
                    *(uint32_t*)sRegSensor[Modb_KindHandle].vReturn = hex_Recv;
                    *(uint32_t*)sRegSensor[Modb_KindHandle].vReturn *= (uint32_t)sRegSensor[Modb_KindHandle].vScale;
                    break;
                case _ETYPE_I32:  
                    *(int32_t*)sRegSensor[Modb_KindHandle].vReturn = hex_Recv;
                    *(int32_t*)sRegSensor[Modb_KindHandle].vReturn *= (uint32_t)sRegSensor[Modb_KindHandle].vScale;
                    break;
                case _ETYPE_U16:   
                    *(uint16_t*)sRegSensor[Modb_KindHandle].vReturn = hex_Recv;
                    *(uint16_t*)sRegSensor[Modb_KindHandle].vReturn *= (uint32_t)sRegSensor[Modb_KindHandle].vScale;
                    break;
                case _ETYPE_I16:  
                    *(int16_t*)sRegSensor[Modb_KindHandle].vReturn = hex_Recv;
                    *(int16_t*)sRegSensor[Modb_KindHandle].vReturn *= (uint32_t)sRegSensor[Modb_KindHandle].vScale;
                    break;
                case _ETYPE_U8:  
                    *(uint8_t*)sRegSensor[Modb_KindHandle].vReturn = hex_Recv;
                    *(uint8_t*)sRegSensor[Modb_KindHandle].vReturn *= (uint32_t)sRegSensor[Modb_KindHandle].vScale;
                    break;
                case _ETYPE_I8:    
                    *(int8_t*)sRegSensor[Modb_KindHandle].vReturn = hex_Recv;
                    *(int8_t*)sRegSensor[Modb_KindHandle].vReturn *= (uint32_t)sRegSensor[Modb_KindHandle].vScale;
                    break;
                  
                default:
                    break;
            }
        }
        
        if(*sRegSensor[Modb_KindHandle].nConnect < MAX_COUNT_DISCONNECT)
            *sRegSensor[Modb_KindHandle].nConnect +=1;
    }
    else
    {
        if(*sRegSensor[Modb_KindHandle].nConnect > 0)
            *sRegSensor[Modb_KindHandle].nConnect -=1;
    }
#else
    uint16_t Transaction_Recv = 0;
    uint16_t Pos = 0;

   
    if (ModbusTCP_Check_Format(0x01, 0x28, &sDataRecvTCP, &ModContent) == true)
    {
        Transaction_Recv = sDataRecvTCP.Data_a8[0] << 8 | sDataRecvTCP.Data_a8[1];
        if(Transaction_Recv == Transaction)
           Result_Recv = true;
    }

    if(Result_Recv == true)
    {
        // pH + Temp
        hex_Recv = Read_Register_Modbus(ModContent.Data_a8, &Pos, 2);
        sModbMeasure[_SS_PH].stateSensor = (uint8_t)Endian_Format(hex_Recv, 2, _E_BE);
        sModbMeasure[_SS_TEMP].stateSensor = (uint8_t)Endian_Format(hex_Recv, 2, _E_BE);

        hex_Recv = Read_Register_Modbus(ModContent.Data_a8, &Pos, 4);
        hex_Recv = Read_Register_Modbus(ModContent.Data_a8, &Pos, 2);
        sModbMeasure[_SS_PH].stateValue = 0x0F & (uint8_t)Endian_Format(hex_Recv, 2, _E_BE);
        sModbMeasure[_SS_TEMP].stateValue = 0x0F &(uint8_t)Endian_Format(hex_Recv, 2, _E_BS);
        
        hex_Recv = Read_Register_Modbus(ModContent.Data_a8, &Pos, 4);
        hex_Recv = Endian_Format(hex_Recv, 4, _E_BE);
        sModbMeasure[_SS_PH].Value_f = Decode_Data_Type_u32(hex_Recv, _ETYPE_F);
        
        hex_Recv = Read_Register_Modbus(ModContent.Data_a8, &Pos, 4);
        hex_Recv = Endian_Format(hex_Recv, 4, _E_BE);
        sModbMeasure[_SS_TEMP].Value_f = Decode_Data_Type_u32(hex_Recv, _ETYPE_F);
        
        // Do
        hex_Recv = Read_Register_Modbus(ModContent.Data_a8, &Pos, 2);
        sModbMeasure[_SS_DO].stateSensor = (uint8_t)Endian_Format(hex_Recv, 2, _E_BE);

        hex_Recv = Read_Register_Modbus(ModContent.Data_a8, &Pos, 4);
        hex_Recv = Read_Register_Modbus(ModContent.Data_a8, &Pos, 2);
        sModbMeasure[_SS_DO].stateValue = 0x0F & (uint8_t)Endian_Format(hex_Recv, 2, _E_BE);
        
        hex_Recv = Read_Register_Modbus(ModContent.Data_a8, &Pos, 4);
        hex_Recv = Endian_Format(hex_Recv, 4, _E_BE);
        sModbMeasure[_SS_DO].Value_f = Decode_Data_Type_u32(hex_Recv, _ETYPE_F);
        
        hex_Recv = Read_Register_Modbus(ModContent.Data_a8, &Pos, 4);
        // NH4
        hex_Recv = Read_Register_Modbus(ModContent.Data_a8, &Pos, 2);
        sModbMeasure[_SS_NH4].stateSensor = (uint8_t)Endian_Format(hex_Recv, 2, _E_BE);

        hex_Recv = Read_Register_Modbus(ModContent.Data_a8, &Pos, 4);
        hex_Recv = Read_Register_Modbus(ModContent.Data_a8, &Pos, 2);
        sModbMeasure[_SS_NH4].stateValue = 0x0F & (uint8_t)Endian_Format(hex_Recv, 2, _E_BE);
        
        hex_Recv = Read_Register_Modbus(ModContent.Data_a8, &Pos, 4);
        hex_Recv = Endian_Format(hex_Recv, 4, _E_BE);
        sModbMeasure[_SS_NH4].Value_f = Decode_Data_Type_u32(hex_Recv, _ETYPE_F);
        
        hex_Recv = Read_Register_Modbus(ModContent.Data_a8, &Pos, 4);
        // TSS
        hex_Recv = Read_Register_Modbus(ModContent.Data_a8, &Pos, 2);
        sModbMeasure[_SS_TSS].stateSensor = (uint8_t)Endian_Format(hex_Recv, 2, _E_BE);

        hex_Recv = Read_Register_Modbus(ModContent.Data_a8, &Pos, 4);
        hex_Recv = Read_Register_Modbus(ModContent.Data_a8, &Pos, 2);
        sModbMeasure[_SS_TSS].stateValue = 0x0F & (uint8_t)Endian_Format(hex_Recv, 2, _E_BE);
        
        hex_Recv = Read_Register_Modbus(ModContent.Data_a8, &Pos, 4);
        hex_Recv = Endian_Format(hex_Recv, 4, _E_BE);
        sModbMeasure[_SS_TSS].Value_f = Decode_Data_Type_u32(hex_Recv, _ETYPE_F) * 1000;
        
        hex_Recv = Read_Register_Modbus(ModContent.Data_a8, &Pos, 4);
        // COD
        hex_Recv = Read_Register_Modbus(ModContent.Data_a8, &Pos, 2);
        sModbMeasure[_SS_COD].stateSensor = (uint8_t)Endian_Format(hex_Recv, 2, _E_BE);

        hex_Recv = Read_Register_Modbus(ModContent.Data_a8, &Pos, 4);
        hex_Recv = Read_Register_Modbus(ModContent.Data_a8, &Pos, 2);
        sModbMeasure[_SS_COD].stateValue = 0x0F & (uint8_t)Endian_Format(hex_Recv, 2, _E_BE);
        
        hex_Recv = Read_Register_Modbus(ModContent.Data_a8, &Pos, 4);
        hex_Recv = Endian_Format(hex_Recv, 4, _E_BE);
        sModbMeasure[_SS_COD].Value_f = Decode_Data_Type_u32(hex_Recv, _ETYPE_F);
        
        sModbMeasure[_SS_PH].nConnect_u8 = MAX_COUNT_DISCONNECT;
        sModbMeasure[_SS_TEMP].nConnect_u8 = MAX_COUNT_DISCONNECT;
        sModbMeasure[_SS_DO].nConnect_u8 = MAX_COUNT_DISCONNECT;
        sModbMeasure[_SS_NH4].nConnect_u8 = MAX_COUNT_DISCONNECT;
        sModbMeasure[_SS_TSS].nConnect_u8 = MAX_COUNT_DISCONNECT;
        sModbMeasure[_SS_COD].nConnect_u8 = MAX_COUNT_DISCONNECT;
    }
    else
    {
        if(sModbMeasure[_SS_PH].nConnect_u8 > 0)
            sModbMeasure[_SS_PH].nConnect_u8 -= 1;
        if(sModbMeasure[_SS_TEMP].nConnect_u8 > 0)
            sModbMeasure[_SS_TEMP].nConnect_u8 -= 1;
        if(sModbMeasure[_SS_DO].nConnect_u8 > 0)
            sModbMeasure[_SS_DO].nConnect_u8 -= 1;
        if(sModbMeasure[_SS_NH4].nConnect_u8 > 0)
            sModbMeasure[_SS_NH4].nConnect_u8 -= 1;
        if(sModbMeasure[_SS_TSS].nConnect_u8 > 0)
            sModbMeasure[_SS_TSS].nConnect_u8 -= 1;
        if(sModbMeasure[_SS_COD].nConnect_u8 > 0)
            sModbMeasure[_SS_COD].nConnect_u8 -= 1;
    }
    
    Transaction++;
#endif
    Modb_KindHandle++;
    fevent_enable(sEventAppModb, _EVENT_MODB_TRANSMIT_DATA);
    return 1;
}

static uint8_t fevent_ptr_temp(uint8_t event)
{
    uint8_t result = false;
    for(uint8_t i = _SS_PH; i<_END_SENSOR; i++)
    {
        if(result == false)
        {
            if(sModbMeasure[i].sUser == _ACTIVE_SENSOR)
            {
                sRegSensor[_E_TEMP_VALUE].idDev = sModbMeasure[i].ID_Modbus;
                sRegSensor[_E_TEMP_S_SENSOR].idDev = sModbMeasure[i].ID_Modbus;
                sRegSensor[_E_TEMP_S_VALUE].idDev = sModbMeasure[i].ID_Modbus;
            }
        }
        else 
            break;
    }
    fevent_enable(sEventAppModb, event);
    return 1;
}
/*======================Modbus TCP Check====================*/
uint8_t ModbusTCP_Check_Format(uint8_t SlaveID, uint16_t nRegis,
                               sData *pSource, sData *Content)
{
    uint16_t Pos = 0;
    uint8_t ModFunc = 0, ModLength = 0, ModSlave = 0;
    uint16_t LengthField = 0;

    //MBAP + FUNC
    if (pSource->Length_u16 < 8)
        return false;

    // bo qua Transaction ID + Protocol ID
    Pos = 4;

    // lay Length field
    LengthField = (pSource->Data_a8[Pos] << 8) | pSource->Data_a8[Pos + 1];
    Pos += 2;

    // kiem tra do dai
    if (LengthField + 6 != pSource->Length_u16)
        return false;

    // Unit ID
    ModSlave = pSource->Data_a8[Pos++];

    // Function
    ModFunc = pSource->Data_a8[Pos++];

    switch (ModFunc)
    {
        case FUN_READ_REGIS:
            ModLength = pSource->Data_a8[Pos++];

            if ((ModSlave != SlaveID) || (ModLength != (nRegis * 2)))
                return false;

            // tro vao data
            Content->Data_a8 = &pSource->Data_a8[Pos];
            Content->Length_u16 = ModLength;
            break;

        default:
            return false;
    }

    return true;
}
/*==========================Handle==========================*/

///*-----------------------Handle Send RS485-----------------------*/
///*

/*
    @brief Send 485 sensor
*/
void        Send_RS458_Sensor(uint8_t *aData, uint16_t Length_u16) 
{
//    HAL_GPIO_WritePin(RS485_TXDE_S_GPIO_Port, RS485_TXDE_S_Pin, GPIO_PIN_SET);
//    HAL_Delay(5);
//    HAL_UART_Transmit(&uart_485, aData, Length_u16, 1000);
//    
//    UTIL_MEM_set(sUart485.Data_a8 , 0x00, sUart485.Length_u16);
//    sUart485.Length_u16 = 0;
//    CountBufferHandleRecv = 0;
//    
//    HAL_GPIO_WritePin(RS485_TXDE_S_GPIO_Port, RS485_TXDE_S_Pin, GPIO_PIN_RESET);
  

    HAL_GPIO_WritePin(DE_GPIO_PORT, DE_GPIO_PIN, GPIO_PIN_SET);
    HAL_Delay(10);
    // Send
    RS485_Init_Data();
    HAL_UART_Transmit(&uart_rs485, aData , Length_u16, 1000); 
    
    //Dua DE ve Receive
    HAL_GPIO_WritePin(DE_GPIO_PORT, DE_GPIO_PIN, GPIO_PIN_RESET);
}

void        Init_Parameter_Sensor(void)
{

}
/*=========================Handle Data=======================*/
uint32_t Read_Register_Rs485(uint8_t aData[], uint16_t *pos, uint8_t LengthData)
{
    uint32_t stamp = 0;
    uint16_t length = *pos;
    if(LengthData == 4)
    {
        stamp = aData[length+2]<<8 | aData[length+3];
        stamp = (stamp << 16) | (aData[length]<<8 | aData[length+1]);
    }
    else if(LengthData == 2)
    {
        stamp = aData[length]<<8 | aData[length+1];
    }
    *pos = *pos + LengthData;
    return stamp;
}

/*==================Handle Define AT command=================*/
#ifdef USING_AT_CONFIG

#endif

/*========================Handle Data========================*/
uint32_t Endian_Format(uint32_t Hex_Data, uint8_t length, uint8_t Type)
{
    uint8_t b1 = (Hex_Data >> 24) & 0xFF;
    uint8_t b2 = (Hex_Data >> 16) & 0xFF;
    uint8_t b3 = (Hex_Data >> 8)  & 0xFF;
    uint8_t b4 = (Hex_Data)       & 0xFF;

    uint32_t Result = Hex_Data;

    if(length == 4)
    {
        switch(Type)
        {
            case _E_BE: // ABCD
                Result = (b1<<24)|(b2<<16)|(b3<<8)|b4;
                break;

            case _E_LE: // DCBA
                Result = (b4<<24)|(b3<<16)|(b2<<8)|b1;
                break;

            case _E_BS: // BADC
                Result = (b2<<24)|(b1<<16)|(b4<<8)|b3;
                break;

            case _E_WS: // CDAB
                Result = (b3<<24)|(b4<<16)|(b1<<8)|b2;
                break;

            default:
                return 0;
        }
    }
    else if(length == 2)
    {
        switch(Type)
        {
            case _E_BE: // 56 78
                Result = (b1<<24)|(b2<<16)|(b3<<8)|b4;
                break;

            case _E_LE: // 56 78 -> 78 56
                Result = (b1<<24)|(b2<<16)|(b4<<8)|b3;
                break;

            case _E_BS: // 56 78 -> 65 87 
                Result = (b1<<24) | (b2<<16) |
                         ((((b3 & 0x0F) << 4) | ((b3 & 0xF0) >> 4)) << 8) |
                         (((b4 & 0x0F) << 4) | ((b4 & 0xF0) >> 4));
                break;

            case _E_WS: // 56 78 -> 87 65
                Result = (b1<<24) | (b2<<16) |
                         ((((b4 & 0x0F) << 4) | ((b4 & 0xF0) >> 4)) << 8) |
                         (((b3 & 0x0F) << 4) | ((b3 & 0xF0) >> 4));
                break;

            default:
                return 0;
        }
    }

    return Result;
}

float Decode_Data_Type_u32(uint32_t Hex_Data, uint8_t Type)
{
    float Result = 0;
    if(Type == _ETYPE_U32)
        Result = (float)((uint32_t)Hex_Data);
    else if(Type == _ETYPE_I32)
        Result = (float)((int32_t)Hex_Data);
    else if(Type == _ETYPE_U16)
        Result = (float)((uint16_t)Hex_Data);
    else if(Type == _ETYPE_U32)
        Result = (float)((int16_t)Hex_Data);
    else
        Convert_uint32Hex_To_Float(Hex_Data, &Result);
    
    return Result;
}

uint32_t Decode_Data_Type_f(float Data_f, uint8_t Type)
{
    uint32_t Result = 0;
    if(Type == _ETYPE_U32)
        Result = (uint32_t)Data_f;
    else if(Type == _ETYPE_I32)
        Result = (int32_t)Data_f;
    else if(Type == _ETYPE_U16)
        Result = (uint16_t)Data_f;
    else if(Type == _ETYPE_U32)
        Result = (int16_t)Data_f;
    else
        Result = Handle_Float_To_hexUint32(Data_f);
    
    return Result;
}

uint32_t Read_Register_Modbus(uint8_t aData[], uint16_t *pos, uint8_t LengthData)
{
    uint32_t stamp = 0;
    uint16_t length = *pos;
    if(LengthData == 4)
    {
        stamp =  ((uint32_t)aData[length] << 24) | 
                    ((uint32_t)aData[length + 1] << 16) | 
                    ((uint32_t)aData[length + 2] << 8)  | 
                    (uint32_t)aData[length + 3];
    }
    else if(LengthData == 2)
    {
        stamp =  ((uint32_t)aData[length] << 8) | (uint32_t)aData[length + 1];
    }
    else 
    {
        stamp = (uint32_t)aData[length];
    }
    *pos = *pos + LengthData;
    return stamp;
}
/*========================AT Command======================*/
void Modem_SER_Set_Mod_TCP_Main (sData *strRecei, uint16_t Pos)
{
    sServerInfor sServerTemp = {0};

    if (Modem_Extract_Server((char *) (strRecei->Data_a8 + Pos), &sServerTemp) == true) {
        UTIL_MEM_set(&sModemInfor.sServerModTCP, 0, sizeof(sModemInfor.sServerModTCP));
        UTIL_MEM_cpy(&sModemInfor.sServerModTCP, &sServerTemp, sizeof(sServerTemp));
        sModemInfor.sServerModTCP.KeepAlive_u32 = 60;

        //Convert lai IP       
        if (UTIL_Convert_IP_To_Buff(sModemInfor.sServerModTCP.aIP, sModemInfor.sServerModTCP.IPnum) == true) {
            sModemInfor.sServerModTCP.DomainOrIp_u8 = __SERVER_IP;
        } else {
            sModemInfor.sServerModTCP.DomainOrIp_u8 = __SERVER_DOMAIN;
        }
        sModemInfor.sServerModTCP.Port_u16 = (uint16_t) UtilStringToInt(sModemInfor.sServerModTCP.aPORT);
        
        Modem_Save_Var();
        Modem_Respond_Str(PortConfig, "OK", 0);
        return;
    }
        
    Modem_Respond_Str(PortConfig, "ERROR", 0);
}


void Modem_SER_Get_Mod_TCP_Main (sData *strRecei, uint16_t Pos)
{
    char aData[128] = {0};
    
    sprintf((char*) aData, "%s:%s\r\n", sModemInfor.sServerModTCP.aIP,
                                        sModemInfor.sServerModTCP.aPORT);  
    
    Modem_Respond_Str(PortConfig, aData, 0);
}
/*==================Handle Task and Init app=================*/
void Init_UartRs485(void)
{
    RS485_Stop_RX_Mode();
    WM_DIG_Init_Uart(&uart_rs485, sWmDigVar.sModbInfor[0].MType_u8);
    RS485_Init_RX_Mode();
}

void Init_UartRs485_2(void)
{
    RS485_2_Stop_RX_Mode();
    WM_DIG_Init_Uart(&uart_rs485_2, sWmDigVar.sModbInfor[0].MType_u8);
    RS485_2_Init_RX_Mode();
}

void       Init_AppModb(void)
{
    Init_UartRs485();
    Init_UartRs485_2();
    Init_Parameter_Sensor();
#ifdef USING_AT_CONFIG
    /* regis cb serial */
    sATCmdList[_GET_MOD_TCP_MAIN].CallBack = Modem_SER_Get_Mod_TCP_Main;
    sATCmdList[_SET_MOD_TCP_MAIN].CallBack = Modem_SER_Set_Mod_TCP_Main;
#endif
    RS485_Para_Init();
}

uint8_t        AppModb_Task(void)
{
    uint8_t i = 0;
    uint8_t Result =  false;
    
    for(i = 0; i < _EVENT_MODB_END; i++)
    {
        if(sEventAppModb[i].e_status == 1)
        {
            Result = true; 
            
            if((sEventAppModb[i].e_systick == 0) ||
               ((HAL_GetTick() - sEventAppModb[i].e_systick) >= sEventAppModb[i].e_period))
            {
                sEventAppModb[i].e_status = 0; //Disable event
                sEventAppModb[i].e_systick= HAL_GetTick();
                sEventAppModb[i].e_function_handler(i);
            }
        }
    }
    
    return Result;
}



