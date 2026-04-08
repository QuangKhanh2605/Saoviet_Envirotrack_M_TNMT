/*
    8/2021
    Thu vien xu ly Uart
*/
#ifndef USER_MESSAGE_H
#define USER_MESSAGE_H

#include "user_modem.h"
#include "user_util.h"

/*======================== Define ======================*/
#define USING_APP_MESS


#define PACKET_MQTT_PROTOCOL

#ifdef PACKET_MQTT_PROTOCOL
#define MY_PINGREQ             0xC0
#define MY_PINGRESP            0xD0

#define SUB_QOS                0x00
#define MY_QOS                 0x01

#define MY_ID1 					0x0A
#define MY_ID2 					0x00

#define SUB_HEADER_LENGTH 		50
#define PUB_HEADER_LENGTH 		50 
#define MAX_LENGTH_MQTT  		1200
#endif

#define TOTAL_SLOT				40

#define	TSVH_OPERA              0
#define	TSVH_FLASH              1
#define	TSVH_MULTI              2

#define	TSVH_OPERA_OR_FLASH		TSVH_OPERA 

/*======================== Structs var======================*/
typedef enum {

    //Mess wait ACK
	TOPIC_NOTIF,            //0

    DATA_HANDSHAKE,         //1
    SEND_SERVER_TIME_PENDING,   //2
    
    DATA_TSVH_MULTI,        //3
	DATA_TSVH_FLASH,        //2
    DATA_TSVH_OPERA,        //3
    DATA_INTAN,             //4
    
    DATA_REC_HISTORY,       //5

    DATA_EVENT,             //6
    DATA_GPS,               //7

	SEND_SIM_ID,            //8
    SEND_RESPOND_SERVER_ACK, // 9
    SEND_SAVE_BOX_FAIL,     //10
    SEND_SAVE_BOX_OK,       //11
    
    SEND_UPDATE_FIRM_OK,    //12
    SEND_UPDATE_FIRM_FAIL,  //13
    SEND_SERVER_FTP_FAIL,       
    //Mess no wait ACK      
    SEND_RESPOND_SERVER,    //14
    SEND_RESPOND_FROM_RF,   //15    
	SEND_SHUTTING_DOWN,     //16
    SEND_RECONNECT_SERVER,  //17    
    SEND_HARD_RS_MCU,       //18
    SEND_ALARM,             //19
    DATA_PING,              //20
    DATA_PING_2,            //21
    DATA_STATUS,            //22
    
	SEND_SERVER_TIME_OK,        //24
	SEND_SERVER_TIME_FAIL,      //25
    SEND_EMPTY_MESS,            //26

	END_MQTT_SEND,
} eKindMessSend;

typedef enum {
	REQUEST_RESET,
    INTAN_DATA,
	REQUEST_SETTING,
    SER_SETTING_PARAM,
    SER_SETTING_PARAM_1,
    SER_SETTING_PARAM_2,
    SER_SETTING_PARAM_3,
    SER_SETTING_PARAM_4,
    SER_SETTING_PARAM_5,
	UPDATE_TIME_SERVER,
    RESET_DATA,
    REQUEST_AT,
    REQUEST_RF,
    
    REQUEST_UPDATE_FIRM,
    REQUEST_STATUS,
    
	END_MQTT_RECEI,
} eKindMessRecv;
//

typedef uint8_t (*pFunc_Send)(int Kind_Send);
typedef struct {
	int 		idMark;
	pFunc_Send 	CallBack;
	sData		sKind;
}sMessageSend;


//
typedef void (*pFunc_Recv)(sData *str_Receiv, int16_t Pos);
typedef struct {
	int 		idMark;
	pFunc_Recv	CallBack;
	sData	    sKind;
} sMessageRecv;


typedef struct {
	int		aMESS_PENDING[TOTAL_SLOT];	
	sData 	str;
    
    sData 	sSubcribe;
    char    aPUB_HEADER[PUB_HEADER_LENGTH];
    
    char    *pTopic;
    char    *pModemId;

    uint8_t Status_u8;  
    sData   pRawData;
    uint8_t Type_u8;
    
    uint8_t PubQos;
    
    void    (*pPacket_Handshake) (sData *pData);
    void    (*pPacket_Status) (sData *pData);
    void    (*pPacket_Intan) (sData *pData);
    void    (*pPacket_Alarm) (sData *pData);
    void    (*pPacket_RespAT) (sData *pData);
    void    (*pPacket_SimID) (sData *pData);
    
    void    (*pSER_Config) (sData *strRecv, uint16_t pos);
    uint8_t (*pSER_Setting) (sData *strRecv, uint16_t pos);
    uint8_t (*pSER_Setting_2) (sData *strRecv, uint16_t pos);
    void    (*pSER_Set_Time) (sData *strRecv, uint16_t pos);
    void    (*pSER_Clear_Rec) (sData *strRecv, uint16_t pos);
    void    (*pSER_Req_AT) (sData *strRecv, uint16_t pos);
    void    (*pSER_Req_RF) (sData *strRecv, uint16_t pos);
    void    (*pSER_Req_Update) (sData *strRecv, uint16_t pos);
} sMessageVariable;

