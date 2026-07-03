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
static uint8_t fevent_modb_rs485_1_refresh(uint8_t event);
static uint8_t fevent_modb_rs485_2_refresh(uint8_t event);

static uint8_t fevent_modb_handle_subreg(uint8_t event);

static uint8_t fevent_modb_rs485_1_handle(uint8_t event);
static uint8_t fevent_modb_rs485_2_handle(uint8_t event);
static uint8_t fevent_modb_tcp_handle(uint8_t event);

static uint8_t fevent_ptr_temp(uint8_t event);
/*==============================Struct=============================*/
sEvent_struct               sEventAppModb[]=
{
  {_EVENT_MODB_ENTRY,              1, 5, 30000,            fevent_modb_entry},            //Doi slave khoi dong moi truyen opera
  
  {_EVENT_MODB_WAIT_CALIB,         0, 5, 5000,             fevent_modb_wait_calib},
  {_EVENT_MODB_RS485_1_REFRESH,    0, 5, 60000,            fevent_modb_rs485_1_refresh},
  {_EVENT_MODB_RS485_2_REFRESH,    0, 5, 60000,            fevent_modb_rs485_2_refresh},
  
  {_EVENT_MODB_HANDLE_SUBREG,      1, 5, 500,              fevent_modb_handle_subreg},
  
  {_EVENT_MODB_RS485_1_HANDLE,     1, 5, 100,              fevent_modb_rs485_1_handle},
  {_EVENT_MODB_RS485_2_HANDLE,     1, 5, 100,              fevent_modb_rs485_2_handle},
  {_EVENT_MODB_TCP_HANDLE,         1, 5, 100,              fevent_modb_tcp_handle},
  
  {_EVENT_PTR_TEMP,                1, 5, 100,              fevent_ptr_temp},
};
uint16_t CountBufferHandleRecv = 0;

static uint8_t aDATA_RECV_MODBUS_TCP [DATA_BUF_SIZE];
sData   sDataRecvTCP = {(uint8_t *) &aDATA_RECV_MODBUS_TCP[0], 0};

extern sData sUart485;
extern sData sUart485_2;

Struct_TransModbusTCP       sTransModTCP = {0};

Struct_Modb_SubReg          sModbSubReg = {0};
Struct_Hanlde_Modb          sHandleModb = {0};

uint8_t Connect_Test = 0;

uint8_t ID_Default = 1;
uint8_t User_True = 1;
uint8_t User_False = 0;

