
#include "user_message.h"
#include "user_string.h"

#ifdef PACKET_MQTT_PROTOCOL
#include "MQTTConnect.h"
#include "MQTTFormat.h"
#include "MQTTPacket.h"
#include "MQTTPublish.h"
#include "MQTTSubscribe.h"
#include "MQTTUnsubscribe.h"
#include "StackTrace.h"
#endif


/*======================== Structs var======================*/
const sMessageSend     sMessSend[] =
{
    //Mess wait ACK
    {TOPIC_NOTIF,						NULL,								{(uint8_t*)"Notif",5}},

    {DATA_HANDSHAKE,					_mDATA_HANDSHAKE,				    {(uint8_t*)"MePDV",5}}, //
    {SEND_SERVER_TIME_PENDING,			_mSEND_SERVER_TIME_PENDING,			{(uint8_t*)"sTime",5}}, 
    
    {DATA_TSVH_MULTI,				    _mDATA_TSVH_MULTI,			        {(uint8_t*)"Multi",5}}, //
    {DATA_TSVH_FLASH,				    _mDATA_TSVH_FLASH,			        {(uint8_t*)"Flash",5}}, //
    {DATA_TSVH_OPERA,				    _mDATA_TSVH_OPERA,			        {(uint8_t*)"Opera",5}}, //
    {DATA_INTAN,				        _mDATA_INTAN,				        {(uint8_t*)"Intan",5}}, //
    
    {DATA_REC_HISTORY,				    _mDATA_REC_HISTORY,				    {(uint8_t*)"Debug",5}}, //

    {DATA_EVENT,					    _mDATA_EVENT,				        {(uint8_t*)"Event",5}}, //
    {DATA_GPS,					        _mDATA_GPS,				            {(uint8_t*)"Gpsda",5}}, //
    
    {SEND_SIM_ID,					    _mSEND_SIM_ID,					    {(uint8_t*)"SimID",5}},
    {SEND_RESPOND_SERVER_ACK,	    	_mSEND_RESPOND_SERVER_ACK,			{(uint8_t*)"sCoFi",5}},
    {SEND_SAVE_BOX_FAIL,				_mSEND_SAVE_BOX_FAIL,				{(uint8_t*)"sCoFi",5}},
    {SEND_SAVE_BOX_OK,				    _mSEND_SAVE_BOK_OK,				    {(uint8_t*)"sCoFi",5}},
    
    {SEND_UPDATE_FIRM_OK,				_mSEND_UPDATE_FIRM_OK,				{(uint8_t*)"Notif",5}},    
    {SEND_UPDATE_FIRM_FAIL,				_mSEND_UPDATE_FIRM_FAIL,			{(uint8_t*)"Notif",5}},    
    {SEND_SERVER_FTP_FAIL,				_mSEND_SERVER_FTP_FAIL,				{(uint8_t*)"Notif",5}},
    //Mess No wait ACK
    {SEND_RESPOND_SERVER,				_mSEND_RESPOND_SERVER,				{(uint8_t*)"sCoFi",5}},
    {SEND_RESPOND_FROM_RF,				_mSEND_RESPOND_FROM_RF,				{(uint8_t*)"ReqRF",5}},
    {SEND_SHUTTING_DOWN,				_mSEND_SHUTTING_DOWN,				{(uint8_t*)"Reset",5}},
    {SEND_RECONNECT_SERVER,				_mSEND_RECONNECT_SERVER,			{(uint8_t*)"ReCon",5}},
    {SEND_HARD_RS_MCU,				    _mSEND_HARD_RS_MCU,			        {(uint8_t*)"Alarm",5}},
    {SEND_ALARM,				        _mSEND_ALARM,				        {(uint8_t*)"Alarm",5}},
    {DATA_PING,							_mDATA_PING,						{(uint8_t*)"mPing",5}}, //
    {DATA_PING_2,					    _mDATA_PING,						{(uint8_t*)"mPing",5}}, //
    {DATA_STATUS,				        _mDATA_STATUS,				        {(uint8_t*)"Statu",5}}, //
    //
    {SEND_SERVER_TIME_OK,				_mSEND_SERVER_TIME_OK,				{(uint8_t*)"Notif",5}},   
    {SEND_SERVER_TIME_FAIL,				_mSEND_SERVER_TIME_FAIL,			{(uint8_t*)"Notif",5}},  
    {SEND_EMPTY_MESS,				    _mSEND_EMPTY_MESS,				    {(uint8_t*)"Notif",5}},  
    
    {END_MQTT_SEND,						NULL,								{NULL}},
};

