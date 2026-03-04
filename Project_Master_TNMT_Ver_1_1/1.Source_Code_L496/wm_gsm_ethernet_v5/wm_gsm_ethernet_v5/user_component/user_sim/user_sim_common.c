

#include "user_sim_common.h"

#include "user_string.h"
#include "user_time.h"

/*=========== Var =====================*/
sFuncCallbackSim     sCbSimComm;

/*============== Struct var ==========*/
sSimInformation	sSimCommInfor = 
{
    .RSSI_u8 = 0,
    .Ber_u8  = 0,
};


sSimVariable	    sSimCommVar;

sSimUpdateVariable    sSimFwUpdate = 
{
    .status_u8 = 0,
};

sSimFileVariable         sFileSys = 
{
    .aNAME = {"/SVWM_Firmware.bin"},
    .LengthName_u8 = 18,
    
    .strHandle = {&sFileSys.aTEMP_STR_HANDLE[0], 0},
    .Handle_u32 = 1,
};


/*=========== Function =====================*/

/*
    Func: Get Serial SIM card 
*/
uint8_t Sim_Common_Get_Sim_ID (sData *strRecv, uint16_t pos)
{
    uint8_t var, mCompare = false;    
    char aTEMP[MAX_LENGTH_SIM_ID] = {0};
        
    if (UTIL_Get_strNum((char *) (strRecv->Data_a8 + pos), aTEMP, sizeof(aTEMP)) == NULL) {
        UTIL_Printf_Str(DBLEVEL_M, "u_sim_com: simid error resp");
        return true;
    }
        
    if (strlen(aTEMP) != strlen(sSimCommInfor.aSIM_ID)) {
        mCompare = true;
	} else {
        //Compare new Sim to Mark Send Sim ID
        for (var = 0; var < strlen(aTEMP); ++var) {
            if (sSimCommInfor.aSIM_ID[var] != aTEMP[var]) {
                mCompare = true; 
                break;
            }
        } 
    }
    //If Mark == 1: Is new sim ID
    if (mCompare == true)
    {
        UTIL_MEM_set(sSimCommInfor.aSIM_ID, 0, sizeof(sSimCommInfor.aSIM_ID));
        for (var = 0; var < strlen(aTEMP); ++var) 
            sSimCommInfor.aSIM_ID[var] = aTEMP[var];

        //Call Func Handler from SIM
        if (sCbSimComm.pHandle_AT != NULL)
            sCbSimComm.pHandle_AT (_SIM_COMM_SIM_CARD);
    }
    
    return true;
}

/*
    Func: Get CSQ
        + Convert to Dec (RSSI)
        + Convert to %
*/

uint8_t Sim_Common_Get_CSQ (sData *strRecv, uint16_t pos)
{
    sSimCommInfor.RSSI_u8 = *(strRecv->Data_a8 + pos) - 0x30;
    sSimCommInfor.Ber_u8  = *(strRecv->Data_a8 + pos + 2) - 0x30;  
    
    if (*(strRecv->Data_a8 + pos + 2) == 0x2C) 
    {
        sSimCommInfor.RSSI_u8 = ((*(strRecv->Data_a8 + pos))<<4) + \
                                                         ((*(strRecv->Data_a8 + pos + 1)) & 0x0F);
        sSimCommInfor.Ber_u8 = *(strRecv->Data_a8 + pos + 3) - 0x30;           // ber 0->7
    }
	/* convert signal strength */
	switch (sSimCommInfor.RSSI_u8) 
    {
		case 0x00:
			sSimCommInfor.RSSI_u8 = 113;
			break;
		case 0x01:
			sSimCommInfor.RSSI_u8 = 111;	
			break;
		case 0x31:
			sSimCommInfor.RSSI_u8 = 51;
			break;
        case 0x99:
			sSimCommInfor.RSSI_u8 = 0;
			break;
		default: //2 den 30/ 2 per step: 0x19 ->19  sau do - 2 = 17
			sSimCommInfor.RSSI_u8 = 113 - ( ( (sSimCommInfor.RSSI_u8) / 16 ) * 10 + sSimCommInfor.RSSI_u8 % 16 ) * 2;
			break;
	}
    
    return 1;
}