Struct_RegSensor            sRegSensor[] =
{
#ifdef MODBUS_SENSOR_SAOVIET
   /*---------------------Kênh 1----------------------*/
    //eKind             //State  //cmdRW //idDev     //cmdLen  //Addr   //vFormat  //vBeLe  //vScale  //subReg  //nPort         //vReturn  //nConnect
  {_E_PH_VALUE,     1,  NULL,    0,      NULL,   2,        0x0002,  _ETYPE_F,  _E_WS,   1,        NULL,     _PORT_RS485_1,       NULL,      NULL},
  {_E_PH_S_SENSOR,  1,  NULL,    0,      NULL,   1,        0x000A,  _ETYPE_U8, _E_BE,   1,        NULL,     _PORT_RS485_1,       NULL,      NULL},
  {_E_PH_S_VALUE,   1,  NULL,    0,      NULL,   1,        0x000B,  _ETYPE_U8, _E_BE,   1,        NULL,     _PORT_RS485_1,       NULL,      NULL},
  
  {_E_CLO_SEND_PH,  2,  NULL,    1,      NULL,  2,        0x0006,  _ETYPE_F,  _E_WS,   1,        NULL,     _PORT_RS485_1,       NULL,      NULL},
  {_E_CLO_VALUE,    3,  NULL,    0,      NULL,  2,        0x0002,  _ETYPE_F,  _E_WS,   1,        NULL,     _PORT_RS485_1,       NULL,      NULL},
  {_E_CLO_S_SENSOR, 3,  NULL,    0,      NULL,  1,        0x000A,  _ETYPE_U8, _E_BE,   1,        NULL,     _PORT_RS485_1,       NULL,      NULL},
  {_E_CLO_S_VALUE,  3,  NULL,    0,      NULL,  1,        0x000B,  _ETYPE_U8, _E_BE,   1,        NULL,     _PORT_RS485_1,       NULL,      NULL},
  
  {_E_EC_VALUE,     4,  NULL,    0,      NULL,   2,        0x0002,  _ETYPE_F,  _E_WS,   1,        NULL,     _PORT_RS485_1,       NULL,      NULL},
  {_E_EC_S_SENSOR,  4,  NULL,    0,      NULL,   1,        0x000A,  _ETYPE_U8, _E_BE,   1,        NULL,     _PORT_RS485_1,       NULL,      NULL},
  {_E_EC_S_VALUE,   4,  NULL,    0,      NULL,   1,        0x000B,  _ETYPE_U8, _E_BE,   1,        NULL,     _PORT_RS485_1,       NULL,      NULL},
  
  {_E_TURB_VALUE,   5,  NULL,    0,      NULL, 2,        0x0002,  _ETYPE_F,  _E_WS,   1,        NULL,     _PORT_RS485_1,       NULL,      NULL},
  {_E_TURB_S_SENSOR,5,  NULL,    0,      NULL, 1,        0x000A,  _ETYPE_U8, _E_BE,   1,        NULL,     _PORT_RS485_1,       NULL,      NULL},
  {_E_TURB_S_VALUE, 5,  NULL,    0,      NULL, 1,        0x000B,  _ETYPE_U8, _E_BE,   1,        NULL,     _PORT_RS485_1,       NULL,      NULL},
  
  {_E_COD_VALUE,    6,  NULL,    0,      NULL,  2,        0x0002,  _ETYPE_F,  _E_WS,   1,        NULL,     _PORT_RS485_1,       NULL,      NULL},
  {_E_COD_S_SENSOR, 6,  NULL,    0,      NULL,  1,        0x000A,  _ETYPE_U8, _E_BE,   1,        NULL,     _PORT_RS485_1,       NULL,      NULL},
  {_E_COD_S_VALUE,  6,  NULL,    0,      NULL,  1,        0x000B,  _ETYPE_U8, _E_BE,   1,        NULL,     _PORT_RS485_1,       NULL,      NULL},
  
  {_E_TSS_VALUE,    7,  NULL,    0,      NULL,  2,        0x0002,  _ETYPE_F,  _E_WS,   1,        NULL,     _PORT_RS485_1,       NULL,      NULL},
  {_E_TSS_S_SENSOR, 7,  NULL,    0,      NULL,  1,        0x000A,  _ETYPE_U8, _E_BE,   1,        NULL,     _PORT_RS485_1,       NULL,      NULL},
  {_E_TSS_S_VALUE,  7,  NULL,    0,      NULL,  1,        0x000B,  _ETYPE_U8, _E_BE,   1,        NULL,     _PORT_RS485_1,       NULL,      NULL},
  
  {_E_NH4_VALUE,    8,  NULL,    0,      NULL,  2,        0x0002,  _ETYPE_F,  _E_WS,   1,        NULL,     _PORT_RS485_1,       NULL,      NULL},
  {_E_NH4_S_SENSOR, 8,  NULL,    0,      NULL,  1,        0x000A,  _ETYPE_U8, _E_BE,   1,        NULL,     _PORT_RS485_1,       NULL,      NULL},
  {_E_NH4_S_VALUE,  8,  NULL,    0,      NULL,  1,        0x000B,  _ETYPE_U8, _E_BE,   1,        NULL,     _PORT_RS485_1,       NULL,      NULL},
  
  {_E_DO_SALT,      9,   NULL,    1,      NULL,   2,        0x0008,  _ETYPE_F,  _E_WS,   1,        NULL,     _PORT_RS485_1,       NULL,      NULL},
  {_E_DO_VALUE,     10,  NULL,    0,      NULL,   2,        0x0002,  _ETYPE_F,  _E_WS,   1,        NULL,     _PORT_RS485_1,       NULL,      NULL},
  {_E_DO_S_SENSOR,  10,  NULL,    0,      NULL,   1,        0x000A,  _ETYPE_U8, _E_BE,   1,        NULL,     _PORT_RS485_1,       NULL,      NULL},
  {_E_DO_S_VALUE,   10,  NULL,    0,      NULL,   1,        0x000B,  _ETYPE_U8, _E_BE,   1,        NULL,     _PORT_RS485_1,       NULL,      NULL},
  
  {_E_SALT_VALUE,   11,  NULL,    0,      NULL,   2,        0x0008,  _ETYPE_F,  _E_WS,   1,        NULL,     _PORT_RS485_1,       NULL,      NULL},
  {_E_SALT_S_SENSOR,11,  NULL,    0,      NULL,   1,        0x000A,  _ETYPE_U8, _E_BE,   1,        NULL,     _PORT_RS485_1,       NULL,      NULL},
  {_E_SALT_S_VALUE, 11,  NULL,    0,      NULL,   1,        0x000B,  _ETYPE_U8, _E_BE,   1,        NULL,     _PORT_RS485_1,       NULL,      NULL},
  
  {_E_TDS_VALUE,    12,  NULL,    0,      NULL,   2,        0x0006,  _ETYPE_F,  _E_WS,   1,        NULL,     _PORT_RS485_1,       NULL,      NULL},
  {_E_TDS_S_SENSOR, 12,  NULL,    0,      NULL,   1,        0x000A,  _ETYPE_U8, _E_BE,   1,        NULL,     _PORT_RS485_1,       NULL,      NULL},
  {_E_TDS_S_VALUE,  12,  NULL,    0,      NULL,   1,        0x000B,  _ETYPE_U8, _E_BE,   1,        NULL,     _PORT_RS485_1,       NULL,      NULL},
  
  {_E_NO3_VALUE,    13,  NULL,    0,      NULL,   2,       0x0002,  _ETYPE_F,  _E_WS,   1,        NULL,     _PORT_RS485_1,       NULL,      NULL},
  {_E_NO3_S_SENSOR, 13,  NULL,    0,      NULL,   1,       0x000A,  _ETYPE_U8, _E_BE,   1,        NULL,     _PORT_RS485_1,       NULL,      NULL},
  {_E_NO3_S_VALUE,  13,  NULL,    0,      NULL,   1,       0x000B,  _ETYPE_U8, _E_BE,   1,        NULL,     _PORT_RS485_1,       NULL,      NULL},
  
  {_E_TEMP_VALUE,    14,    NULL,    0,     NULL,   2,      0x0004,  _ETYPE_F,  _E_WS,   1,        NULL,     _PORT_RS485_1,       NULL,      NULL},
  {_E_TEMP_S_SENSOR, 14,    NULL,    0,     NULL,   1,      0x000A,  _ETYPE_U8, _E_BE,   1,        NULL,     _PORT_RS485_1,       NULL,      NULL},
  {_E_TEMP_S_VALUE,  14,    NULL,    0,     NULL,   1,      0x000C,  _ETYPE_U8, _E_BE,   1,        NULL,     _PORT_RS485_1,       NULL,      NULL},
    
  /*-------------------Kênh 2------------------*/
  {_E_PH_VALUE_2,     1,  NULL,    0,      NULL,   2,        0x0002,  _ETYPE_F,  _E_WS,   1,        NULL,     _PORT_RS485_2,       NULL,      NULL},
  {_E_PH_S_SENSOR_2,  1,  NULL,    0,      NULL,   1,        0x000A,  _ETYPE_U8, _E_BE,   1,        NULL,     _PORT_RS485_2,       NULL,      NULL},
  {_E_PH_S_VALUE_2,   1,  NULL,    0,      NULL,   1,        0x000B,  _ETYPE_U8, _E_BE,   1,        NULL,     _PORT_RS485_2,       NULL,      NULL},
  
  {_E_TURB_VALUE_2,   5,  NULL,    0,      NULL, 2,        0x0002,  _ETYPE_F,  _E_WS,   1,        NULL,     _PORT_RS485_2,       NULL,      NULL},
  {_E_TURB_S_SENSOR_2,5,  NULL,    0,      NULL, 1,        0x000A,  _ETYPE_U8, _E_BE,   1,        NULL,     _PORT_RS485_2,       NULL,      NULL},
  {_E_TURB_S_VALUE_2, 5,  NULL,    0,      NULL, 1,        0x000B,  _ETYPE_U8, _E_BE,   1,        NULL,     _PORT_RS485_2,       NULL,      NULL},
 
  {_E_TEMP_VALUE_2,    14,    NULL,    0,     NULL,   2,      0x0004,  _ETYPE_F,  _E_WS,   1,        NULL,     _PORT_RS485_2,       NULL,      NULL},
  {_E_TEMP_S_SENSOR_2, 14,    NULL,    0,     NULL,   1,      0x000A,  _ETYPE_U8, _E_BE,   1,        NULL,     _PORT_RS485_2,       NULL,      NULL},
  {_E_TEMP_S_VALUE_2,  14,    NULL,    0,     NULL,   1,      0x000C,  _ETYPE_U8, _E_BE,   1,        NULL,     _PORT_RS485_2,       NULL,      NULL},
  /*-------------------Kênh Modbus TCP------------------*/
  {_E_TURB1_WRITE,   15,    NULL,       1,     NULL,     2,      0x0000,  _ETYPE_F,  _E_WS,   1,        NULL,     _PORT_ETH_TCP,       NULL,      NULL},
  {_E_PH1_WRITE,     15,    NULL,       1,     NULL,     2,      0x0002,  _ETYPE_F,  _E_WS,   1,        NULL,     _PORT_ETH_TCP,       NULL,      NULL},

  {_E_TURB2_WRITE,   15,    NULL,       1,     NULL,     2,      0x0006,  _ETYPE_F,  _E_WS,   1,        NULL,     _PORT_ETH_TCP,       NULL,      NULL},
  {_E_PH2_WRITE,     15,    NULL,       1,     NULL,     2,      0x0008,  _ETYPE_F,  _E_WS,   1,        NULL,     _PORT_ETH_TCP,       NULL,      NULL},
#else 
  {_E_PH_S_SENSOR,  1,  NULL,    0,      NULL,  1,        0,    _ETYPE_U8, _E_BE,   1,        NULL,     _PORT_ETH_TCP,       NULL,      NULL},
  {_E_TEMP_S_SENSOR,1,  NULL,    0,      NULL,  1,        0,    _ETYPE_U8, _E_BE,   1,        NULL,     _PORT_ETH_TCP,       NULL,      NULL},
  {_E_PH_S_VALUE,   1,  NULL,    0,      NULL,  1,        3,    _ETYPE_U8, _E_BE,   1,        NULL,     _PORT_ETH_TCP,       NULL,      NULL},
  {_E_TEMP_S_VALUE, 1,  NULL,    0,      NULL,  1,        3,    _ETYPE_U8, _E_BS,   1,        NULL,     _PORT_ETH_TCP,       NULL,      NULL},
  {_E_PH_VALUE,     1,  NULL,    0,      NULL,  2,        4,    _ETYPE_F,  _E_BE,   1,        NULL,     _PORT_ETH_TCP,       NULL,      NULL},
  {_E_TEMP_VALUE,   1,  NULL,    0,      NULL,  2,        6,    _ETYPE_F,  _E_BE,   1,        NULL,     _PORT_ETH_TCP,       NULL,      NULL},
  
  {_E_DO_S_SENSOR,  1,  NULL,    0,      NULL,   1,       8,    _ETYPE_U8, _E_BE,   1,        NULL,     _PORT_ETH_TCP,       NULL,      NULL},
  {_E_DO_S_VALUE,   1,  NULL,    0,      NULL,   1,       11,   _ETYPE_U8, _E_BE,   1,        NULL,     _PORT_ETH_TCP,       NULL,      NULL},
  {_E_DO_VALUE,     1,  NULL,    0,      NULL,   2,       12,   _ETYPE_F,  _E_BE,   1,        NULL,     _PORT_ETH_TCP,       NULL,      NULL},
  
  {_E_NH4_S_SENSOR, 1,  NULL,    0,      NULL,  1,        16,   _ETYPE_U8, _E_BE,   1,        NULL,     _PORT_ETH_TCP,       NULL,      NULL},
  {_E_NH4_S_VALUE,  1,  NULL,    0,      NULL,  1,        19,   _ETYPE_U8, _E_BE,   1,        NULL,     _PORT_ETH_TCP,       NULL,      NULL},
  {_E_NH4_VALUE,    1,  NULL,    0,      NULL,  2,        20,   _ETYPE_F,  _E_BE,   1,        NULL,     _PORT_ETH_TCP,       NULL,      NULL},
  
  {_E_TSS_S_SENSOR, 1,  NULL,    0,      NULL,  1,        24,   _ETYPE_U8, _E_BE,   1,        NULL,     _PORT_ETH_TCP,       NULL,      NULL},
  {_E_TSS_S_VALUE,  1,  NULL,    0,      NULL,  1,        27,   _ETYPE_U8, _E_BE,   1,        NULL,     _PORT_ETH_TCP,       NULL,      NULL},
  {_E_TSS_VALUE,    1,  NULL,    0,      NULL,  2,        28,   _ETYPE_F,  _E_BE,   1000,     NULL,     _PORT_ETH_TCP,       NULL,      NULL},
  
  {_E_COD_S_SENSOR, 1,  NULL,    0,      NULL,  1,        32,   _ETYPE_U8, _E_BE,   1,        NULL,     _PORT_ETH_TCP,       NULL,      NULL},
  {_E_COD_S_VALUE,  1,  NULL,    0,      NULL,  1,        35,   _ETYPE_U8, _E_BE,   1,        NULL,     _PORT_ETH_TCP,       NULL,      NULL},
  {_E_COD_VALUE,    1,  NULL,    0,      NULL,  2,        36,   _ETYPE_F,  _E_BE,   1,        NULL,     _PORT_ETH_TCP,       NULL,      NULL},
#endif
};
/*========================Function Handle========================*/