/* */
const sMessageRecv sMessRecv[] =
{
    // ban tin danh rieng cho SIM
    {REQUEST_RESET,				_rREQUEST_RESET,				{(uint8_t*)"Reset",5}},
    {INTAN_DATA,                _rINTAN_DATA,                   {(uint8_t*)"Intan",5}},
    {REQUEST_SETTING,			_rREQUEST_SETTING,				{(uint8_t*)"sCoFi",5}},
    {SER_SETTING_PARAM,			_sSETTING_PARAM,				{(uint8_t*)"sSett",5}},
    {SER_SETTING_PARAM_1,	    _sSETTING_PARAM_1,				{(uint8_t*)"sSet1",5}},
    {SER_SETTING_PARAM_2,		_sSETTING_PARAM_2,				{(uint8_t*)"sSet2",5}},
    {SER_SETTING_PARAM_3,		_sSETTING_PARAM_3,				{(uint8_t*)"sSet3",5}},
    {SER_SETTING_PARAM_4,		_sSETTING_PARAM_4,				{(uint8_t*)"sSet4",5}},
    {SER_SETTING_PARAM_5,		_sSETTING_PARAM_5,				{(uint8_t*)"sSet5",5}},
    {UPDATE_TIME_SERVER,		_rUPDATE_TIME_SERVER,			{(uint8_t*)"sTime",5}},
    {RESET_DATA,				_rRESET_DATA,	    			{(uint8_t*)"Clear",5}},
    {REQUEST_AT,				_rREQ_AT_CMD,	    			{(uint8_t*)"ReqAT",5}},
    {REQUEST_RF,				_rREQ_AT_TO_RF,	    			{(uint8_t*)"ReqRF",5}},
    
    {REQUEST_UPDATE_FIRM,	    _rUPDATE_FIRMWARE,		        {(uint8_t*)"rFirm",5}},
    {REQUEST_STATUS,		    _rREQUEST_STATUS,			    {(uint8_t*)"Statu",5}},
    
    {END_MQTT_RECEI,			NULL,							{NULL}},
};

static uint8_t aDATA_RAW[1024];

sMessageVariable	sMessage = 
{
    .pRawData = {aDATA_RAW, 0},
};

#ifdef PACKET_MQTT_PROTOCOL
MQTTPacket_connectData 	Connect_Packet = MQTTPacket_connectData_initializer;
MQTTString 				topicString = MQTTString_initializer;
#endif

uint8_t	aSubcribe_Header[SUB_HEADER_LENGTH];
uint8_t	aPublish_Header[SUB_HEADER_LENGTH];

uint8_t	aDATA_MQTT[MAX_LENGTH_MQTT];        //Buffer chưa data da duoc dong goi MQTT: Bao gom Payload + Header MQTT

/*======================== Function ======================*/

void mSet_default_MQTT(void)
{
	sMessage.str.Data_a8 		= aDATA_MQTT;
	sMessage.str.Length_u16 	= 0;

    sMessage.Status_u8         = FALSE;
}

char *mGet_Pub_Header(int step, uint8_t kind)
{
    UTIL_MEM_set(sMessage.aPUB_HEADER, 0, sizeof(sMessage.aPUB_HEADER));
    
    if (kind == 0) {
        sprintf(sMessage.aPUB_HEADER, "%s%s/%s", sMessage.pTopic, (char *) sMessSend[step].sKind.Data_a8, sMessage.pModemId);
	} else {
        sprintf(sMessage.aPUB_HEADER, "%s%s/%s", sMessage.pTopic, (char *) sMessRecv[step].sKind.Data_a8, sMessage.pModemId);
    }
    
    return sMessage.aPUB_HEADER;
}