/*
    Func: Get Imei Module SIM
*/

uint8_t Sim_Common_Get_Imei(sData *strRecv, uint16_t pos)
{    
    UTIL_MEM_set(sSimCommInfor.aIMEI, 0, sizeof(sSimCommInfor.aIMEI));
    if (UTIL_Get_strNum((char *) (strRecv->Data_a8 + pos), sSimCommInfor.aIMEI, MAX_LENGTH_SIM_IMEI) == NULL) {
        UTIL_Printf_Str(DBLEVEL_M, "u_sim_com: simimei error resp");
    }

    return true;
}


/*
    Func: Get sTime from BTS
*/

uint8_t Sim_Common_Get_CCLK (sData *strRecv, uint16_t pos)
{
    char *sNext = (char *) strRecv->Data_a8 + pos;
    char aTEMP[10] = {0};
    uint8_t aRTC_TEMP[8] = {0}, count = 0;
    
    if ( (strRecv->Length_u16 >  14) && (sSimCommVar.iGetClock_u8 == false) ) {
        while (count < 6)
        {
            UTIL_MEM_set(aTEMP, 0, sizeof(aTEMP));
            sNext = UTIL_Get_strNum((char *) sNext, aTEMP, sizeof(aTEMP));
            if (sNext == NULL) {
                UTIL_Printf_Str(DBLEVEL_M, "u_sim_com: cclk error resp");
                return true;
            }
            
            aRTC_TEMP[count++] = atoi(aTEMP) % 100;  
        }
       
        sSimCommVar.sTime.year   = aRTC_TEMP[0] % 100;
        sSimCommVar.sTime.month  = aRTC_TEMP[1];
        sSimCommVar.sTime.date   = aRTC_TEMP[2];
        sSimCommVar.sTime.hour   = aRTC_TEMP[3];  
        sSimCommVar.sTime.min    = aRTC_TEMP[4];
        sSimCommVar.sTime.sec    = aRTC_TEMP[5];
        
        if (Check_update_Time (&sSimCommVar.sTime) == 0) {
            UTIL_Printf_Str(DBLEVEL_M, "u_sim_com: cclk error format");
            return true;
        }

        //Convert lai day. 1/1/2012 la chu nhat. Thu 2 - cn: 2-8
        sSimCommVar.sTime.day = ((HW_RTC_GetCalendarValue_Second (sSimCommVar.sTime, 1) / SECONDS_IN_1DAY) + 6) % 7 + 1;
       
        if (sCbSimComm.pHandle_AT != NULL)
            sCbSimComm.pHandle_AT(_SIM_COMM_GET_STIME);
        
        sSimCommVar.iGetClock_u8 = true;
    }
    
    return 1;
}




uint8_t Sim_Common_CPIN_Ready (sData *strRecv, uint16_t pos)
{
	sSimCommVar.CallReady_u8     = TRUE;
    sSimCommVar.State_u8   = _SIM_NETWORK;
    
    return 1;
}


uint8_t Sim_Common_Tranparent(sData *strRecv, uint16_t pos)
{
    sSimCommVar.DataMode_u8      = 1;

    return 1;
}


uint8_t Sim_Common_Non_Tranparent(sData *strRecv, uint16_t pos)
{
    sSimCommVar.DataMode_u8      = 0;
    
    return 1;
}


uint8_t Sim_Common_Check_Attached (sData *strRecv, uint16_t pos)
{
	char *p = NULL;

	p = strstr((char*) strRecv->Data_a8, "+CGATT: 1");
	if (p != NULL) {
		sSimCommVar.State_u8  = _SIM_SETTING;
        sSimCommVar.NetReady_u8      = TRUE;
        return 1;
	}

	return 0;
}



uint8_t Sim_Common_SMS_Send_1 (sData *uart_string, uint16_t pos)
{
    if (sCbSimComm.pHandle_AT != NULL)
        sCbSimComm.pHandle_AT (_SIM_COMM_SMS_SEND_1);   
    
    return 1;
}