/*======================== External Var struct ======================*/

extern const sMessageSend sMessSend[];
extern const sMessageRecv sMessRecv[];

extern sMessageVariable	sMessage;

/*======================== Function ======================*/

/*---------------Function Callback send-----------------------*/
uint8_t     _mDATA_HANDSHAKE(int Kind_Send);

uint8_t     _mDATA_TSVH_MULTI(int Kind_Send);
uint8_t 	_mDATA_TSVH_FLASH(int Kind_Send);
uint8_t     _mDATA_TSVH_OPERA (int Kind_Send);
uint8_t 	_mDATA_REC_HISTORY(int Kind_Send);
uint8_t     _mDATA_INTAN(int Kind_Send);
uint8_t 	_mSEND_SHUTTING_DOWN(int Kind_Send);
uint8_t 	_mDATA_PING(int Kind_Send);
uint8_t 	_mSEND_SIM_ID(int Kind_Send);

uint8_t     _mDATA_EVENT(int Kind_Send);
uint8_t     _mDATA_GPS(int Kind_Send);

uint8_t		_mSEND_RESPOND_SERVER(int Kind_Send);
uint8_t     _mSEND_RESPOND_FROM_RF(int Kind_Send);
uint8_t		_mSEND_RESPOND_SERVER_ACK(int Kind_Send);

uint8_t		_mSEND_SERVER_TIME_PENDING(int Kind_Send);
uint8_t		_mSEND_SERVER_TIME_OK(int Kind_Send);
uint8_t		_mSEND_SERVER_TIME_FAIL(int Kind_Send);
uint8_t		_mSEND_SAVE_BOX_FAIL(int Kind_Send);
uint8_t     _mSEND_SAVE_BOK_OK(int Kind_Send);
uint8_t		_mSEND_EMPTY_MESS(int Kind_Send);

uint8_t     _mSEND_UPDATE_FIRM_OK(int Kind_Send);
uint8_t     _mSEND_UPDATE_FIRM_FAIL(int Kind_Send);
uint8_t     _mSEND_SERVER_FTP_FAIL(int Kind_Send);

uint8_t     _mSEND_RECONNECT_SERVER(int Kind_Send);
uint8_t     _mSEND_HARD_RS_MCU(int Kind_Send);
uint8_t     _mSEND_ALARM(int Kind_Send);
uint8_t     _mDATA_STATUS(int Kind_Send);

/*---------------Function Calback receive-----------------------*/
void    _rREQUEST_RESET(sData *str_Receiv,int16_t Pos);
void    _rINTAN_DATA (sData *str_Receiv,int16_t Pos);
void    _rREQUEST_SETTING(sData *str_Receiv,int16_t Pos);
void    _rUPDATE_TIME_SERVER(sData *str_Receiv,int16_t Pos);
void    _sSETTING_PARAM(sData *str_Receiv, int16_t Pos);
void    _sSETTING_PARAM(sData *strRecv,int16_t Pos);
void    _sSETTING_PARAM_1(sData *strRecv,int16_t Pos);
void    _sSETTING_PARAM_2(sData *strRecv,int16_t Pos);
void    _sSETTING_PARAM_3(sData *strRecv,int16_t Pos);
void    _sSETTING_PARAM_4(sData *strRecv,int16_t Pos);
void    _sSETTING_PARAM_5(sData *strRecv,int16_t Pos);

void    _rRESET_DATA(sData *str_Receiv,int16_t Pos);
void    _rREQ_AT_CMD(sData *str_Receiv,int16_t Pos);
void    _rREQ_AT_TO_RF(sData *str_Receiv,int16_t Pos);
void    _rUPDATE_FIRMWARE(sData *str_Receiv,int16_t Pos);
void    _rREQUEST_STATUS(sData *str_Receiv,int16_t Pos);

/*---------------Function handle-----------------------*/
//handle Send
void        mInit_Header_Subcribe (void);
void 		mSet_default_MQTT(void);

void 		mInit_Header_MQTT(void);
void        mConnect_MQTT (char *ClientID, char *sUser, char *sPass, uint8_t KeepAlive);
void     	mSubcribe_MQTT (void);

void        mData_MQTT (sData *sPayload, uint8_t MessType, uint8_t Qos);
uint8_t     mCheck_Empty_Mess (void);

void        mReset_Raw_Data (void);
void        mUnmark_Mess (uint8_t type);
void        mGet_Data_From_Mem (uint8_t tdata, uint8_t *pData, uint16_t Length);


#endif