void mData_MQTT (sData *sPayload, uint8_t MessType, uint8_t Qos)
{
    //Dua con tro MQTT.str.data_u8 tro dau mang aDATA_MQTT
    mSet_default_MQTT();     
    
    sMessage.PubQos = Qos;
	topicString.cstring = mGet_Pub_Header(MessType, 0);

#ifdef MQTT_PROTOCOL
    if (UTIL_var.ModeConnNow_u8 == _CONNECT_FTP_UPLOAD) {
        UTIL_MEM_cpy(sMessage.str.Data_a8, sPayload->Data_a8, sPayload->Length_u16);
        sMessage.str.Length_u16 = sPayload->Length_u16;
    } else {
        sMessage.str.Length_u16 = MQTTSerialize_publish(sMessage.str.Data_a8, sizeof(aDATA_MQTT), 0, Qos, \
                0, 1, topicString, sPayload->Data_a8, sPayload->Length_u16);
    }
#endif // MQTT_PROTOCOL  
}

/*============= Func Callback ============================*/

uint8_t _mDATA_HANDSHAKE(int Kind_Send)
{
    uint8_t     aTEMP_PAYLOAD[1024] = {0};
    sData       strPayload = {aTEMP_PAYLOAD, 0};

    if (sMessage.pPacket_Handshake != NULL)
        sMessage.pPacket_Handshake(&strPayload);
    //Send MQTT
    mData_MQTT(&strPayload, Kind_Send, MY_QOS);

    return 1;
}


uint8_t _mDATA_TSVH_FLASH(int Kind_Send)
{
    mData_MQTT(&sMessage.pRawData, Kind_Send, MY_QOS);
    
    return 1;
}



uint8_t _mDATA_TSVH_MULTI(int Kind_Send)
{
    mData_MQTT(&sMessage.pRawData, Kind_Send, MY_QOS);
    
    return 1;
}


uint8_t _mDATA_TSVH_OPERA (int Kind_Send)
{   
    mData_MQTT(&sMessage.pRawData, Kind_Send, MY_QOS);
    
    return 1;
}


uint8_t _mDATA_INTAN(int Kind_Send)
{
    uint8_t     aTEMP_PAYLOAD[1024] = {0};
    sData       strPayload = {aTEMP_PAYLOAD, 0};
    
    if (sMessage.pPacket_Intan != NULL)
        sMessage.pPacket_Intan(&strPayload);

    mData_MQTT(&strPayload, Kind_Send, MY_QOS);
    
    if (sMessage.str.Length_u16 == 0)
        return false;

    return 1;
}


uint8_t _mDATA_REC_HISTORY(int Kind_Send)
{
    
    
    return 1;
}



uint8_t _mSEND_SHUTTING_DOWN(int Kind_Send)
{
    sData sPayLoadShutdown = {(uint8_t*)"MCU shutting down", 17};

    mData_MQTT(&sPayLoadShutdown, Kind_Send, 0);

    return 1;
}

uint8_t _mSEND_RECONNECT_SERVER(int Kind_Send)
{
    sData sPayLoadReConn = {(uint8_t*)"MQTT ReConnect!", 15};

    mData_MQTT(&sPayLoadReConn, Kind_Send, 0);

    return 1;
}

uint8_t _mSEND_HARD_RS_MCU(int Kind_Send)
{
    sData sPayLoadReConn = {(uint8_t*)"Alarm: Hard Reset MCU!", 23};

    mData_MQTT(&sPayLoadReConn, Kind_Send, 0);

    return 1;
}


uint8_t _mSEND_ALARM(int Kind_Send)
{
    uint8_t     aTEMP_PAYLOAD[512] = {0};
    sData       strPayload = {aTEMP_PAYLOAD, 0};

    if (sMessage.pPacket_Alarm != NULL)
        sMessage.pPacket_Alarm(&strPayload);
       
    mData_MQTT(&strPayload, Kind_Send, 0);
        
    return 1;
}

/*
 * 		PUBLISH MESSAGE
 */