uint8_t Sim_Common_SMS_Send_2 (sData *uart_string, uint16_t pos)
{
    if (sCbSimComm.pHandle_AT != NULL)
        sCbSimComm.pHandle_AT (_SIM_COMM_SMS_SEND_2);  
    
    return 1;
}

uint8_t Sim_Common_SMS_Send_3 (sData *uart_string, uint16_t pos)
{
    if (sCbSimComm.pHandle_AT != NULL)
        sCbSimComm.pHandle_AT (_SIM_COMM_SMS_SEND_3);  
    
    return 1;
}

uint8_t Sim_Common_Call_1 (sData *uart_string, uint16_t pos)
{
    if (sCbSimComm.pHandle_AT != NULL)
        sCbSimComm.pHandle_AT (_SIM_COMM_CALLING_1);    
    
    return 1;
}

uint8_t Sim_Common_Call_2 (sData *uart_string, uint16_t pos)
{
    if (sCbSimComm.pHandle_AT != NULL)
        sCbSimComm.pHandle_AT (_SIM_COMM_CALLING_2);  
    
    return 1;
}


// tcp mqtt
uint8_t Sim_Common_TCP_Close(sData *strRecv, uint16_t pos)
{
    uint8_t aCID = CID_SERVER;
    
    Sim_Common_Send_AT_Cmd( &aCID, 1 );
    
    return 1;
}

uint8_t Sim_Common_TCP_Close_2(sData *strRecv, uint16_t pos)
{
    if (sSimCommVar.ServerReady_u8 == true) {
        if (sCbSimComm.pHandle_AT != NULL)
            sCbSimComm.pHandle_AT (_SIM_COMM_TCP_CLOSE);  
    }
    
    sSimCommVar.ServerReady_u8 = false;
    
    return 1;
}


uint8_t Sim_Common_TCP_Open_1 (sData *strRecv, uint16_t pos)
{
    char aTEMP[64] = {0};

    sprintf(aTEMP, "%c,\"TCP\",\"%s\",%s", CID_SERVER, sSimCommVar.sServer.sIP, sSimCommVar.sServer.sPort);
    Sim_Common_Send_AT_Cmd( (uint8_t *) aTEMP, strlen(aTEMP) );
      
    sSimCommVar.State_u8        = _SIM_INIT_TCP;
    
    return 1;
}


uint8_t Sim_Common_Ping (sData *strRecv, uint16_t pos)
{
    if (sCbSimComm.pHandle_AT != NULL)
        sCbSimComm.pHandle_AT (_SIM_COMM_TCP_SEND_1);
    
    return 1;
}

uint8_t Sim_Common_TCP_Send_1 (sData *strRecv, uint16_t pos)
{
    if (sCbSimComm.pHandle_AT != NULL)
        sCbSimComm.pHandle_AT (_SIM_COMM_TCP_SEND_1);
    
    return 1;
}

uint8_t Sim_Common_TCP_Send_2 (sData *strRecv, uint16_t pos)
{
    if (sCbSimComm.pHandle_AT != NULL)
        sCbSimComm.pHandle_AT (_SIM_COMM_TCP_SEND_2);
    
    return 1;
}



uint8_t Sim_Common_MQTT_Conn_1 (sData *strRecv, uint16_t pos)
{
    if (sCbSimComm.pHandle_AT != NULL)
        sCbSimComm.pHandle_AT (_SIM_COMM_MQTT_CONN_1);
    
    return 1;
}

uint8_t Sim_Common_MQTT_Conn_2 (sData *strRecv, uint16_t pos)
{
//    if (sCbSimComm.pHandle_AT != NULL)
//        sCbSimComm.pHandle_AT (_SIM_COMM_MQTT_CONN_2);
    
    return 1;
}

uint8_t Sim_Common_MQTT_Sub_1 (sData *strRecv, uint16_t pos)
{
    if (sCbSimComm.pHandle_AT != NULL)
        sCbSimComm.pHandle_AT (_SIM_COMM_MQTT_SUB_1);
    
    return 1;
}