void       RS485_Para_Init(void)
{
#ifdef MODBUS_SENSOR_SAOVIET
    //PH
    Config_RegSen_Read(_E_PH_VALUE, &sMeasureMain[0][_SS_PH].ID_Modbus, &sMeasureMain[0][_SS_PH].sUser, 
                                    &sMeasureMain[0][_SS_PH].Value_f, &sMeasureMain[0][_SS_PH].nConnect_u8);
    
    Config_RegSen_Read(_E_PH_S_SENSOR, &sMeasureMain[0][_SS_PH].ID_Modbus, &sMeasureMain[0][_SS_PH].sUser, 
                                       &sMeasureMain[0][_SS_PH].stateSensor, &sMeasureMain[0][_SS_PH].nConnect_u8);

    Config_RegSen_Read(_E_PH_S_VALUE,  &sMeasureMain[0][_SS_PH].ID_Modbus, &sMeasureMain[0][_SS_PH].sUser, 
                                       &sMeasureMain[0][_SS_PH].stateValue, &sMeasureMain[0][_SS_PH].nConnect_u8);
    //CLO
    Config_RegSen_Write(_E_CLO_SEND_PH,  &sMeasureMain[0][_SS_CLO].ID_Modbus, &sMeasureMain[0][_SS_CLO].sUser, 
                                        &sModbSubReg.pH, &sMeasureMain[0][_SS_CLO].nConnect_u8);
    
    Config_RegSen_Read(_E_CLO_VALUE,  &sMeasureMain[0][_SS_CLO].ID_Modbus, &sMeasureMain[0][_SS_CLO].sUser, 
                                       &sMeasureMain[0][_SS_CLO].Value_f, &sMeasureMain[0][_SS_CLO].nConnect_u8);
    
    Config_RegSen_Read(_E_CLO_S_SENSOR,  &sMeasureMain[0][_SS_CLO].ID_Modbus, &sMeasureMain[0][_SS_CLO].sUser, 
                                       &sMeasureMain[0][_SS_CLO].stateSensor, &sMeasureMain[0][_SS_CLO].nConnect_u8);
    
    Config_RegSen_Read(_E_CLO_S_VALUE,  &sMeasureMain[0][_SS_CLO].ID_Modbus, &sMeasureMain[0][_SS_CLO].sUser, 
                                       &sMeasureMain[0][_SS_CLO].stateValue, &sMeasureMain[0][_SS_CLO].nConnect_u8);    
    
    //EC
    Config_RegSen_Read(_E_EC_VALUE,  &sMeasureMain[0][_SS_EC].ID_Modbus, &sMeasureMain[0][_SS_EC].sUser, 
                                       &sMeasureMain[0][_SS_EC].Value_f, &sMeasureMain[0][_SS_EC].nConnect_u8);    
    
    Config_RegSen_Read(_E_EC_S_SENSOR,  &sMeasureMain[0][_SS_EC].ID_Modbus, &sMeasureMain[0][_SS_EC].sUser, 
                                       &sMeasureMain[0][_SS_EC].stateSensor, &sMeasureMain[0][_SS_EC].nConnect_u8); 
    
    Config_RegSen_Read(_E_EC_S_VALUE,  &sMeasureMain[0][_SS_EC].ID_Modbus, &sMeasureMain[0][_SS_EC].sUser, 
                                       &sMeasureMain[0][_SS_EC].stateValue, &sMeasureMain[0][_SS_EC].nConnect_u8); 
    //TURB
    Config_RegSen_Read(_E_TURB_VALUE,  &sMeasureMain[0][_SS_TURB].ID_Modbus, &sMeasureMain[0][_SS_TURB].sUser, 
                                       &sMeasureMain[0][_SS_TURB].Value_f, &sMeasureMain[0][_SS_TURB].nConnect_u8);    
    
    Config_RegSen_Read(_E_TURB_S_SENSOR,  &sMeasureMain[0][_SS_TURB].ID_Modbus, &sMeasureMain[0][_SS_TURB].sUser, 
                                       &sMeasureMain[0][_SS_TURB].stateSensor, &sMeasureMain[0][_SS_TURB].nConnect_u8); 
    
    Config_RegSen_Read(_E_TURB_S_VALUE,  &sMeasureMain[0][_SS_TURB].ID_Modbus, &sMeasureMain[0][_SS_TURB].sUser, 
                                       &sMeasureMain[0][_SS_TURB].stateValue, &sMeasureMain[0][_SS_TURB].nConnect_u8); 
    //COD
    Config_RegSen_Read(_E_COD_VALUE,  &sMeasureMain[0][_SS_COD].ID_Modbus, &sMeasureMain[0][_SS_COD].sUser, 
                                       &sMeasureMain[0][_SS_COD].Value_f, &sMeasureMain[0][_SS_COD].nConnect_u8);    
    
    Config_RegSen_Read(_E_COD_S_SENSOR,  &sMeasureMain[0][_SS_COD].ID_Modbus, &sMeasureMain[0][_SS_COD].sUser, 
                                       &sMeasureMain[0][_SS_COD].stateSensor, &sMeasureMain[0][_SS_COD].nConnect_u8); 
    
    Config_RegSen_Read(_E_COD_S_VALUE,  &sMeasureMain[0][_SS_COD].ID_Modbus, &sMeasureMain[0][_SS_COD].sUser, 
                                       &sMeasureMain[0][_SS_COD].stateValue, &sMeasureMain[0][_SS_COD].nConnect_u8); 
    //TSS
    Config_RegSen_Read(_E_TSS_VALUE,  &sMeasureMain[0][_SS_TSS].ID_Modbus, &sMeasureMain[0][_SS_TSS].sUser, 
                                       &sMeasureMain[0][_SS_TSS].Value_f, &sMeasureMain[0][_SS_TSS].nConnect_u8);    
    
    Config_RegSen_Read(_E_TSS_S_SENSOR,  &sMeasureMain[0][_SS_TSS].ID_Modbus, &sMeasureMain[0][_SS_TSS].sUser, 
                                       &sMeasureMain[0][_SS_TSS].stateSensor, &sMeasureMain[0][_SS_TSS].nConnect_u8); 
    
    Config_RegSen_Read(_E_TSS_S_VALUE,  &sMeasureMain[0][_SS_TSS].ID_Modbus, &sMeasureMain[0][_SS_TSS].sUser, 
                                       &sMeasureMain[0][_SS_TSS].stateValue, &sMeasureMain[0][_SS_TSS].nConnect_u8); 
    //NH4
    Config_RegSen_Read(_E_NH4_VALUE,  &sMeasureMain[0][_SS_NH4].ID_Modbus, &sMeasureMain[0][_SS_NH4].sUser, 
                                       &sMeasureMain[0][_SS_NH4].Value_f, &sMeasureMain[0][_SS_NH4].nConnect_u8);    
    
    Config_RegSen_Read(_E_NH4_S_SENSOR,  &sMeasureMain[0][_SS_NH4].ID_Modbus, &sMeasureMain[0][_SS_NH4].sUser, 
                                       &sMeasureMain[0][_SS_NH4].stateSensor, &sMeasureMain[0][_SS_NH4].nConnect_u8); 
    
    Config_RegSen_Read(_E_NH4_S_VALUE,  &sMeasureMain[0][_SS_NH4].ID_Modbus, &sMeasureMain[0][_SS_NH4].sUser, 
                                       &sMeasureMain[0][_SS_NH4].stateValue, &sMeasureMain[0][_SS_NH4].nConnect_u8); 
    //DO
    Config_RegSen_Write(_E_DO_SALT,  &sMeasureMain[0][_SS_DO].ID_Modbus, &sMeasureMain[0][_SS_DO].sUser, 
                                        &sModbSubReg.Salt_PSU, &sMeasureMain[0][_SS_DO].nConnect_u8);
    
    Config_RegSen_Read(_E_DO_VALUE,  &sMeasureMain[0][_SS_DO].ID_Modbus, &sMeasureMain[0][_SS_DO].sUser, 
                                       &sMeasureMain[0][_SS_DO].Value_f, &sMeasureMain[0][_SS_DO].nConnect_u8);
    
    Config_RegSen_Read(_E_DO_S_SENSOR,  &sMeasureMain[0][_SS_DO].ID_Modbus, &sMeasureMain[0][_SS_DO].sUser, 
                                       &sMeasureMain[0][_SS_DO].stateSensor, &sMeasureMain[0][_SS_DO].nConnect_u8);
    
    Config_RegSen_Read(_E_DO_S_VALUE,  &sMeasureMain[0][_SS_DO].ID_Modbus, &sMeasureMain[0][_SS_DO].sUser, 
                                       &sMeasureMain[0][_SS_DO].stateValue, &sMeasureMain[0][_SS_DO].nConnect_u8);    
    
    //SALT
    Config_RegSen_Read(_E_SALT_VALUE,  &sMeasureMain[0][_SS_SALT].ID_Modbus, &sMeasureMain[0][_SS_SALT].sUser, 
                                       &sMeasureMain[0][_SS_SALT].Value_f, &sMeasureMain[0][_SS_SALT].nConnect_u8);    
    
    Config_RegSen_Read(_E_SALT_S_SENSOR,  &sMeasureMain[0][_SS_SALT].ID_Modbus, &sMeasureMain[0][_SS_SALT].sUser, 
                                       &sMeasureMain[0][_SS_SALT].stateSensor, &sMeasureMain[0][_SS_SALT].nConnect_u8); 
    
    Config_RegSen_Read(_E_SALT_S_VALUE,  &sMeasureMain[0][_SS_SALT].ID_Modbus, &sMeasureMain[0][_SS_SALT].sUser, 
                                       &sMeasureMain[0][_SS_SALT].stateValue, &sMeasureMain[0][_SS_SALT].nConnect_u8); 
    //TDS
    Config_RegSen_Read(_E_TDS_VALUE,  &sMeasureMain[0][_SS_TDS].ID_Modbus, &sMeasureMain[0][_SS_TDS].sUser, 
                                       &sMeasureMain[0][_SS_TDS].Value_f, &sMeasureMain[0][_SS_TDS].nConnect_u8);    
    
    Config_RegSen_Read(_E_TDS_S_SENSOR,  &sMeasureMain[0][_SS_TDS].ID_Modbus, &sMeasureMain[0][_SS_TDS].sUser, 
                                       &sMeasureMain[0][_SS_TDS].stateSensor, &sMeasureMain[0][_SS_TDS].nConnect_u8); 
    
    Config_RegSen_Read(_E_TDS_S_VALUE,  &sMeasureMain[0][_SS_TDS].ID_Modbus, &sMeasureMain[0][_SS_TDS].sUser, 
                                       &sMeasureMain[0][_SS_TDS].stateValue, &sMeasureMain[0][_SS_TDS].nConnect_u8); 
    //NO3
    Config_RegSen_Read(_E_NO3_VALUE,  &sMeasureMain[0][_SS_NO3].ID_Modbus, &sMeasureMain[0][_SS_NO3].sUser, 
                                       &sMeasureMain[0][_SS_NO3].Value_f, &sMeasureMain[0][_SS_NO3].nConnect_u8);    
    
    Config_RegSen_Read(_E_NO3_S_SENSOR,  &sMeasureMain[0][_SS_NO3].ID_Modbus, &sMeasureMain[0][_SS_NO3].sUser, 
                                       &sMeasureMain[0][_SS_NO3].stateSensor, &sMeasureMain[0][_SS_NO3].nConnect_u8); 
    
    Config_RegSen_Read(_E_NO3_S_VALUE,  &sMeasureMain[0][_SS_NO3].ID_Modbus, &sMeasureMain[0][_SS_NO3].sUser, 
                                       &sMeasureMain[0][_SS_NO3].stateValue, &sMeasureMain[0][_SS_NO3].nConnect_u8); 
    //TEMP
    Config_RegSen_Read(_E_TEMP_VALUE,  &sMeasureMain[0][_SS_TEMP].ID_Modbus, &sMeasureMain[0][_SS_TEMP].sUser, 
                                       &sMeasureMain[0][_SS_TEMP].Value_f, &sMeasureMain[0][_SS_TEMP].nConnect_u8);    
    
    Config_RegSen_Read(_E_TEMP_S_SENSOR,  &sMeasureMain[0][_SS_TEMP].ID_Modbus, &sMeasureMain[0][_SS_TEMP].sUser, 
                                       &sMeasureMain[0][_SS_TEMP].stateSensor, &sMeasureMain[0][_SS_TEMP].nConnect_u8); 
    
    Config_RegSen_Read(_E_TEMP_S_VALUE,  &sMeasureMain[0][_SS_TEMP].ID_Modbus, &sMeasureMain[0][_SS_TEMP].sUser, 
                                       &sMeasureMain[0][_SS_TEMP].stateValue, &sMeasureMain[0][_SS_TEMP].nConnect_u8); 
    
    //PH 2
    Config_RegSen_Read(_E_PH_VALUE_2, &sMeasureMain[1][_SS_PH].ID_Modbus, &sMeasureMain[1][_SS_PH].sUser, 
                                    &sMeasureMain[1][_SS_PH].Value_f, &sMeasureMain[1][_SS_PH].nConnect_u8);
    
    Config_RegSen_Read(_E_PH_S_SENSOR_2, &sMeasureMain[1][_SS_PH].ID_Modbus, &sMeasureMain[1][_SS_PH].sUser, 
                                       &sMeasureMain[1][_SS_PH].stateSensor, &sMeasureMain[1][_SS_PH].nConnect_u8);

    Config_RegSen_Read(_E_PH_S_VALUE_2,  &sMeasureMain[1][_SS_PH].ID_Modbus, &sMeasureMain[1][_SS_PH].sUser, 
                                       &sMeasureMain[1][_SS_PH].stateValue, &sMeasureMain[1][_SS_PH].nConnect_u8);
    
    //TURB 2
    Config_RegSen_Read(_E_TURB_VALUE_2,  &sMeasureMain[1][_SS_TURB].ID_Modbus, &sMeasureMain[1][_SS_TURB].sUser, 
                                       &sMeasureMain[1][_SS_TURB].Value_f, &sMeasureMain[1][_SS_TURB].nConnect_u8);    
    
    Config_RegSen_Read(_E_TURB_S_SENSOR_2,  &sMeasureMain[1][_SS_TURB].ID_Modbus, &sMeasureMain[1][_SS_TURB].sUser, 
                                       &sMeasureMain[1][_SS_TURB].stateSensor, &sMeasureMain[1][_SS_TURB].nConnect_u8); 
    
    Config_RegSen_Read(_E_TURB_S_VALUE_2,  &sMeasureMain[1][_SS_TURB].ID_Modbus, &sMeasureMain[1][_SS_TURB].sUser, 
                                       &sMeasureMain[1][_SS_TURB].stateValue, &sMeasureMain[1][_SS_TURB].nConnect_u8); 
    
    //TEMP 2
    Config_RegSen_Read(_E_TEMP_VALUE_2,  &sMeasureMain[1][_SS_TEMP].ID_Modbus, &sMeasureMain[1][_SS_TEMP].sUser, 
                                       &sMeasureMain[1][_SS_TEMP].Value_f, &sMeasureMain[1][_SS_TEMP].nConnect_u8);    
    
    Config_RegSen_Read(_E_TEMP_S_SENSOR_2,  &sMeasureMain[1][_SS_TEMP].ID_Modbus, &sMeasureMain[1][_SS_TEMP].sUser, 
                                       &sMeasureMain[1][_SS_TEMP].stateSensor, &sMeasureMain[1][_SS_TEMP].nConnect_u8); 
    
    Config_RegSen_Read(_E_TEMP_S_VALUE_2,  &sMeasureMain[1][_SS_TEMP].ID_Modbus, &sMeasureMain[1][_SS_TEMP].sUser, 
                                       &sMeasureMain[1][_SS_TEMP].stateValue, &sMeasureMain[1][_SS_TEMP].nConnect_u8); 
    
    //Modbus TCP
    Config_RegSen_Write(_E_TURB1_WRITE,  &ID_Default, &User_True, 
                                       &sMeasureMain[0][_SS_TURB].Value_f, &Connect_Test);    
    
    Config_RegSen_Write(_E_PH1_WRITE,  &ID_Default, &User_True, 
                                       &sMeasureMain[0][_SS_PH].Value_f, &Connect_Test);    

    Config_RegSen_Write(_E_TURB2_WRITE,  &ID_Default, &User_True, 
                                       &sMeasureMain[1][_SS_TURB].Value_f, &Connect_Test); 
    Config_RegSen_Write(_E_PH2_WRITE,  &ID_Default, &User_True, 
                                       &sMeasureMain[1][_SS_PH].Value_f, &Connect_Test); 
#else
    //----------------------------Ph------------------------------
    Config_RegSen_Read(_E_PH_VALUE, &sMeasureMain[0][_SS_PH].ID_Modbus, &sMeasureMain[0][_SS_PH].sUser, 
                                    &sMeasureMain[0][_SS_PH].Value_f, &sMeasureMain[0][_SS_PH].nConnect_u8);
    
    Config_RegSen_Read(_E_PH_S_SENSOR, &sMeasureMain[0][_SS_PH].ID_Modbus, &sMeasureMain[0][_SS_PH].sUser, 
                                       &sMeasureMain[0][_SS_PH].stateSensor, &sMeasureMain[0][_SS_PH].nConnect_u8);

    Config_RegSen_Read(_E_PH_S_VALUE,  &sMeasureMain[0][_SS_PH].ID_Modbus, &sMeasureMain[0][_SS_PH].sUser, 
                                       &sMeasureMain[0][_SS_PH].stateValue, &sMeasureMain[0][_SS_PH].nConnect_u8);
    //----------------------------COD------------------------------
    Config_RegSen_Read(_E_COD_VALUE,  &sMeasureMain[0][_SS_COD].ID_Modbus, &sMeasureMain[0][_SS_COD].sUser, 
                                       &sMeasureMain[0][_SS_COD].Value_f, &sMeasureMain[0][_SS_COD].nConnect_u8);    
    
    Config_RegSen_Read(_E_COD_S_SENSOR,  &sMeasureMain[0][_SS_COD].ID_Modbus, &sMeasureMain[0][_SS_COD].sUser, 
                                       &sMeasureMain[0][_SS_COD].stateSensor, &sMeasureMain[0][_SS_COD].nConnect_u8); 
    
    Config_RegSen_Read(_E_COD_S_VALUE,  &sMeasureMain[0][_SS_COD].ID_Modbus, &sMeasureMain[0][_SS_COD].sUser, 
                                       &sMeasureMain[0][_SS_COD].stateValue, &sMeasureMain[0][_SS_COD].nConnect_u8); 
    //----------------------------TSS-----------------------------
    Config_RegSen_Read(_E_TSS_VALUE,  &sMeasureMain[0][_SS_TSS].ID_Modbus, &sMeasureMain[0][_SS_TSS].sUser, 
                                       &sMeasureMain[0][_SS_TSS].Value_f, &sMeasureMain[0][_SS_TSS].nConnect_u8);    
    
    Config_RegSen_Read(_E_TSS_S_SENSOR,  &sMeasureMain[0][_SS_TSS].ID_Modbus, &sMeasureMain[0][_SS_TSS].sUser, 
                                       &sMeasureMain[0][_SS_TSS].stateSensor, &sMeasureMain[0][_SS_TSS].nConnect_u8); 
    
    Config_RegSen_Read(_E_TSS_S_VALUE,  &sMeasureMain[0][_SS_TSS].ID_Modbus, &sMeasureMain[0][_SS_TSS].sUser, 
                                       &sMeasureMain[0][_SS_TSS].stateValue, &sMeasureMain[0][_SS_TSS].nConnect_u8); 
    //----------------------------NH4-----------------------------
    Config_RegSen_Read(_E_NH4_VALUE,  &sMeasureMain[0][_SS_NH4].ID_Modbus, &sMeasureMain[0][_SS_NH4].sUser, 
                                       &sMeasureMain[0][_SS_NH4].Value_f, &sMeasureMain[0][_SS_NH4].nConnect_u8);    
    
    Config_RegSen_Read(_E_NH4_S_SENSOR,  &sMeasureMain[0][_SS_NH4].ID_Modbus, &sMeasureMain[0][_SS_NH4].sUser, 
                                       &sMeasureMain[0][_SS_NH4].stateSensor, &sMeasureMain[0][_SS_NH4].nConnect_u8); 
    
    Config_RegSen_Read(_E_NH4_S_VALUE,  &sMeasureMain[0][_SS_NH4].ID_Modbus, &sMeasureMain[0][_SS_NH4].sUser, 
                                       &sMeasureMain[0][_SS_NH4].stateValue, &sMeasureMain[0][_SS_NH4].nConnect_u8); 
    //----------------------------DO------------------------------
    Config_RegSen_Read(_E_DO_VALUE,  &sMeasureMain[0][_SS_DO].ID_Modbus, &sMeasureMain[0][_SS_DO].sUser, 
                                       &sMeasureMain[0][_SS_DO].Value_f, &sMeasureMain[0][_SS_DO].nConnect_u8);
    
    Config_RegSen_Read(_E_DO_S_SENSOR,  &sMeasureMain[0][_SS_DO].ID_Modbus, &sMeasureMain[0][_SS_DO].sUser, 
                                       &sMeasureMain[0][_SS_DO].stateSensor, &sMeasureMain[0][_SS_DO].nConnect_u8);
    
    Config_RegSen_Read(_E_DO_S_VALUE,  &sMeasureMain[0][_SS_DO].ID_Modbus, &sMeasureMain[0][_SS_DO].sUser, 
                                       &sMeasureMain[0][_SS_DO].stateValue, &sMeasureMain[0][_SS_DO].nConnect_u8);   
    //---------------------------TEMP-----------------------------
    Config_RegSen_Read(_E_TEMP_VALUE,  &sMeasureMain[0][_SS_TEMP].ID_Modbus, &sMeasureMain[0][_SS_TEMP].sUser, 
                                       &sMeasureMain[0][_SS_TEMP].Value_f, &sMeasureMain[0][_SS_TEMP].nConnect_u8);    
    
    Config_RegSen_Read(_E_TEMP_S_SENSOR,  &sMeasureMain[0][_SS_TEMP].ID_Modbus, &sMeasureMain[0][_SS_TEMP].sUser, 
                                       &sMeasureMain[0][_SS_TEMP].stateSensor, &sMeasureMain[0][_SS_TEMP].nConnect_u8); 
    
    Config_RegSen_Read(_E_TEMP_S_VALUE,  &sMeasureMain[0][_SS_TEMP].ID_Modbus, &sMeasureMain[0][_SS_TEMP].sUser, 
                                       &sMeasureMain[0][_SS_TEMP].stateValue, &sMeasureMain[0][_SS_TEMP].nConnect_u8); 
#endif
}