uint8_t _mDATA_PING(int Kind_Send)
{   
//    mSet_default_MQTT(); 
//
//    *(sMessage.str.Data_a8 + sMessage.str.Length_u16++) = MY_PINGREQ;
//    *(sMessage.str.Data_a8 + sMessage.str.Length_u16++) = 0x00;
//       
//    return 1;
  
//    mSet_default_MQTT(); 
//    
//    sMQTT.PulishQos = 0;  
//	sMQTT.PulishRetained = 0;
//	sMQTT.PulishPacketId = 1;
//
//    *(sMQTT.str.Data_a8 + sMQTT.str.Length_u16++) = MY_PINGREQ;
//    *(sMQTT.str.Data_a8 + sMQTT.str.Length_u16++) = 0x00;
  
    mSet_default_MQTT(); 

    uint8_t var = 0;
    uint8_t	    i = 0;
	uint8_t     TempCrc = 0;
    uint8_t     aTEMP_PAYLOAD[128] = {0};
    sData       strPayload = {aTEMP_PAYLOAD, 0};
    
    Reset_Buff(&strPayload);
    *(strPayload.Data_a8 + strPayload.Length_u16++) = '#';
	// SIM_ID
	for (var = 0; var < strlen(sSimCommInfor.aSIM_ID); ++var)
		*(strPayload.Data_a8 + strPayload.Length_u16++)  = sSimCommInfor.aSIM_ID[var];
    //Insert Version
    *(strPayload.Data_a8 + strPayload.Length_u16++)  = '#';
    for (var = 0; var < strlen(sFirmVersion); ++var)
		*(strPayload.Data_a8 + strPayload.Length_u16++)  = sFirmVersion[var];

    *(strPayload.Data_a8 + strPayload.Length_u16++)  = '#';
    
#ifdef USING_APP_SIM
    *(strPayload.Data_a8 + strPayload.Length_u16++)  = 'C';
    *(strPayload.Data_a8 + strPayload.Length_u16++)  = 'S';
    *(strPayload.Data_a8 + strPayload.Length_u16++)  = 'Q';
    *(strPayload.Data_a8 + strPayload.Length_u16++)  = '-';
    
    Convert_Uint64_To_StringDec(&strPayload, sSimCommInfor.RSSI_u8, 0);
    
    *(strPayload.Data_a8 + strPayload.Length_u16++)  = '#';
#endif

#ifdef USING_APP_SIM
//    if (sSimVar.ModeConnectNow_u8 == MODE_CONNECT_DATA_MAIN)
//    {
        //IP
//        *(sMQTT.sPayload.Data_a8 + sMQTT.sPayload.Length_u16++) = sSimInfor.sServer.sIPMain.Length_u16 + sSimInfor.sServer.sPortMain.Length_u16 + 1; // length
        for (var = 0; var < strlen(sSimCommVar.sServer.sIP) ; ++var)
            *(strPayload.Data_a8 + strPayload.Length_u16++)  = sSimCommVar.sServer.sIP[var];
        //Port
        *(strPayload.Data_a8 + strPayload.Length_u16++) = ':'; // length
        for (var = 0; var < strlen(sSimCommVar.sServer.sPort); ++var)
            *(strPayload.Data_a8 + strPayload.Length_u16++)  = sSimCommVar.sServer.sPort[var];
//    } else
//    {
//        //IP
////         *(sMQTT.sPayload.Data_a8 + sMQTT.sPayload.Length_u16++) = sSimInfor.sServer.sIPBackUp.Length_u16 + sSimInfor.sServer.sPortBackUp.Length_u16 + 1;
//        for (var = 0; var < sSimCommInfor.sServer.sIP.Length_u16; ++var)
//            *(sMQTT.str.Data_a8 + sMQTT.str.Length_u16++)  = sSimCommInfor.sServer.sIPBackUp.Data_a8[var];
//        //Port
//        *(sMQTT.str.Data_a8 + sMQTT.str.Length_u16++) = ':';
//        for (var = 0; var < sSimCommInfor.sServer.sPortBackUp.Length_u16; ++var)
//            *(sMQTT.str.Data_a8 + sMQTT.str.Length_u16++)  = sSimCommInfor.sServer.sPortBackUp.Data_a8[var];
//    }
    
    *(strPayload.Data_a8 + strPayload.Length_u16++)  = '#';
#endif
    
    	// caculator crc
	strPayload.Length_u16++;
	for (i = 0; i < (strPayload.Length_u16 - 1); i++)
		TempCrc ^= *(strPayload.Data_a8 + i);

       
    mData_MQTT(&strPayload, Kind_Send, MY_QOS);
    return 1;
}