uint8_t Sim_Common_MQTT_Sub_2 (sData *strRecv, uint16_t pos)
{
    sSimCommVar.State_u8 = _SIM_CONN_MQTT;
    
    if (sCbSimComm.pHandle_AT != NULL)
        sCbSimComm.pHandle_AT (_SIM_COMM_MQTT_SUB_2);
    
    return 1;
}


uint8_t Sim_Common_MQTT_Pub_1 (sData *strRecv, uint16_t pos)
{
    if (sCbSimComm.pHandle_AT != NULL)
        sCbSimComm.pHandle_AT (_SIM_COMM_MQTT_PUB_1);
    
    return 1;
}

uint8_t Sim_Common_MQTT_Pub_2 (sData *strRecv, uint16_t pos)
{
    sSimCommVar.State_u8 = _SIM_CONN_MQTT;
    
    if (sCbSimComm.pHandle_AT != NULL)
        sCbSimComm.pHandle_AT (_SIM_COMM_MQTT_PUB_2);
    
    return 1;
}


/*==== htt[ func cb*/
uint8_t Sim_Common_HTTP_Head_1 (sData *strRecv, uint16_t pos)
{
    if (sCbSimComm.pHandle_AT != NULL)
        sCbSimComm.pHandle_AT (_SIM_COMM_HTTP_HEAD_1);
    
    return 1;
}

uint8_t Sim_Common_HTTP_Head_2 (sData *strRecv, uint16_t pos)
{    
    if (sCbSimComm.pHandle_AT != NULL)
        sCbSimComm.pHandle_AT (_SIM_COMM_HTTP_HEAD_2);
    
    return 1;
}

uint8_t Sim_Common_HTTP_Send_1 (sData *strRecv, uint16_t pos)
{
    if (sCbSimComm.pHandle_AT != NULL)
        sCbSimComm.pHandle_AT (_SIM_COMM_HTTP_SEND_1);
    
    return 1;
}

uint8_t Sim_Common_HTTP_Send_2 (sData *strRecv, uint16_t pos)
{
    if (sCbSimComm.pHandle_AT != NULL)
        sCbSimComm.pHandle_AT (_SIM_COMM_HTTP_SEND_2);
    
    return 1;
}


uint8_t Sim_Common_HTTP_Send_Ex (sData *strRecv, uint16_t pos)
{   
    sCbSimComm.pHandle_AT (_SIM_COMM_HTTP_SEND_EX);
    
    return 1;
}


/*================================ Func HTTP ===========================*/

