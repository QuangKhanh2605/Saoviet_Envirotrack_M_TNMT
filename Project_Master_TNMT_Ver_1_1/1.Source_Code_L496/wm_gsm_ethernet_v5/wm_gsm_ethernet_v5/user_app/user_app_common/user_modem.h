
/*
    8/2021
    Thu vien quan ly modem
*/

#ifndef USER_MODEM_H
#define USER_MODEM_H

#include "user_util.h"

#include "user_define.h"
#include "user_lpm.h"
#include "user_message.h"
#include "user_at_serial.h"

/*===================Define=========================*/
#define MAX_LENGTH_DCU_ID	    20
#define MAX_LENGTH_ALARM        512
#define MAX_LENGTH_IP		    40
#define MAX_LENGTH_PORT		    10
#define MAX_LENGTH_USER_PASS	40

#define MAX_LENGTH_FILE_PATH	200
#define MAX_LENGTH_FILE_NAME    80
#define MAX_LENGTH_FILE_URL     200

#define MAX_LENGTH_AT_CMD       64
#define MAX_LENGTH_AT_RESP      256

#define MAX_TIME_DISCONNECT     43200000

/*===================Struct, Var=========================*/
/*
 * 		KIND DCU
 */
typedef enum
{
	_MODEM_WM_NBIOT,
	_MODEM_WM_GSM,
	_MODEM_COS_PHI,
	_MODEM_EM_NBIOT,
    _MODEM_LEVEL,
    _MODEM_WM_GSM_LORA,
    _MODEM_TEMH_GSM,
    _MODEM_CONV_GSM,
    _CONTROL_OXY,
    _MODEM_WM_GSM_ETH,
}sTypeModem;


typedef enum
{
    __SERVER_IP,
    __SERVER_DOMAIN,
}eKindServer;


typedef enum
{
    __HARD_ONLINE_ONLY,
    __HARD_ONLINE_PSM,
}eKindHardWare;

typedef struct
{
    uint8_t         nTSVH_u8;               //so ban tin tsvh se gui
    uint32_t        Duration_u32;           //Chu ki thuc day log tsvh
    
    uint32_t        DurAlarm_u32;           //Chu ki thuc day check alarm
    
    uint32_t		DurOnline_u32;          //Tan suat gui ban tin: che do ONLINE
	uint16_t		UnitMin_u16;            //Gia tri dong goi vao ban tin: min
}sFrequenceSend;


typedef struct
{
	char    aIP[MAX_LENGTH_IP];
	char    aPORT[MAX_LENGTH_PORT];
	char  	aUSER[MAX_LENGTH_USER_PASS];
	char  	aPASS[MAX_LENGTH_USER_PASS];
    
    uint8_t     IPnum[6];
    uint16_t    Port_u16;
    
    uint32_t	KeepAlive_u32;
    uint8_t     DomainOrIp_u8;
}sServerInfor;


typedef struct   
{
    sServerInfor sServer;
    
    char    aPATH[MAX_LENGTH_FILE_PATH];
    char    aFILE_NAME[MAX_LENGTH_FILE_NAME];
    char    aFILE_URL[MAX_LENGTH_FILE_URL];
}sUpdateInfor;



typedef struct
{
    uint8_t         Type_u8;                    //Loai Modem
    char  			aID[MAX_LENGTH_DCU_ID];     //ID
    sFrequenceSend  sFrequence;                 //tan suat thu thap du lieu
        
    sServerInfor    sServerMain;                //thong tin server                  //so lan reset
    sServerInfor    sServerBack;                
    uint16_t        cReset_u16;                 //so lan reset
    sUpdateInfor    sServerTn;
}sModemInformation;


typedef struct
{
	uint8_t			rSaveBox_u8;		    //Flag danh dau vao che do Save box
    
    uint8_t	        cTSVH_u8;               //so lan log tsvh moi send di
    uint8_t	        cAlarm_u8;              //so lan check alarm de log tsvh
    
    uint8_t         CountConfig_u8;
    
    ST_TIME_FORMAT  sRTCSet;
    sUpdateInfor    sFwUpdate;
    uint8_t         DetHardware_u8;  
    
    char            aALARM[MAX_LENGTH_ALARM];
    char            aAT_CMD[MAX_LENGTH_AT_CMD];
    char            aAT_RESP[MAX_LENGTH_AT_RESP];
    
    uint32_t        DuraOpera_u32;
    uint32_t        LandMarkOpera_u32;
}sModemVariable;



typedef struct 
{
	uint8_t     Name_u8;
	char	    *pTopic;					
} sKindModem;



/*===================External Var, Struct=========================*/
extern sModemVariable	    sModemVar;
extern sModemInformation    sModemInfor;
extern StructpFuncLPM       sModemLPMHandle;
extern const sKindModem     sSV_Modem[];
/*================Extern var struct=====================*/


/*===================Function=========================*/
/*------ Cac function xu ly cac task----------------*/
void    Modem_Init(void);
void 	Modem_Init_Var(void);
void 	Modem_Save_Var(void);