uint8_t _mDATA_STATUS(int Kind_Send)
{   
    uint8_t     aTEMP_PAYLOAD[128] = {0};
    sData       strPayload = {aTEMP_PAYLOAD, 0};
    
    if (sMessage.pPacket_Status != NULL)
        sMessage.pPacket_Status(&strPayload);
    
    mData_MQTT(&strPayload, Kind_Send, 0);
    
    return 1;
}



uint8_t _mDATA_EVENT(int Kind_Send)
{        
    mData_MQTT(&sMessage.pRawData, Kind_Send, MY_QOS);
    
    return 1;
}


uint8_t _mDATA_GPS(int Kind_Send)
{        
    mData_MQTT(&sMessage.pRawData, Kind_Send, MY_QOS);

    return 1;
}



uint8_t _mSEND_SIM_ID(int Kind_Send)
{
    uint8_t     aTEMP_PAYLOAD[32] = {0};
    sData       strPayload = {aTEMP_PAYLOAD, 0};
    
    if (sMessage.pPacket_SimID != NULL)
        sMessage.pPacket_SimID(&strPayload);

    mData_MQTT(&strPayload, Kind_Send, MY_QOS);

    return 1;
}



uint8_t _mSEND_RESPOND_SERVER(int Kind_Send)
{
	uint8_t     aTEMP_PAYLOAD[256] = {0};
    sData       strPayload = {aTEMP_PAYLOAD, 0};

    if (sMessage.pPacket_RespAT != NULL)
        sMessage.pPacket_RespAT(&strPayload);

    mData_MQTT(&strPayload, Kind_Send, 0);

    return 1;
}


uint8_t _mSEND_RESPOND_FROM_RF(int Kind_Send)
{
	uint8_t     aTEMP_PAYLOAD[256] = {0};
    sData       strPayload = {aTEMP_PAYLOAD, 0};

    if (sMessage.pPacket_RespAT != NULL)
        sMessage.pPacket_RespAT(&strPayload);

    mData_MQTT(&strPayload, Kind_Send, 0);

    return 1;
}




uint8_t _mSEND_RESPOND_SERVER_ACK(int Kind_Send)
{
    uint8_t     aTEMP_PAYLOAD[256] = {0};
    sData       strPayload = {aTEMP_PAYLOAD, 0};

    if (sMessage.pPacket_RespAT != NULL)
        sMessage.pPacket_RespAT(&strPayload);
    
    mData_MQTT(&strPayload, Kind_Send, MY_QOS);

    return 1;
}


//
void mPayload_Load_MesErr(sData *pData, sData *MesErr)
{
 	uint8_t var;

	for (var = 0; var < MesErr->Length_u16; ++var)
		*(pData->Data_a8 + pData->Length_u16++) = *(MesErr->Data_a8 + var);
}

uint8_t _mSEND_SERVER_TIME_PENDING(int Kind_Send)
{
    sData sPayTimePending = {(uint8_t *) "Realtime pending", 16};

    mData_MQTT(&sPayTimePending, Kind_Send, MY_QOS);

    return 1;
}

uint8_t _mSEND_SERVER_TIME_OK(int Kind_Send)
{
    sData sPayTimeOK = {(uint8_t *) "Realtime OK", 11};

    mData_MQTT(&sPayTimeOK, Kind_Send, 0);

    return 1;
}


uint8_t _mSEND_SERVER_TIME_FAIL(int Kind_Send)
{
    sData sPayTimeFail = {(uint8_t *) "Realtime fail", 13};

    mData_MQTT(&sPayTimeFail, Kind_Send, 0);

    return 1;
}

uint8_t _mSEND_SAVE_BOX_FAIL(int Kind_Send)
{
    sData sPaySaveBoxFail = {(uint8_t*)"Luu Kho Fail", 12};

    mData_MQTT(&sPaySaveBoxFail, Kind_Send, MY_QOS);

    return 1;
}

uint8_t _mSEND_SAVE_BOK_OK(int Kind_Send)
{
    sData sPaySaveBoxOK = {(uint8_t*)"\r\nOK", 4};

    mData_MQTT(&sPaySaveBoxOK, Kind_Send, MY_QOS);

    return 1;
}