uint8_t Sim_Common_Http_Read_Data (sData *uart_string, uint16_t pos)
{
    int             Pos = 0;
    sData           strHeader      = {(uint8_t*)"$HTTPREAD:", 10};
    sData           strStartHttp   = {(uint8_t*)"\r\n", 2};
    sData           strHeaderFw;
    
    sData           strFix;
    uint16_t        i = 0, j = 0;
    uint8_t         aNumbyte[10] = {0};
    uint8_t         lengthnum = 0;
    uint16_t        NumbyteRecei = 0;
    uint8_t         aTEMP_DATA[8] = {0};
                   
    Pos = Find_String_V2((sData*) &strHeader, uart_string);
	if (Pos >= 0)
	{
        //Get Number recv string
        for (i = Pos; i < uart_string->Length_u16; i++)
        {
            if (*(uart_string->Data_a8 + i) == ',')
            {
                i++;
                while ((*(uart_string->Data_a8 + i) >= 0x30) && (*(uart_string->Data_a8 + i) <= 0x39))
                {
                    aNumbyte[lengthnum++] = *(uart_string->Data_a8 + i) - 0x30;
                    i++;
                }
                
                break;
            }
        }
        //convert ra so byte nhan dc
        for (i = 0; i < lengthnum; i++)
             NumbyteRecei = NumbyteRecei *10 + aNumbyte[i];
        
        if (NumbyteRecei == 0) 
        {
            sSimFwUpdate.FailStatus_u8 = SEND_UPDATE_FIRMWARE_FAIL_LENGTH;
            sCbSimComm.pHandle_AT ( _SIM_COMM_HTTP_UPDATE_FAIL ); 
            return 0;
        }
        
        strFix.Data_a8      = uart_string->Data_a8 + Pos;
        strFix.Length_u16   = uart_string->Length_u16 - Pos;
        
        Pos = Find_String_V2((sData*)&strStartHttp, &strFix);  //check vi tri bat dau cua toan bo data http
        if (Pos >= 0)
        {   
            Pos  += strStartHttp.Length_u16;
            //
            strFix.Data_a8 = strFix.Data_a8 + Pos;
            if (NumbyteRecei > (strFix.Length_u16 - Pos - 6))
            {
                sSimFwUpdate.FailStatus_u8 = SEND_UPDATE_FIRMWARE_FAIL_LENGTH;
                sCbSimComm.pHandle_AT ( _SIM_COMM_HTTP_UPDATE_FAIL ); 
                return 0;
            }
            strFix.Length_u16 = NumbyteRecei;
            //
            sSimFwUpdate.CountByteTotal_u32 += NumbyteRecei;                          // tong so data ma ban tin nhan doc ra cua http
            sSimFwUpdate.AddOffset_u32 = sSimFwUpdate.CountByteTotal_u32;
            //Neu la Pack dau tien, Check 32 byte header xem co chu: ModemFirmware:SV1.1,0x0800C000
            if (sSimFwUpdate.CountPacket_u8 == 0)
            {
                strHeaderFw.Data_a8 = (uint8_t *) sCbSimComm.pGet_Fw_Version;
                strHeaderFw.Length_u16 = LENGTH_HEADER_FW;
        
                Pos = Find_String_V2((sData*)&strHeaderFw, &strFix);  
                if (Pos >= 0)
                    sSimFwUpdate.IsFirmSaoViet_u8 = 1;
                //Lay byte cuoi cung ra: byte 32 cua Packdau tien. Sau header 31 byte
                sSimFwUpdate.LastCrcFile_u8 = *(strFix.Data_a8 + 31);  
                //Check Get Fw Sao Viet
                if (sSimFwUpdate.IsFirmSaoViet_u8 == 0)    //neu khong co header SV code thi return luon
                {
                    sSimFwUpdate.FailStatus_u8 = SEND_UPDATE_FIRMWARE_FAIL_FLASH;
                    sCbSimComm.pHandle_AT ( _SIM_COMM_HTTP_UPDATE_FAIL ); 
                    return 0;
                }
                //Fix string data
                strFix.Data_a8 += 32;
                strFix.Length_u16 -= 32;
            }
        
            //Cacal Crc
            for (i = 0; i < strFix.Length_u16; i++)
                sSimFwUpdate.LasCrcCal_u8 += *(strFix.Data_a8 + i);
                      
            //Increase Count Packet
            sSimFwUpdate.CountPacket_u8++;
            
            //Ghi data vao. Bu bao cho thanh boi cua FLASH_BYTE_WRTIE
            if ((strFix.Length_u16 % FLASH_BYTE_WRTIE) != 0)
                strFix.Length_u16 += ( FLASH_BYTE_WRTIE - (strFix.Length_u16 % FLASH_BYTE_WRTIE) );
            //write fw to Mem: Onchip flash or External mem
            for (i = 0; i < (strFix.Length_u16 / FLASH_BYTE_WRTIE); i++)
            {
                //Phai chia ra buffer rieng moi k loi
                for (j = 0; j < FLASH_BYTE_WRTIE; j++)
                    aTEMP_DATA[j] = *(strFix.Data_a8 + i * FLASH_BYTE_WRTIE + j); 
                //
                if(OnchipFlashWriteData(sSimFwUpdate.AddSave_u32, &aTEMP_DATA[0], FLASH_BYTE_WRTIE) != HAL_OK)
                {
                    sSimFwUpdate.FailStatus_u8 = SEND_UPDATE_FIRMWARE_FAIL_FLASH;
                    sCbSimComm.pHandle_AT ( _SIM_COMM_HTTP_UPDATE_FAIL ); 
                    return 0;
                } else
                    sSimFwUpdate.AddSave_u32 += FLASH_BYTE_WRTIE;
            }          
                 
            //kiem tra xem nhan du byte chua de bao ok
            if (sSimFwUpdate.CountByteTotal_u32 >= sSimFwUpdate.FirmLength_u32)  //Binh thuong la dau == //Test de >=           
            {
                if (sSimFwUpdate.LastCrcFile_u8 == sSimFwUpdate.LasCrcCal_u8)
                {
                    sSimFwUpdate.CountByteTotal_u32 += (FLASH_BYTE_WRTIE - sSimFwUpdate.CountByteTotal_u32 % FLASH_BYTE_WRTIE);
                    
                    sCbSimComm.pHandle_AT ( _SIM_COMM_HTTP_UPDATE_OK );
                    return 1;
                } else
                {
                    sSimFwUpdate.FailStatus_u8 = SEND_UPDATE_FIRMWARE_FAIL_LENGTH;
                    sCbSimComm.pHandle_AT ( _SIM_COMM_HTTP_UPDATE_FAIL ); 
                    return 0;
                }
            }
            
            //Continue
            sCbSimComm.pHandle_AT ( _SIM_COMM_HTTP_READ_OK );
            return 1;
        }
    }
    //Set Fail Setup Http
    sSimFwUpdate.FailStatus_u8 = SEND_UPDATE_FIRMWARE_FAIL_SETUP;
    sCbSimComm.pHandle_AT ( _SIM_COMM_HTTP_UPDATE_FAIL ); 
    
    return 0;
}