static uint8_t fevent_modb_entry(uint8_t event)
{
//    fevent_enable(sEventAppRs485, _EVENT_RS485_TRANSMIT);
    fevent_enable(sEventAppModb, _EVENT_MODB_RS485_1_REFRESH);
    fevent_enable(sEventAppModb, _EVENT_MODB_RS485_2_REFRESH);
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
static uint8_t fevent_modb_rs485_1_refresh(uint8_t event)
{
    Init_UartRs485();
    fevent_enable(sEventAppModb, event);
    return 1;
}

static uint8_t fevent_modb_rs485_2_refresh(uint8_t event)
{
    Init_UartRs485_2();
    fevent_enable(sEventAppModb, event);
    return 1;
}

static uint8_t fevent_modb_handle_subreg(uint8_t event)
{
    //Handle SubReg pH
    if(sMeasureMain[0][_SS_PH].Value_f == 0)
      sModbSubReg.pH = 7;
    else if(sMeasureMain[0][_SS_PH].Value_f < 5)
      sModbSubReg.pH = 5;
    else if(sMeasureMain[0][_SS_PH].Value_f > 9)
      sModbSubReg.pH = 9;
    else
      sModbSubReg.pH = sMeasureMain[0][_SS_PH].Value_f;
    
    //Handle SubReg PSU
    sModbSubReg.Salt_PSU = sMeasureMain[0][_SS_SALT].Value_f * 10;
       
    fevent_enable(sEventAppModb, event);
    return 1;
}

static uint8_t fevent_modb_rs485_1_handle(uint8_t event)
{
    static uint8_t step = 0;
    static Struct_CtrlModbM     sCtrlModbM = {0};
    uint8_t aFrame[48] = {0};
    sData   strFrame = {(uint8_t *) &aFrame[0], 0};
    
    switch(step)
    {
        case 0:
            if(Handle_Trans_Modb(_PORT_RS485_1, sRegSensor, &sCtrlModbM, &strFrame) == 1)
            {
                RS485_1_Trans(strFrame.Data_a8, strFrame.Length_u16);
                sEventAppModb[_EVENT_MODB_RS485_1_HANDLE].e_period = TIMEOUT_MODB_RTU;
                step = 1;
            }
            break;
          
        case 1:
            if(Handle_Recv_Modb(sRegSensor, &sCtrlModbM, sUart485) == 1)
                fevent_enable(sEventAppModb, _EVENT_MODB_RS485_1_REFRESH);
            
            sEventAppModb[_EVENT_MODB_RS485_1_HANDLE].e_period = 100;
            step = 0;
            break;
          
        default:
            break;
    }

    fevent_enable(sEventAppModb, event);
    return 1;
}

static uint8_t fevent_modb_rs485_2_handle(uint8_t event)
{
    static uint8_t step = 0;
    static Struct_CtrlModbM     sCtrlModbM = {0};
    uint8_t aFrame[48] = {0};
    sData   strFrame = {(uint8_t *) &aFrame[0], 0};
    
    switch(step)
    {
        case 0:
            if(Handle_Trans_Modb(_PORT_RS485_2, sRegSensor, &sCtrlModbM, &strFrame) == 1)
            {
                RS485_2_Trans(strFrame.Data_a8, strFrame.Length_u16);
                sEventAppModb[_EVENT_MODB_RS485_2_HANDLE].e_period = TIMEOUT_MODB_RTU;
                step = 1;
            }
            break;
          
        case 1:
            if(Handle_Recv_Modb(sRegSensor, &sCtrlModbM, sUart485_2) == 1)
                fevent_enable(sEventAppModb, _EVENT_MODB_RS485_2_REFRESH);
            
            sEventAppModb[_EVENT_MODB_RS485_2_HANDLE].e_period = 100;
            step = 0;
            break;
          
        default:
            break;
    }

    fevent_enable(sEventAppModb, event);
    return 1;
}

static uint8_t fevent_modb_tcp_handle(uint8_t event)
{
    static uint8_t step = 0;
    static Struct_CtrlModbM     sCtrlModbM = {0};
    uint8_t aFrame[48] = {0};
    sData   strFrame = {(uint8_t *) &aFrame[0], 0};
    
    switch(step)
    {
        case 0:
            if(Handle_Trans_Modb(_PORT_ETH_TCP, sRegSensor, &sCtrlModbM, &strFrame) == 1)
            {
                memset(sTransModTCP.aData, 0xAA, sizeof(sTransModTCP.aData));
                sTransModTCP.length = 0;
                
                sTransModTCP.aData[sTransModTCP.length++] = sCtrlModbM.Transaction_TCP >> 8;
                sTransModTCP.aData[sTransModTCP.length++] = sCtrlModbM.Transaction_TCP;
                sTransModTCP.aData[sTransModTCP.length++] = 0x00 >> 8;
                sTransModTCP.aData[sTransModTCP.length++] = 0x00;
                sTransModTCP.aData[sTransModTCP.length++] = (strFrame.Length_u16 - 2) >> 8;
                sTransModTCP.aData[sTransModTCP.length++] = (strFrame.Length_u16 - 2);
                
                for(uint8_t i = 0; i < strFrame.Length_u16 - 2; i++)
                    sTransModTCP.aData[sTransModTCP.length++] = strFrame.Data_a8[i];

                Reset_Buff(&sDataRecvTCP);
                sTransModTCP.Flag = TRUE;
                
                sEventAppModb[_EVENT_MODB_TCP_HANDLE].e_period = TIMEOUT_MODB_TCP;
                step = 1;
            }
            break;
          
        case 1:
            Handle_Recv_Modb(sRegSensor, &sCtrlModbM, sDataRecvTCP);
            
            sEventAppModb[_EVENT_MODB_TCP_HANDLE].e_period = 100;
            step = 0;
            break;
          
        default:
            break;
    }

    fevent_enable(sEventAppModb, event);
    return 1;
}

static uint8_t fevent_ptr_temp(uint8_t event)
{
    for(uint8_t i = _SS_PH; i<_END_SENSOR; i++)
    {
        if(sMeasureMain[0][i].sUser == _ACTIVE_SENSOR)
        {
            sRegSensor[_E_TEMP_VALUE].idDev = &sMeasureMain[0][i].ID_Modbus;
            sRegSensor[_E_TEMP_S_SENSOR].idDev = &sMeasureMain[0][i].ID_Modbus;
            sRegSensor[_E_TEMP_S_VALUE].idDev = &sMeasureMain[0][i].ID_Modbus;
            break;
        }
    }
    
    for(uint8_t i = _SS_PH; i<_END_SENSOR; i++)
    {
        if(sMeasureMain[1][i].sUser == _ACTIVE_SENSOR)
        {
            sRegSensor[_E_TEMP_VALUE_2].idDev = &sMeasureMain[1][i].ID_Modbus;
            sRegSensor[_E_TEMP_S_SENSOR_2].idDev = &sMeasureMain[1][i].ID_Modbus;
            sRegSensor[_E_TEMP_S_VALUE_2].idDev = &sMeasureMain[1][i].ID_Modbus;
            break;
        }
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
    
    if(ModSlave != SlaveID)
        return false;

    switch (ModFunc)
    {
        case FUN_READ_BYTE:
        case FUN_READ_REGIS:
            ModLength = pSource->Data_a8[Pos++];

            if (ModLength != (nRegis * 2))
                return false;

            // tro vao data
            Content->Data_a8 = &pSource->Data_a8[Pos];
            Content->Length_u16 = ModLength;
            break;

        case FUN_WRITE_BYTE:
        case FUN_WRITE_MULTI:
            break;
      
        default:
            return false;
            break;
    }

    return true;
}

uint8_t Modbus_RTU_Check_Format (uint8_t SlaveID, uint16_t nRegis,
                                   sData *pSource, sData *Content)
{
    uint16_t CrcCalcu = 0;
    uint8_t aCRC_GET[2] = {0};
    uint16_t Pos = 0;
    uint8_t ModFunc = 0, ModLength = 0, ModSlave = 0;
    
    if (pSource->Length_u16 < 4)
        return false;
    
    CrcCalcu = ModRTU_CRC(pSource->Data_a8, pSource->Length_u16 - 2);
            
    aCRC_GET[0] = (uint8_t) (CrcCalcu & 0x00FF);
    aCRC_GET[1] = (uint8_t) ( (CrcCalcu >> 8) & 0x00FF );
    
    if ( (aCRC_GET[0] != *(pSource->Data_a8 + pSource->Length_u16 - 2)) 
        || (aCRC_GET[1] != *(pSource->Data_a8 + pSource->Length_u16 - 1)) )
        return false;
            
    ModSlave = *(pSource->Data_a8 + Pos++);
    ModFunc = *(pSource->Data_a8 + Pos++);
    
    if(ModSlave != SlaveID)
        return false;
            
    switch (ModFunc)
    {
        case FUN_READ_BYTE:
        case FUN_READ_REGIS:
            ModLength = *(pSource->Data_a8 + Pos++);
            //check frame
            if (ModLength != (nRegis * 2))
                return false;
            
            //tro content vao data
            Content->Data_a8 = pSource->Data_a8 + Pos;
            Content->Length_u16 = ModLength;   
            break;
        case FUN_WRITE_BYTE:
        case FUN_WRITE_MULTI:
            break;
      
        default:
            return false;
            break;
    }
    
    return true;
}

/*==========================Handle Tran and Recv=========================*/
uint8_t Handle_Trans_Modb(uint8_t Port, Struct_RegSensor  sReg[], Struct_CtrlModbM  *sCtrl, sData *sFrame)
{
    uint32_t hex_Data = 0;
    if (!sReg || !sCtrl || !sFrame) return 0;
    
    if(sCtrl->iHandle == _E_MODB_SS_END)
      sCtrl->iHandle = 0;
    
    while(1)
    {
        if(*sRegSensor[sCtrl->iHandle].State == 1 && sRegSensor[sCtrl->iHandle].nPort == Port)
            break;
          
        if(sRegSensor[sCtrl->iHandle].nConnect != NULL && *sRegSensor[sCtrl->iHandle].State != 1)
            *sRegSensor[sCtrl->iHandle].nConnect = 0;
        
        if(++sCtrl->iHandle == _E_MODB_SS_END) 
        {
            return 0;
        }
    }
    
    sCtrl->iStartBlock = sCtrl->iHandle;
    sCtrl->iEndBlock   = sCtrl->iHandle;
    sCtrl->iReg        = sReg[sCtrl->iHandle].cmdLen;
    
    if (sReg[sCtrl->iStartBlock].cmdRW == 1)
    {
        uint8_t aData[50] = {0};
        uint8_t Length = 0;
        uint8_t i = sCtrl->iHandle;
        while(1)
        {
            if(sReg[i].vFormat == _ETYPE_F)
                hex_Data = Decode_Data_Type_f_to_u32(*(float*)sReg[i].subReg, sReg[i].vFormat);
            else 
                hex_Data = (uint32_t)sReg[i].subReg;

            hex_Data = Endian_Format(hex_Data, sReg[i].cmdLen * 2, sReg[i].vBeLe);

            for (uint8_t j = 0; j < sReg[i].cmdLen * 2; j++)
                aData[Length++] = (uint8_t)(hex_Data >> (8 * ((sReg[i].cmdLen * 2 - 1) - j)));
            
            i++;
            if((i < _E_MODB_SS_END) &&
               (sReg[i].Block == sReg[i-1].Block) && 
               (sReg[i].nPort == sReg[i-1].nPort) &&
               (sReg[i].cmdAddr == sReg[i-1].cmdAddr + sReg[i-1].cmdLen))
            {
                sCtrl->iEndBlock = i;
                sCtrl->iReg = sReg[i].cmdAddr - sReg[sCtrl->iStartBlock].cmdAddr + sReg[i].cmdLen;
            }
            else
              break;

        }

        uint8_t fun_code = (sCtrl->iReg == 1) ? FUN_WRITE_BYTE : FUN_WRITE_MULTI;
        ModRTU_Master_Write_Frame(sFrame, *sReg[sCtrl->iStartBlock].idDev, fun_code, sReg[sCtrl->iStartBlock].cmdAddr, sCtrl->iReg, aData);
    }
    else
    {
        uint8_t i = sCtrl->iHandle;
        while ((i + 1) < _E_MODB_SS_END)
        {
            if (sReg[i + 1].Block == sReg[i].Block && 
                sReg[i + 1].nPort == sReg[i].nPort && 
                sReg[i + 1].cmdAddr >= sReg[i].cmdAddr && 
                sReg[i + 1].idDev == sReg[i].idDev)
            {
                sCtrl->iEndBlock = i + 1;
                sCtrl->iReg = sReg[i + 1].cmdAddr - sReg[sCtrl->iStartBlock].cmdAddr + sReg[i + 1].cmdLen;
                i++; 
            }
            else 
              break;
        }
        ModRTU_Master_Read_Frame(sFrame, *sReg[sCtrl->iStartBlock].idDev, 0x03, sReg[sCtrl->iStartBlock].cmdAddr, sCtrl->iReg);
    }
    return 1;
}

uint8_t Handle_Recv_Modb(Struct_RegSensor  sReg[], Struct_CtrlModbM  *sCtrl, sData sRecv)
{
    uint8_t Result_Recv = false;
    uint32_t hex_Recv = 0;
    sData ModContent;
    
    if (!sReg || !sCtrl || !sReg[sCtrl->iHandle].idDev) return 0;

    if (sReg[sCtrl->iHandle].nPort > _PORT_ETH_TCP)
    {
        Result_Recv = Modbus_RTU_Check_Format(*sReg[sCtrl->iStartBlock].idDev, sCtrl->iReg, &sRecv, &ModContent);
    }
    else
    {
        Result_Recv = ModbusTCP_Check_Format(*sReg[sCtrl->iHandle].idDev, sCtrl->iReg, &sRecv, &ModContent);
        
        sCtrl->Transaction_TCP++;
    }
    
    while(sCtrl->iHandle <= sCtrl->iEndBlock)
    {
        if(Result_Recv)
        {     
            sCtrl->iAddr = sReg[sCtrl->iHandle].cmdAddr - sReg[sCtrl->iStartBlock].cmdAddr;
            if(sReg[sCtrl->iHandle].cmdRW == 0)
            {
                if(sReg[sCtrl->iHandle].cmdLen == 2)
                {
                    hex_Recv =  ((uint32_t)ModContent.Data_a8[sCtrl->iAddr*2 + 0] << 24) | 
                                ((uint32_t)ModContent.Data_a8[sCtrl->iAddr*2 + 1] << 16) | 
                                ((uint32_t)ModContent.Data_a8[sCtrl->iAddr*2 + 2] << 8)  | 
                                (uint32_t)ModContent.Data_a8[sCtrl->iAddr*2 + 3];
                }
                else
                    hex_Recv =  ((uint32_t)ModContent.Data_a8[sCtrl->iAddr*2 + 0] << 8) | 
                                (uint32_t)ModContent.Data_a8[sCtrl->iAddr*2 + 1];
                
                hex_Recv = Endian_Format(hex_Recv, sReg[sCtrl->iHandle].cmdLen*2, sReg[sCtrl->iHandle].vBeLe);
            }
            
            if(sReg[sCtrl->iHandle].vReturn != NULL)
            {
                switch(sReg[sCtrl->iHandle].vFormat)
                {
                    case _ETYPE_F:  
                        *(float*)sReg[sCtrl->iHandle].vReturn = Decode_Data_Type_u32_to_f(hex_Recv, sReg[sCtrl->iHandle].vFormat);
                        *(float*)sReg[sCtrl->iHandle].vReturn *= sReg[sCtrl->iHandle].vScale;
                        break;
                        
                    case _ETYPE_U32:  
                        *(uint32_t*)sReg[sCtrl->iHandle].vReturn = hex_Recv;
                        *(uint32_t*)sReg[sCtrl->iHandle].vReturn *= (uint32_t)sReg[sCtrl->iHandle].vScale;
                        break;
                    case _ETYPE_I32:  
                        *(int32_t*)sReg[sCtrl->iHandle].vReturn = hex_Recv;
                        *(int32_t*)sReg[sCtrl->iHandle].vReturn *= (uint32_t)sReg[sCtrl->iHandle].vScale;
                        break;
                    case _ETYPE_U16:   
                        *(uint16_t*)sReg[sCtrl->iHandle].vReturn = hex_Recv;
                        *(uint16_t*)sReg[sCtrl->iHandle].vReturn *= (uint32_t)sReg[sCtrl->iHandle].vScale;
                        break;
                    case _ETYPE_I16:  
                        *(int16_t*)sReg[sCtrl->iHandle].vReturn = hex_Recv;
                        *(int16_t*)sReg[sCtrl->iHandle].vReturn *= (uint32_t)sReg[sCtrl->iHandle].vScale;
                        break;
                    case _ETYPE_U8:  
                        *(uint8_t*)sReg[sCtrl->iHandle].vReturn = hex_Recv;
                        *(uint8_t*)sReg[sCtrl->iHandle].vReturn *= (uint32_t)sReg[sCtrl->iHandle].vScale;
                        break;
                    case _ETYPE_I8:    
                        *(int8_t*)sReg[sCtrl->iHandle].vReturn = hex_Recv;
                        *(int8_t*)sReg[sCtrl->iHandle].vReturn *= (uint32_t)sReg[sCtrl->iHandle].vScale;
                        break;
                      
                    default:
                        break;
                }
            }
            
            if(*sReg[sCtrl->iHandle].nConnect < MAX_COUNT_DISCONNECT)
                *sReg[sCtrl->iHandle].nConnect +=1;
        }
        else
        {
            if(*sReg[sCtrl->iHandle].nConnect > 0)
                *sReg[sCtrl->iHandle].nConnect -=1;
        }
        sCtrl->iHandle++;
    }
    
    if(Result_Recv == true)
        return 1;
    else 
        return 0;
}

void Config_RegSen_Read(uint8_t Kind,uint8_t *ID, uint8_t *User, void *Return, uint8_t *nConnect)
{
    sRegSensor[Kind].idDev = ID;
    sRegSensor[Kind].State = User;
    sRegSensor[Kind].vReturn = Return;
    sRegSensor[Kind].nConnect = nConnect;
}

void Config_RegSen_Write(uint8_t Kind,uint8_t *ID, uint8_t *User, void *subReg, uint8_t *nConnect)
{
    sRegSensor[Kind].idDev = ID;
    sRegSensor[Kind].State = User;
    sRegSensor[Kind].subReg = subReg ;
    sRegSensor[Kind].nConnect = nConnect;
}

/*==========================Handle==========================*/
void        Init_Parameter_Sensor(void)
{

}

/*==================Handle Define AT command=================*/
#ifdef USING_AT_CONFIG

#endif

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
    
    sprintf((char*) aData, "%s,%s,%s,%s\r\n", sModemInfor.sServerModTCP.aIP,
                                        sModemInfor.sServerModTCP.aPORT,
                                        sModemInfor.sServerModTCP.aUSER,
                                        sModemInfor.sServerModTCP.aPASS);  
    
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