uint8_t _mSEND_EMPTY_MESS(int Kind_Send)
{
    sData sPayEmpFlashFail = {(uint8_t*)"Empty Mess FLash!", 17};
  
    mData_MQTT(&sPayEmpFlashFail, Kind_Send, 0);

    return 1;
}


uint8_t _mSEND_UPDATE_FIRM_OK(int Kind_Send)
{
    char aTEMP_PAYLOAD[64] = {0};
    sData str;
    
	sprintf(aTEMP_PAYLOAD, "Update Firmware Successfully!");
    UTIL_Log_Str(DBLEVEL_M, aTEMP_PAYLOAD);

    str.Data_a8 = (uint8_t *)aTEMP_PAYLOAD;
    str.Length_u16 = strlen(aTEMP_PAYLOAD);

    mData_MQTT(&str, Kind_Send, 0);

    return 1;
}


uint8_t _mSEND_UPDATE_FIRM_FAIL(int Kind_Send)
{
    char aTEMP_PAYLOAD[64] = {0};
    sData str;
    
	sprintf(aTEMP_PAYLOAD, "Update Firmware Fail!");
    UTIL_Log_Str(DBLEVEL_M, aTEMP_PAYLOAD);

    str.Data_a8 = (uint8_t *)aTEMP_PAYLOAD;
    str.Length_u16 = strlen(aTEMP_PAYLOAD);

    mData_MQTT(&str, Kind_Send, 0);

    return 1;
}

uint8_t _mSEND_SERVER_FTP_FAIL(int Kind_Send)
{
    char aTEMP[] = {"FTP Send Fail!"};
  
    sData str = {(uint8_t *) aTEMP, strlen(aTEMP)};
    
    mData_MQTT(&str, Kind_Send, 0);

    return 1;
}





/*---------------Function Callbacck recei-----------------------*/

void _rREQUEST_RESET(sData *str_Receiv, int16_t Pos)
{
    sMessage.aMESS_PENDING[SEND_SHUTTING_DOWN] = TRUE;
}

void _rINTAN_DATA (sData *str_Receiv,int16_t Pos)
{
#ifdef USING_APP_WM
	//31/8/2022: intan -> measure ap suat -> send Intan
	sWmVar.rIntan_u8 = true;
	//Active event Measure Press
	fevent_active(sEventAppWM, _EVENT_ENTRY_WM);
#endif
}


void _rREQUEST_SETTING(sData *str_Receiv, int16_t Pos)
{
    if (sMessage.pSER_Config != NULL)
        sMessage.pSER_Config(str_Receiv, Pos + sMessRecv[REQUEST_SETTING].sKind.Length_u16);
}


void _sSETTING_PARAM(sData *strRecv,int16_t Pos)
{   
    uint8_t status = false;
    
    if (sMessage.pSER_Setting != NULL) {
        status = sMessage.pSER_Setting(strRecv, Pos + sMessRecv[SER_SETTING_PARAM].sKind.Length_u16);
        
//        if (status == false) {
//            DCU_Respond_str(_AT_REQUEST_SERVER, "FAIL_SET");
//        } 
            
        if (status == true) {          
            Modem_Respond_Str (_UART_SIM, "OK_SET", 1);        
            //danh dau ban tin cap nhat config
            sMessage.aMESS_PENDING[DATA_HANDSHAKE] = TRUE;
        }
    }
}


void _sSETTING_PARAM_1(sData *strRecv,int16_t Pos)
{
    uint8_t status = false;
    
    if (sMessage.pSER_Setting != NULL) {
        status = sMessage.pSER_Setting(strRecv, Pos + sMessRecv[SER_SETTING_PARAM_1].sKind.Length_u16);
        
//        if (status == false) {
//            DCU_Respond_str(_AT_REQUEST_SERVER, "FAIL_SET");
//        } 
            
        if (status == true) {          
            Modem_Respond_Str (_UART_SIM, "OK_SET1", 1);        
            //danh dau ban tin cap nhat config
            sMessage.aMESS_PENDING[DATA_HANDSHAKE] = TRUE;
        }
    }
}