/*================================ Func URC ===========================*/
/*
    Func: Call Handler URC Reset SIM
*/
uint8_t Sim_Common_URC_Reset (sData *strRecv, uint16_t pos)
{
    sCbSimComm.phandle_URC(_SIM_COMM_URC_RESET_SIM);
    
    return 1;
}

/*
    Func: Call Handler URC LOST SIM
*/
uint8_t Sim_Common_URC_Lost_Card (sData *strRecv, uint16_t pos)
{
    sCbSimComm.phandle_URC(_SIM_COMM_URC_SIM_LOST);
    
    return 1;
}

/*
    Func: Call Handler URC LOST SIM
*/
uint8_t Sim_Common_URC_Closed (sData *strRecv, uint16_t pos)
{ 
    sCbSimComm.phandle_URC(_SIM_COMM_URC_CLOSED);  
    
    return 1;
}

uint8_t Sim_Common_URC_Call_Ready (sData *strRecv, uint16_t pos)
{
    sSimCommVar.CallReady_u8 = TRUE;
    
    return 1;
}

/*
    Func: Call Handler URC Error
*/
uint8_t Sim_Common_URC_Error (sData *strRecv, uint16_t pos)
{
    sCbSimComm.phandle_URC(_SIM_COMM_URC_ERROR);
    
    return 1;
}


/*
    Func: Call Handler URC Receive server
*/
uint8_t Sim_Common_URC_Recv_Server (sData *strRecv, uint16_t pos)
{
    sCbSimComm.phandle_URC(_SIM_COMM_URC_RECEIV_SERVER);
    
    return 1;
}

/*
    Func: check dl: check tiep \r\n tu vi tri nay
        sau do den lenh
*/

uint8_t Sim_Common_URC_Recv_Dl (sData *strRecv, uint16_t pos)
{
    sCbSimComm.phandle_Recv(strRecv->Data_a8 + pos, strRecv->Length_u16 - pos);
         
    return 1;
}


/*
    Func: Call Handler URC calling
*/
uint8_t Sim_Common_URC_Calling (sData *strRecv, uint16_t pos)
{
    sCbSimComm.phandle_URC(_SIM_COMM_URC_CALLING); 
    
    return 1;
}

uint8_t Sim_Common_URC_Call_Fail (sData *strRecv, uint16_t pos)
{    
    sCbSimComm.phandle_URC(_SIM_COMM_URC_CALL_FAIL); 
    
    return 1;
}


uint8_t Sim_Common_URC_Sms_Content (sData *strRecv, uint16_t pos)
{
    sCbSimComm.phandle_URC(_SIM_COMM_URC_SMS_CONTENT);
    
    return 1;
}

