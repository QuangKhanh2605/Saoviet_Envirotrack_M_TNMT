


#ifndef USER_WM_DIGITAL_H
#define USER_WM_DIGITAL_H


#include "user_util.h"

#include "woteck_wm.h"
#include "level_S_YW_01B.h"
#include "level_SUP_PX261_B.h"
#include "wm_mt100.h"
#include "level_ultrasonic.h"
#include "wm_sence.h"
#include "woteck_ultrasonic.h"
#include "wm_mong_cai.h"
#include "siemen_mag8000.h"

#include "user_rs485.h"


#define MAX_SLAVE_MODBUS        5

/*=========== Var struct =========*/
typedef enum
{
    __MET_WOTECK,           //0
    __MET_LEVEL_LIQ,        //1
    __MET_LEVEL_LIQ_SUP,    //2
    __MET_MT100,            //3
    __MET_LEVEL_ULTRA,      //4
    __MET_WM_SENCE,         //5
    __MET_WOTECK_ULTRA,     //6
    __MET_WM_MONG_CAI,      //7
    __MET_SI_MAG8000,       //8
    __MET_UNKNOWN,
}eRS485MeterType;


typedef struct 
{
    uint32_t BaudRate;
    uint32_t WordLength; 
	uint32_t StopBits;
    uint32_t Parity;
} sUartInterfaceInfor;

typedef struct
{
    uint8_t     Status_u8;
    uint8_t     Type_u8;
    
    int64_t     nTotal_i64;
    int64_t     nForw_i64;
    int64_t     nRev_i64;
    int32_t	    Flow_i32;               //luu luong   
    int32_t	    FlowM3_i32;  
    
    char        *sFlowUnit;
    char        *sTotalUnit;
    
    int32_t	    Pressure_i32;           //Ap suat
    
    uint8_t     Factor;
    
    uint16_t    LUnit_u16;
    uint16_t    LDecimal_u16;
    int16_t     LVal_i16;
    int16_t     LZeroPoint_i16;
    uint16_t    Lwire_u16;
    uint16_t    Lstatic_u16;    
    uint16_t    Ldynamic_u16;   
    int16_t     Ldelta_i16; 
    
    uint32_t    LandMark_u32;       //moc thoi gian mat ket noi
    uint32_t    inReg;              //thu tu thanh ghi dang doc
    
    sLevelParameter     sLevel01;
    sLevelSupParameter  sLevelSup;
    sLevelUltrasonic    sLevelUltra;
    sMT100Data          sMt100;
    sSENCEData          sSence;
    sWoteckUltraData    sWoteckUltra;
    sWoteckData         sWoteck01;
    sMongCaiData        sMongcai;
    sMag8000Data        sMag8000;
    
    uint8_t PinPercent_u8;
}sModbusDeviceData;



typedef void (*pFunc_Init ) (uint8_t Type);
typedef uint8_t (*pFunc_Decode )(uint8_t ireg, sData *pdata, void *staget);

typedef struct
{
    uint8_t             Type_u8;
    uint16_t            Addr_u32;
    uint8_t             MaxRegis_u8;
    sUartInterfaceInfor sUart;
	pFunc_Decode        fDecode;
}sWMDigitalInfor;

typedef struct 
{
    uint8_t             MType_u8;
    uint8_t             SlaveId_u8;
}sModbusInfor;


typedef struct 
{
    sModbusInfor        sModbInfor[MAX_SLAVE_MODBUS]; 
    sModbusDeviceData   sModbDevData[MAX_SLAVE_MODBUS];
    uint8_t             nModbus_u8;
}sWMDigitalVariable;


extern sWMDigitalVariable       sWmDigVar;
extern sWMDigitalInfor          sListWmDigital[10];
/*============= Function =============*/

void    WM_DIG_Init_Uart (UART_HandleTypeDef *huart, uint8_t type);
void    WM_DIG_Get_Data (uint8_t chann, uint8_t type);
void    WM_DIG_Packet_Mess (sData *pData, uint8_t chann);
uint16_t WM_DIG_cm_To_cUnit (uint8_t chann);

uint8_t WM_DIG_Get_Infor (uint8_t chann,  uint8_t type, uint16_t *addr, uint8_t *nReg);


#endif