void _sSETTING_PARAM_2(sData *strRecv,int16_t Pos)
{
    uint8_t status = false;
    
    if (sMessage.pSER_Setting != NULL) {
        status = sMessage.pSER_Setting(strRecv, Pos + sMessRecv[SER_SETTING_PARAM_2].sKind.Length_u16);
        
//        if (status == false) {
//            DCU_Respond_str(_AT_REQUEST_SERVER, "FAIL_SET");
//        } 
            
        if (status == true) {          
            Modem_Respond_Str (_UART_SIM, "OK_SET2", 1);        
            //danh dau ban tin cap nhat config
            sMessage.aMESS_PENDING[DATA_HANDSHAKE] = TRUE;
        }
    }
}

void _sSETTING_PARAM_3(sData *strRecv,int16_t Pos)
{
    uint8_t status = false;
    
    if (sMessage.pSER_Setting != NULL) {
        status = sMessage.pSER_Setting(strRecv, Pos + sMessRecv[SER_SETTING_PARAM_3].sKind.Length_u16);
        
//        if (status == false) {
//            DCU_Respond_str(_AT_REQUEST_SERVER, "FAIL_SET");
//        } 
            
        if (status == true) {          
            Modem_Respond_Str (_UART_SIM, "OK_SET3", 1);        
            //danh dau ban tin cap nhat config
            sMessage.aMESS_PENDING[DATA_HANDSHAKE] = TRUE;
        }
    }
}

void _sSETTING_PARAM_4(sData *strRecv,int16_t Pos)
{
    uint8_t status = false;
    
    if (sMessage.pSER_Setting != NULL) {
        status = sMessage.pSER_Setting(strRecv, Pos + sMessRecv[SER_SETTING_PARAM_4].sKind.Length_u16);
        
//        if (status == false) {
//            DCU_Respond_str(_AT_REQUEST_SERVER, "FAIL_SET");
//        } 
            
        if (status == true) {          
            Modem_Respond_Str (_UART_SIM, "OK_SET4", 1);        
            //danh dau ban tin cap nhat config
            sMessage.aMESS_PENDING[DATA_HANDSHAKE] = TRUE;
        }
    }
}


void _sSETTING_PARAM_5(sData *strRecv,int16_t Pos)
{
    uint8_t status = false;
    
    if (sMessage.pSER_Setting != NULL) {
        status = sMessage.pSER_Setting_2(strRecv, Pos + sMessRecv[SER_SETTING_PARAM_5].sKind.Length_u16);
        
        if (status == true) {          
            Modem_Respond_Str (_UART_SIM, "OK_SET5", 1);        
            //danh dau ban tin cap nhat config
            sMessage.aMESS_PENDING[DATA_HANDSHAKE] = TRUE;
        }
    }
}



void _rUPDATE_TIME_SERVER(sData *str_Receiv, int16_t Pos) 
{
    if (sMessage.pSER_Set_Time != NULL)
        sMessage.pSER_Set_Time(str_Receiv, Pos + sMessRecv[UPDATE_TIME_SERVER].sKind.Length_u16);
}

void _rRESET_DATA(sData *str_Receiv,int16_t Pos)
{
    if (sMessage.pSER_Clear_Rec != NULL)
        sMessage.pSER_Clear_Rec(str_Receiv, Pos);
}

void _rREQ_AT_CMD(sData *str_Receiv,int16_t Pos)
{    
    if (sMessage.pSER_Req_AT != NULL)
        sMessage.pSER_Req_AT(str_Receiv, Pos);
}

void _rREQ_AT_TO_RF(sData *str_Receiv,int16_t Pos)
{
    if (sMessage.pSER_Req_RF != NULL)
        sMessage.pSER_Req_RF(str_Receiv, Pos);
}




void _rUPDATE_FIRMWARE(sData *str_Receiv,int16_t Pos)
{
    if (sMessage.pSER_Req_Update != NULL)
        sMessage.pSER_Req_Update(str_Receiv, Pos);
}


void _rREQUEST_STATUS(sData *str_Receiv,int16_t Pos)
{
    sMessage.aMESS_PENDING[DATA_STATUS] = true;
}