uint8_t Sim_Common_URC_Sms_Indi (sData *strRecv, uint16_t pos)
{
    sCbSimComm.phandle_URC(_SIM_COMM_URC_SMS_IND);
    
    return 1;
}



/*================================ Func Process ===========================*/

/*
    Func: Init Struct Update Fw variable
*/

void Sim_Common_Init_Struct_UpdateFw (void)
{
	sSimFwUpdate.CountByteTotal_u32 			= 0;
	sSimFwUpdate.Pending_u8 					= 0;
    //
    sSimFwUpdate.AddOffset_u32                  = 0;
    sSimFwUpdate.CountPacket_u8                 = 0;
    sSimFwUpdate.LastCrcFile_u8                 = 0;
    sSimFwUpdate.LasCrcCal_u8                   = 0;
    sSimFwUpdate.IsFirmSaoViet_u8               = FALSE;
}

/*
    Func: Init Update fw before send AT+HTTP Read
        + Init Struct var
        + Erase area New Fw
*/

uint8_t Sim_Common_Init_UpdateFw (void)
{      
    if ( sCbSimComm.pConfig_Update == NULL)
        return 0;
    
    sCbSimComm.pConfig_Update();
    
    Sim_Common_Init_Struct_UpdateFw();
    Sim_Common_Init_Struct_FileSys();

    return 1;
}


//
void Sim_Common_Send_AT_Cmd (uint8_t *pData, uint16_t length) 
{
//    if (length < 64)
//    {
        UTIL_Printf( DBLEVEL_M, pData, length );
//    } else
//    {
//        UTIL_Printf( DBLEVEL_M, pData, 64 );
//    }
    
    if (sCbSimComm.pSend != NULL)
        sCbSimComm.pSend(pData, length);
}




void Sim_Common_Init_Struct_FileSys (void)
{
    sFileSys.Handle_u32 = 1;
    UTIL_MEM_set ( sFileSys.aTEMP_STR_HANDLE, 0, sizeof (sFileSys.aTEMP_STR_HANDLE) );
    sFileSys.strHandle.Length_u16 = 0;
}

/*
    Func: Clear Queue step
*/

void Sim_Common_Queue_Clear (Struct_Queue_Type *pQueue)
{
    qQueue_Clear(pQueue);
}



uint8_t Sim_Common_Analys_Error (void)
{
    if (sSimCommVar.RxPinReady_u8 == false)  
    {
        UTIL_Printf_Str( DBLEVEL_M, "u_sim: error power or rx pin uart\r\n" );
        return _SIM_COMM_ERROR_PIN_RX;
    }
    
    if (sSimCommVar.TxPinReady_u8 == false)  
    {
        UTIL_Printf_Str( DBLEVEL_M, "u_sim: error tx pin uart\r\n" );
        return _SIM_COMM_ERROR_PIN_TX;
    }
    
    if (sSimCommVar.CallReady_u8 == false)  
    {
        UTIL_Printf_Str( DBLEVEL_M, "u_sim: error card sim\r\n" );
        return _SIM_COMM_ERROR_CARD;
    }
    
    if (sSimCommVar.NetReady_u8 == false)  
    {
        UTIL_Printf_Str( DBLEVEL_M, "u_sim: error network\r\n" );
        return _SIM_COMM_ERROR_ATTACH;
    }
    
    if (sSimCommVar.ServerReady_u8 == false)  
    {
        UTIL_Printf_Str( DBLEVEL_M, "u_sim: error connect server\r\n" );
        return _SIM_COMM_ERROR_TCP;
    }
    
    return _SIM_COMM_ERROR_NONE;
}

sSimInformation *pSim_Common_Get_Infor (void)
{
    return &sSimCommInfor;
}


/**/

void Sim_Common_Init (sSimVariable *sSimCommVar)
{
    sSimCommVar->CallReady_u8           = false;

    sSimCommVar->State_u8               = _SIM_START; 
    
    sSimCommVar->RxPinReady_u8          = false;
    sSimCommVar->TxPinReady_u8          = false;
    sSimCommVar->NetReady_u8            = false;
    sSimCommVar->ServerReady_u8         = false;
}