uint8_t Modem_Reset_MCU (void);
uint8_t Modem_Reset_MCU_Immediately (void);

void    Modem_Packet_Alarm_String (const char *str);
//Func Modem App
void    Modem_Respond(uint8_t portNo, uint8_t *data, uint16_t length, uint8_t ack);
void    Modem_Respond_Str(uint8_t portNo, const char *str, uint8_t ack);

void    Modem_Deinit_Peripheral (void);
void    Modem_Init_Peripheral (void);
void    Modem_Init_Before_IRQ_Handle (void);
void    Modem_Deinit_Before_IRQ_Handle (void);

void    MX_GPIO_DeInit(void);
void    Modem_Uart_DeInit(void);
void    Modem_Init_Gpio_Again(void);

/*Func Cb Setting serial*/
#ifdef USING_AT_CONFIG
void    Modem_SER_Set_ID (sData *str, uint16_t Pos);
void    Modem_SER_Get_ID (sData *str, uint16_t Pos);

void    Modem_SER_Set_Duty_Cycle (sData *strRecei, uint16_t Pos);
void    Modem_SER_Get_Duty_Cycle (sData *strRecei, uint16_t Pos);
void    Modem_SER_Set_Dura_Online (sData *strRecei, uint16_t Pos);
void    Modem_SER_Get_Dura_Online (sData *strRecei, uint16_t Pos);

void    Modem_SER_Set_Numwakeup (sData *strRecei, uint16_t Pos);
void    Modem_SER_Set_sTime (sData *strRecei, uint16_t Pos);
void    Modem_SER_Get_sTime (sData *strRecei, uint16_t Pos);
void    Modem_SER_Get_Firm_Version (sData *strRecei, uint16_t Pos);
void    Modem_SER_Req_Reset (sData *strRecei, uint16_t Pos);

uint8_t Modem_Extract_Server (char *src, sServerInfor *pServer);

void    Modem_SER_Set_Server_Main (sData *strRecei, uint16_t Pos);
void    Modem_SER_Get_Server_Main (sData *strRecei, uint16_t Pos);

void    Modem_SER_Set_Server_Back (sData *strRecei, uint16_t Pos);
void    Modem_SER_Get_Server_Back (sData *strRecei, uint16_t Pos);

void    Modem_SER_Change_Server (sData *strRecei, uint16_t Pos);

uint8_t Modem_Extract_URL (sUpdateInfor *pUpdate, char *src);

void    Modem_SER_HTTP_Update (sData *strRecei, uint16_t Pos);
void    Modem_SER_FTP_Update (sData *strRecei, uint16_t Pos);

void    Modem_SER_Get_Level_Debug (sData *str_Receiv, uint16_t Pos);
void    Modem_SER_Set_Level_Debug (sData *str_Receiv, uint16_t Pos);

void    Modem_SER_Set_FTP_Main (sData *strRecei, uint16_t Pos);
void    Modem_SER_Get_FTP_Main (sData *strRecei, uint16_t Pos);

#endif

uint8_t Modem_Check_AT(sData *str, uint8_t Type);
uint8_t	Modem_Split_Update_Infor (sData *str_Receiv, uint16_t Pos);

void    Modem_Packet_MePDV (sData *pData);
void    Modem_Packet_Mess_Status (sData *pData);
void    Modem_Packet_Mess_Opera (sData *pData);
void    Modem_Packet_Mess_Intan (sData *pData);
void    Modem_Packet_Mess_Alarm (sData *pData);
void    Modem_Packet_Mess_RespAT (sData *pData);
void    Modem_Packet_Mess_SimID (sData *pData);

void    Modem_SER_Config (sData *strRecv, uint16_t pos);
uint8_t Modem_SER_Setting (sData *strRecv, uint16_t pos);
uint8_t Modem_SER_Setting_2 (sData *strRecv, uint16_t Pos);

void    Modem_SER_Set_Time (sData *strRecv, uint16_t Pos);
void    Modem_SER_Req_AT (sData *strRecv, uint16_t pos);
void    Modem_SER_Req_RF (sData *strRecv, uint16_t pos);
void    Modem_SER_Req_Update (sData *strRecv, uint16_t pos);

void    Modem_Set_Server_Infor_To_App(void);
void    Modem_Monitor_Connect_Server (void);


uint8_t Modem_Set_sTime (ST_TIME_FORMAT *sClock, uint8_t GMT);
char*   Modem_Get_Firm_Version (void);
char*   Modem_Get_ID (void);
uint8_t Modem_Change_Server (void);

void    Modem_SER_Set_cReset (sData *strRecei, uint16_t Pos);
void    Modem_SER_Get_cReset (sData *strRecei, uint16_t Pos);

void    Modem_PowOn_Mem (void);
void    Modem_Resp_From_Mem(uint8_t tdata, uint8_t *pdata, uint16_t length);

void    Modem_SER_Set_Time_Alarm (sData *strRecei, uint16_t Pos);
void    Modem_SER_Get_Time_Alarm (sData *strRecei, uint16_t Pos);


#endif /*  */