/*---------------Function handle-----------------------*/
void mInit_Header_Subcribe (void)
{
#ifdef PACKET_MQTT_PROTOCOL
	MQTTString topicSubcri[] 	= {MQTTString_initializer, MQTTString_initializer};
	int	MsgId, ReqQos[] = {SUB_QOS, SUB_QOS};
    char aSUB_1[SUB_HEADER_LENGTH] = {0};
    char aSUB_2[SUB_HEADER_LENGTH] = {0};

    UTIL_MEM_set(aSubcribe_Header, 0, sizeof(aSubcribe_Header));
	sMessage.sSubcribe.Data_a8 = aSubcribe_Header;
	sMessage.sSubcribe.Length_u16 = 0;
    
    // Topic 1
	sprintf(aSUB_1, "AMI/%s/#", sMessage.pModemId);
    sprintf(aSUB_2, "AMI/sTime/#");

	MsgId = MY_ID1 | (MY_ID2 << 8);
    topicSubcri[0].cstring = aSUB_1;
	topicSubcri[1].cstring = aSUB_2;

	sMessage.sSubcribe.Length_u16 = MQTTSerialize_subscribe(sMessage.sSubcribe.Data_a8, SUB_HEADER_LENGTH, 0, MsgId, 2, topicSubcri, ReqQos);
#endif
}



void mConnect_MQTT (char *ClientID, char *sUser, char *sPass, uint8_t KeepAlive)
{
#ifdef PACKET_MQTT_PROTOCOL
	mSet_default_MQTT();
        
	Connect_Packet.clientID.cstring = ClientID;
	Connect_Packet.keepAliveInterval = KeepAlive;
	Connect_Packet.cleansession = 1;
    
    Connect_Packet.username.cstring = (char*)sUser;
    Connect_Packet.password.cstring = (char*)sPass;

	//dua ban tin chua id username pass vao buff
	sMessage.str.Length_u16 = MQTTSerialize_connect(sMessage.str.Data_a8, sizeof(aDATA_MQTT), &Connect_Packet);
#endif
}



void mSubcribe_MQTT (void)
{
    mInit_Header_Subcribe();

	mSet_default_MQTT();
	sMessage.str.Data_a8 		= sMessage.sSubcribe.Data_a8;
	sMessage.str.Length_u16 	= sMessage.sSubcribe.Length_u16;
}

/*
    Func: Check empty mess
*/
uint8_t mCheck_Empty_Mess (void)
{
    uint16_t i = 0;

    for (i = TOPIC_NOTIF; i < END_MQTT_SEND; i++)
        if (sMessage.aMESS_PENDING[i] == 1)
            return 0;

    return 1;
}

/*
    Func: Get buffer data from Flash
        + Check crc and format again: 
            + False: -> Luu vao flash part B and skip 
            + true: Copy data to buff sim mqtt
                    mark new mess to send now
*/

void mGet_Data_From_Mem (uint8_t tdata, uint8_t *pData, uint16_t Length)
{ 
    uint8_t type = 0;
    //Memset buff 
    Reset_Buff (&sMessage.pRawData);
          
    if (Length > sizeof (aDATA_RAW) )
        Length = sizeof (aDATA_RAW);
    
    UTIL_MEM_cpy(aDATA_RAW, pData, Length);
    sMessage.pRawData.Data_a8 = aDATA_RAW;
    sMessage.pRawData.Length_u16 = Length;
    
    switch (tdata)
    {
        case _MEM_DATA_TSVH:
        #if (TSVH_OPERA_OR_FLASH == TSVH_OPERA)
            type = DATA_TSVH_OPERA;
        #elif (TSVH_OPERA_OR_FLASH == TSVH_FLASH)
            type = DATA_TSVH_FLASH;
        #else
            type = DATA_TSVH_MULTI;
        #endif
            break;
        case _MEM_DATA_EVENT:
            type = DATA_EVENT;
            break;
        case _MEM_DATA_GPS:
            type = DATA_GPS;
            break;
        default:
            return;
    }
    //active event send mess mqtt
    sMessage.aMESS_PENDING[type] = TRUE;  
}



void mUnmark_Mess (uint8_t type)
{
    sMessage.aMESS_PENDING[type] = FALSE; 
    Reset_Buff(&sMessage.pRawData);
}

void mReset_Raw_Data (void)
{
    mUnmark_Mess(DATA_TSVH_MULTI);
    mUnmark_Mess(DATA_TSVH_FLASH);
    mUnmark_Mess(DATA_TSVH_OPERA); 
    mUnmark_Mess(DATA_EVENT); 
    mUnmark_Mess(DATA_GPS); 
}


