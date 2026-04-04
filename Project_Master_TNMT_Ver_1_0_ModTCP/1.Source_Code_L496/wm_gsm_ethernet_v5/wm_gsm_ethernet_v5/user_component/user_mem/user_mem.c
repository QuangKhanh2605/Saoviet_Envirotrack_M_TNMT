
#include "user_mem.h"
//#include "user_define.h"
//#include "user_convert_variable.h"


/*======== Struct var ===========*/
#ifdef MEM_REC_IN_FLASH
    sMemRecordInfor     sRecTSVH = 
    {
        .AddrStart_u32      = ADDR_MESS_A_START,   
        .AddrStop_u32       = ADDR_MESS_A_STOP,

        .Max_u16            = FLASH_MAX_MESS_TSVH_SAVE, 
        .Size_u16           = SIZE_DATA_TSVH, 
    };
    
    sMemRecordInfor     sRecEvent = 
    {
        .AddrStart_u32      = ADDR_EVENT_START,
        .AddrStop_u32       = ADDR_EVENT_STOP,

        .Max_u16            = FLASH_MAX_MESS_EVENT_SAVE,
        .Size_u16           = SIZE_DATA_EVENT,
    };
    
    //Struct Log data
    sMemRecordInfor     sRecLog =    
    {
        .AddrStart_u32      = ADDR_LOG_START,   
        .AddrStop_u32       = ADDR_LOG_STOP,           

        .Max_u16            = FLASH_MAX_MESS_LOG_SAVE,     
        .Size_u16           = SIZE_DATA_LOG,
    };
    
    //Struct GPS Data
    sMemRecordInfor     sRecGPS =    
    {
        .AddrStart_u32      = ADDR_GPS_START,   
        .AddrStop_u32       = ADDR_GPS_STOP,

        .Size_u16     = 0,
        .Max_u16      = 0,     
    };
#endif
    
#ifdef MEM_REC_EX_EEPROM
    sMemRecordInfor     sRecTSVH = 
    {
        .AddrStart_u32      = CAT_ADDR_MESS_A_START,                      
        .AddrStop_u32       = CAT_ADDR_MESS_A_STOP,

        .Max_u16            = CAT_MAX_MESS_TSVH_SAVE,  
        .Size_u16           = CAT_SIZE_DATA_TSVH, 
    };

    sMemRecordInfor     sRecEvent = 
    {
        .AddrStart_u32      = CAT_ADDR_EVENT_START,
        .AddrStop_u32       = CAT_ADDR_EVENT_STOP,        
    
        .Max_u16            = CAT_MAX_MESS_EVENT_SAVE,
        .Size_u16           = CAT_SIZE_DATA_EVENT, 
    };

    sMemRecordInfor     sRecLog = 
    {
        .AddrStart_u32      = CAT_ADDR_LOG_START,   
        .AddrStop_u32       = CAT_ADDR_LOG_STOP,

        .Max_u16            = CAT_MAX_MESS_LOG_SAVE,
        .Size_u16           = CAT_SIZE_DATA_LOG,
    };
    
    //Struct GPS Data
    sMemRecordInfor     sRecGPS =    
    {
        .AddrStart_u32      = CAT_ADDR_GPS_START,   
        .AddrStop_u32       = CAT_ADDR_GPS_STOP,

        .Size_u16           = CAT_SIZE_DATA_GPS,
        .Max_u16            = CAT_MAX_MESS_GPS_SAVE,     
    };
#endif
    
#ifdef MEM_REC_EX_FLASH
     
    sMemRecordInfor     sRecTSVH = 
    {
        .AddrStart_u32      = FLASH_ADDR_TSVH_START,
        .AddrStop_u32       = FLASH_ADDR_TSVH_STOP,

        .Size_u16           = FLASH_SIZE_DATA_TSVH, 
        .Max_u16            = FLASH_MAX_RECORD_TSVH, 
    };
    
    sMemRecordInfor     sRecEvent = 
    {
        .AddrStart_u32      = FLASH_ADDR_EVENT_START,
        .AddrStop_u32       = FLASH_ADDR_EVENT_STOP, 

        .Size_u16           = FLASH_SIZE_DATA_EVENT, 
        .Max_u16            = FLASH_MAX_RECORD_EVENT, 
    };
    
    sMemRecordInfor     sRecLog = 
    {
        .AddrStart_u32      = FLASH_ADDR_LOG_START,
        .AddrStop_u32       = FLASH_ADDR_LOG_STOP, 

        .Size_u16           = FLASH_SIZE_DATA_LOG, 
        .Max_u16            = FLASH_MAX_RECORD_LOG, 
    };

    //Struct GPS Data
    sMemRecordInfor     sRecGPS =    
    {
        .AddrStart_u32      = FLASH_ADDR_GPS_START,   
        .AddrStop_u32       = FLASH_ADDR_GPS_STOP, 

        .Size_u16           = FLASH_SIZE_DATA_GPS,
        .Max_u16            = FLASH_MAX_RECORD_GPS,     
    };
#endif 
    
sMemVariable sMemVar;
    
static uint8_t _Cb_mem_Test_Write(uint8_t event);
static uint8_t _Cb_mem_Ctrl_Read (uint8_t event);
static uint8_t _Cb_mem_Ctrl_Write (uint8_t event);
static uint8_t _Cb_mem_Ctrl_Request (uint8_t event);
static uint8_t _Cb_mem_Erase (uint8_t event);

/*================ Var struct =================*/  
sEvent_struct sEventMem[] = 
{
    { _EVENT_MEM_TEST_WRITE,  		    1, 0, 5,        _Cb_mem_Test_Write }, 
    { _EVENT_MEM_ERASE,  	            0, 0, 0,        _Cb_mem_Erase },
    
    { _EVENT_MEM_CTRL_READ,  		    0, 0, 1000,     _Cb_mem_Ctrl_Read },    
    { _EVENT_MEM_CTRL_WRITE,  		    0, 0, 1000,     _Cb_mem_Ctrl_Write },   
    { _EVENT_MEM_CTRL_REQ,  		    0, 0, 6000,     _Cb_mem_Ctrl_Request },
};    

static sMemQueueWrite    sQMemWrite[20];
static sMemQueueRead     sQMemRead[20];

Struct_Queue_Type   qMemRead;
Struct_Queue_Type   qMemWrite;


/*================ Function ===================*/

uint8_t Mem_Task(void)
{
    uint8_t i = 0;
	uint8_t Result = false;

	for (i = 0; i < _EVENT_END_MEM; i++)
	{
		if (sEventMem[i].e_status == 1)
		{
            if (i != _EVENT_MEM_CTRL_READ)
                Result = true;

			if ((sEventMem[i].e_systick == 0) ||
					((HAL_GetTick() - sEventMem[i].e_systick)  >=  sEventMem[i].e_period))
			{
                sEventMem[i].e_status = 0;  //Disable event
				sEventMem[i].e_systick = HAL_GetTick();
				sEventMem[i].e_function_handler(i);
			}
		}
	}
    
    Result |= OnFlash_Task(); 

#ifdef MEM_REC_EX_EEPROM
    Result |= CAT24Mxx_Task(); 
#endif
    
#ifdef MEM_REC_EX_FLASH
    Result |= eFlash_Task(); 
#endif //MEM_REC_EX_FLASH
    
    Result |= SD_Card_Task(); 
      
	return Result;
}



   
void Mem_Init(void)
{
    qQueue_Create (&qMemWrite, 20, sizeof (sMemQueueWrite), (sMemQueueWrite *) &sQMemWrite);  
    qQueue_Create (&qMemRead, 20, sizeof (sMemQueueRead), (sMemQueueRead *) &sQMemRead);    
    
    Mem_Init_Index_Record();
    
    sMemVar.Status_u8 = ERROR;
        
#ifdef MEM_REC_IN_FLASH
    sMemVar.Type_u8 = __INTERNAL_FLASH;
    OnFlash_Init();
     
    sOnFlash.pMem_Write_OK = Mem_Cb_Write_OK;
    sOnFlash.pMem_Read_OK = Mem_Cb_Read_OK;
#endif


#ifdef MEM_REC_EX_EEPROM
    sMemVar.Type_u8 = __EXTERNAL_EEPROM;
    CAT24Mxx_Init();
    sCAT24Var.pMem_Write_OK = Mem_Cb_Write_OK;
    sCAT24Var.pMem_Read_OK = Mem_Cb_Read_OK;
    sCAT24Var.pMem_Wrtie_ERROR = Mem_Cb_Wrtie_ERROR;
    sCAT24Var.pMem_Erase_Chip_OK = Mem_Cb_Erase_Chip_OK;
#endif

#ifdef MEM_REC_EX_FLASH
    sMemVar.Type_u8 = __EXTERNAL_FALSH;
    eFlash_Init();
    
    sS25FLvar.pMem_Write_OK = Mem_Cb_Write_OK;
    sS25FLvar.pMem_Read_OK  = Mem_Cb_Read_OK;   

    sS25FLvar.pMem_Wrtie_ERROR = Mem_Cb_Wrtie_ERROR;
    sS25FLvar.pMem_Erase_Chip_OK = Mem_Cb_Erase_Chip_OK;
    sS25FLvar.pMem_Wrtie_Retry  = Mem_Cb_Write_Retry;
#endif

    
    SD_Card_Init();
         
      
#ifdef USING_AT_CONFIG    
    sATCmdList[_ERASE_MEMORY].CallBack = Mem_SER_rErase_Memory;
    sATCmdList[_QUERY_LAST_FOR_LOG].CallBack = Mem_SER_Get_Last_For_LOG;
    sATCmdList[_QUERY_LAST_FOR_OPERA].CallBack = Mem_SER_Get_Last_For_OPERA;
    sATCmdList[_QUERY_LAST_FOR_EVENT].CallBack = Mem_SER_Get_Last_For_EVENT;
    sATCmdList[_QUERY_LAST_FOR_GPS].CallBack = Mem_SER_Get_Last_For_GPS;  
    sATCmdList[_QUERY_INDEX_LOG].CallBack = Mem_SER_Get_Index_Log;   
#endif    
}



/*--------- Cb event -----------------*/
static uint8_t _Cb_mem_Test_Write(uint8_t event)
{
    uint8_t result = false;
    
    result = Mem_Test_Write();
    
    if (result == true)
    {
        sMemVar.Status_u8 = true;
        sMemVar.cHardReset_u32 = 0;

        fevent_enable( sEventMem, _EVENT_MEM_CTRL_READ);   
        fevent_enable( sEventMem, _EVENT_MEM_CTRL_WRITE);
    } else if (result == false) {
        sMemVar.Status_u8 = ERROR;
        UTIL_Printf_Str( DBLEVEL_M, "u_mem: mem write error!\r\n" );
        //power again
        if (sMemVar.pRestart != NULL) {
            sMemVar.pRestart();
        }
    } else {
        fevent_enable( sEventMem, event);
    }

	return true;
}

/*
    them code de co the nhay vao lan luot tung ban tin
    
*/
static uint8_t _Cb_mem_Ctrl_Read (uint8_t event)
{      
    static uint16_t MAX_COUNT_PENDING = 0;
    static uint8_t cPending_u8 = 0, last_type = _MEM_DATA_TSVH;
    uint8_t kindData;
    uint16_t length = 0;
    
    if (sMemVar.Status_u8 == true)
    {            
        MAX_COUNT_PENDING = MAX_TIME_RETRY_READ / sEventMem[_EVENT_MEM_CTRL_READ].e_period;
        
        /*  + K co item nao trong queue set pending ve false   */
        if ( ( Mem_Queue_Read_Empty() == true) && (sMemVar.rPending_u8 == pending) )
        {
            if (cPending_u8++ >= MAX_COUNT_PENDING ) {
                cPending_u8 = 0;
                sMemVar.rPending_u8 = false;
                //xoa du lieu buff gui data qua sim
                if (sMemVar.pReset_Buff_Sim != NULL) {
                    sMemVar.pReset_Buff_Sim();
                }
                
                last_type = (last_type + 1) % _MEM_DATA_END;
            }  
        }
        
        switch (last_type) 
        {
            case _MEM_DATA_TSVH:
                //tsvh
                if ( (sRecTSVH.iSend_u16 != sRecTSVH.iSave_u16) && (sMemVar.rPending_u8 == false) ) {
                    UTIL_Printf_Str( DBLEVEL_M, "u_mem: read new rec tsvh...\r\n" );
                    kindData = _MEM_DATA_TSVH;
                    length = sRecTSVH.Size_u16;
                    sMemVar.rPending_u8 = true;
                }
                break;
            case _MEM_DATA_EVENT:
                //event
                if ( (sRecEvent.iSend_u16 != sRecEvent.iSave_u16) && (sMemVar.rPending_u8 == false) ) {
                    UTIL_Printf_Str( DBLEVEL_M, "u_mem: read new rec event...\r\n" );
                    kindData = _MEM_DATA_EVENT;
                    length = sRecEvent.Size_u16;
                    sMemVar.rPending_u8 = true;
                } 
                break;
            case _MEM_DATA_GPS:
                //gps
                if ( (sRecGPS.iSend_u16 != sRecGPS.iSave_u16) && (sMemVar.rPending_u8 == false) ) {
                    UTIL_Printf_Str( DBLEVEL_M, "u_mem: read new rec gps...\r\n" );
                    kindData = _MEM_DATA_GPS;
                    length = sRecGPS.Size_u16;
                    sMemVar.rPending_u8 = true;
                } 
            break;
        }
                
        if (sMemVar.rPending_u8 == true) {
            cPending_u8 = 0;
            if (Mem_Read_Data(sMemVar.Type_u8, kindData, _MEM_READ_NEW_MESS, 0, length) == true) {
                sMemVar.rPending_u8 = pending;
            } else {
                sMemVar.rPending_u8 = false;
            }
        } 
        
        if (sMemVar.rPending_u8 != pending) {
            last_type = (last_type + 1) % _MEM_DATA_END;
        }
    }
    
    fevent_enable( sEventMem, event);

    return 1;
}


static uint8_t _Cb_mem_Ctrl_Write (uint8_t event)
{
    sMemQueueWrite   sqWriteTemp;
    static uint16_t MAX_COUNT_PENDING = 0;
    static uint8_t cPending_u8 = 0;
    uint32_t addr = 0;
    
    if (sMemVar.Status_u8 == true)
    {
        MAX_COUNT_PENDING = MAX_TIME_RETRY_READ / sEventMem[_EVENT_MEM_CTRL_READ].e_period;
       
        if (sMemVar.wPending_u8 == true)
        {
            if (cPending_u8++ >= MAX_COUNT_PENDING ) {
                cPending_u8 = 0;
                sMemVar.wPending_u8 = false;
                //xoa du lieu buff gui data qua sim
                if (sMemVar.pReset_Buff_Sim != NULL) {
                    sMemVar.pReset_Buff_Sim();
                }
            }  
            
            fevent_enable( sEventMem, event);
        }
        
        if ( (sMemVar.wPending_u8 == false) 
            && (qGet_Number_Items (&qMemWrite) != 0) )
        {
            UTIL_Printf_Str( DBLEVEL_H, "u_mem: write new rec...\r\n");
            qQueue_Receive(&qMemWrite, (sMemQueueWrite *) &sqWriteTemp, 0);
            
            switch (sqWriteTemp.Type_u8)
            {
                case __EXTERNAL_EEPROM:
                #ifdef MEM_REC_EX_EEPROM
                    addr = Mem_Cacul_Addr_Mess(sqWriteTemp.TypeData_u8, __MEM_WRITE);
                    if (addr != 0xFFFFFFFF) { 
                        sCAT24Var.sHWrite.Addr_u32 = addr;
                    } else {
                        sCAT24Var.sHWrite.Addr_u32 = sqWriteTemp.Addr_u32;
                    }
                    
                    sCAT24Var.sHWrite.Length_u16 = sqWriteTemp.Length_u16;
                    UTIL_MEM_cpy(sCAT24Var.sHWrite.aData, sqWriteTemp.aData, sqWriteTemp.Length_u16);
                    fevent_active( sEventCAT24, _EVENT_CAT24Mxx_WRITE);
                #endif
                    break;
                case __INTERNAL_FLASH:
                #ifdef MEM_REC_IN_FLASH
                    addr = Mem_Cacul_Addr_Mess(sqWriteTemp.TypeData_u8, __MEM_WRITE);
                    if (addr != 0xFFFFFFFF) { 
                        sOnFlash.sHWrite.Addr_u32 = addr;
                    } else {
                        sOnFlash.sHWrite.Addr_u32 = sqWriteTemp.Addr_u32;
                    }
                    
                    sOnFlash.sHWrite.Length_u16 = sqWriteTemp.Length_u16;
                    UTIL_MEM_cpy(sOnFlash.sHWrite.aData, sqWriteTemp.aData, sqWriteTemp.Length_u16);
                    fevent_active( sEventOnChipFlash, _EVENT_ON_FLASH_WRITE_BUFF);
                #endif
                    break;
                case __EXTERNAL_FALSH:
                #ifdef MEM_REC_EX_FLASH
                    addr = Mem_Cacul_Addr_Mess(sqWriteTemp.TypeData_u8, __MEM_WRITE);
                    if (addr != 0xFFFFFFFF) { 
                        sS25FLvar.sHWrite.Addr_u32 = addr;
                    } else {
                        sS25FLvar.sHWrite.Addr_u32 = sqWriteTemp.Addr_u32;
                    }
                    
                    sS25FLvar.sHWrite.Length_u16 = sqWriteTemp.Length_u16;
                    UTIL_MEM_cpy(sS25FLvar.sHWrite.aData, sqWriteTemp.aData, sqWriteTemp.Length_u16);
                    fevent_active( sEventExFlash, _EVENT_FLASH_WRITE_BUFF);
                #endif
                    break;
                default:
                    break;
            }
            
            sMemVar.wPending_u8 = true;
            fevent_enable( sEventMem, event);
        }
    }
    
    return 1;
}


static uint8_t _Cb_mem_Ctrl_Request (uint8_t event)
{    
    uint32_t addr = 0;
    uint16_t length = 0;
    
    if (sMemVar.Status_u8 == true) 
    {
        if ( (sMemVar.sMemReq.Status_u8 == true) && (sMemVar.rPending_u8 == false) ) 
        {
            if (sMemVar.sMemReq.iStart_u16 != sMemVar.sMemReq.iStop_u16) {
                VLevelDebug = DBLEVEL_MESS;
                UTIL_ADV_TRACE_SetVerboseLevel(DBLEVEL_MESS);
            
                sMemVar.sMemReq.Landmark_u32 = RtCountSystick_u32;
                
                addr = sMemVar.sMemReq.pRecord->AddrStart_u32 + sMemVar.sMemReq.iStart_u16 * sMemVar.sMemReq.pRecord->Size_u16;
                length = sMemVar.sMemReq.pRecord->Size_u16;
                
                if (Mem_Read_Data(sMemVar.Type_u8, _MEM_DATA_OTHER, _MEM_READ_RESP_AT, addr, length) == true) {
                    sMemVar.rPending_u8 = pending;
                } else {
                    sMemVar.rPending_u8 = false;
                }
      
                //Tang index 
                if (sMemVar.sMemReq.iStart_u16++ >= sMemVar.sMemReq.pRecord->Max_u16) {
                    sMemVar.sMemReq.iStart_u16 = 0;
                }
            } else {
                //Ket thuc
                sMemVar.sMemReq.Status_u8 = false;

                VLevelDebug = DBLEVEL_M;
                UTIL_ADV_TRACE_SetVerboseLevel(DBLEVEL_M);
                
                return 1;
            }
        }
        
        //qua timeout ma k doc them ban tin -> ket thuc
        if (Check_Time_Out(sMemVar.sMemReq.Landmark_u32, 600000) == true) {
            sMemVar.sMemReq.iStart_u16 = sMemVar.sMemReq.iStop_u16;
        }
        
        sEventMem[event].e_period = sMemVar.sMemReq.Delay_u32;
        fevent_enable( sEventMem, event);
    }
    
    return 1;
}




static uint8_t _Cb_mem_Erase (uint8_t event)
{
#ifdef MEM_REC_EX_FLASH
    eFlash_Push_Step_Erase_Chip();
#endif
    
#ifdef MEM_REC_EX_EEPROM
    if (CAT24Mxx_Erase() == 0) {
        UTIL_Printf_Str( DBLEVEL_M, "u_mem: erase eeprom ok. reset mcu!\r\n" );
        Mem_Cb_Erase_Chip_OK();
    }
#endif
  
#ifdef MEM_REC_IN_FLASH
    UTIL_Printf_Str( DBLEVEL_M, "u_mem: erase ok. reset mcu!\r\n" );
    Mem_Cb_Erase_Chip_OK();
#endif

	return 1;
}


    
    
    
/*-------- function handle -----------*/

uint8_t Mem_Test_Write(void)
{
#ifdef MEM_REC_EX_EEPROM
    return true;
#endif
    
#ifdef MEM_REC_IN_FLASH
    return true;
#endif
    
#ifdef MEM_REC_EX_FLASH
    return eFlash_Test_Write();
#endif
}


uint8_t Mem_Queue_Write_Empty (void)
{
    if (qGet_Number_Items(&qMemWrite) == 0) {
        return true;
    }
    
    return false;
}

uint8_t Mem_Queue_Read_Empty (void)
{
    if (qGet_Number_Items(&qMemRead) == 0) {
        return true;
    }
    
    return false;
}





/*
    Func: App sim push data to write to extenal flash
        + Type Data: 0: TSVH ,1: Event  2: LOG
        + PartAorB: Xac dinh dia chi A hay B
        + pData
        + Length
*/
void Mem_Write_Data (uint8_t tmem, uint8_t tdata, uint32_t addr, 
                        uint8_t *pData, uint16_t Length, uint16_t MAX_LENGTH)
{
    sMemQueueWrite  qFlashTemp;
    uint16_t Count = 0, i = 0;
    uint8_t TempCrc = 0;
      
    qFlashTemp.Type_u8 = tmem;
    qFlashTemp.TypeData_u8 = tdata;
    qFlashTemp.Addr_u32 = addr;
    
    qFlashTemp.aData[Count++] = BYTE_TEMP_FIRST;
    qFlashTemp.aData[Count++] = (Length >> 8) & 0xFF;
    qFlashTemp.aData[Count++] = Length & 0xFF;
        
    for (i = 0; i < Length; i++)
        qFlashTemp.aData[Count++] =  pData[i];
    
    //Gioi han size luu vao
    if (Count > (MAX_LENGTH - 1) )
        Count = MAX_LENGTH - 1;
    
    // caculator crc
    Count++;
	for (i = 0; i < (Count - 1); i++)
		TempCrc ^= qFlashTemp.aData[i];

    qFlashTemp.aData[Count-1] = TempCrc;   
    qFlashTemp.Length_u16 = Count;
    
    //Send to queue fwrite
    Mem_Send_To_Queue_Write(&qFlashTemp);
}


uint8_t Mem_Read_Data (uint8_t tmem, uint8_t tdata, uint8_t kind, uint32_t addr, uint16_t length)
{
    uint8_t Result = false;

    switch (tmem)
    {
        case __EXTERNAL_EEPROM:
        #ifdef MEM_REC_EX_EEPROM
            if (Mem_Cacul_Addr_Mess(tdata, __MEM_READ) != 0xFFFFFFFF) {  
                sCAT24Var.sHRead.Addr_u32 = Mem_Cacul_Addr_Mess(tdata, __MEM_READ);
            } else {
                sCAT24Var.sHRead.Addr_u32 = addr;
            }
            
            sCAT24Var.sHRead.Length_u16 = length;
            sCAT24Var.sHRead.Kind_u8 = kind;
            
            fevent_active( sEventCAT24, _EVENT_CAT24Mxx_READ);
            Result = true;
        #endif
            break;
        case __INTERNAL_FLASH:
        #ifdef MEM_REC_IN_FLASH
            //lay addr, length: 
            if (Mem_Cacul_Addr_Mess(tdata, __MEM_READ) != 0xFFFFFFFF) {   
                sOnFlash.sHRead.Addr_u32 = Mem_Cacul_Addr_Mess(tdata, __MEM_READ);
            } else {
                sOnFlash.sHRead.Addr_u32 = addr;
            }
            
            sOnFlash.sHRead.Length_u16 = length;
            sOnFlash.sHRead.Kind_u8 = kind;
            
            fevent_active( sEventOnChipFlash, _EVENT_ON_FLASH_READ_BUFF);
            Result = true;
        #endif
            break;
        case __EXTERNAL_FALSH:
        #ifdef MEM_REC_EX_FLASH
            if (Mem_Cacul_Addr_Mess(tdata, __MEM_READ) != 0xFFFFFFFF) {   
                sS25FLvar.sHRead.Addr_u32 = Mem_Cacul_Addr_Mess(tdata, __MEM_READ);
            } else {
                sS25FLvar.sHRead.Addr_u32 = addr;
            }
            
            sS25FLvar.sHRead.Length_u16 = length;
            sS25FLvar.sHRead.Kind_u8 = kind;
            
            fevent_active( sEventExFlash, _EVENT_FLASH_READ_BUFF);
            Result = true;
        #endif
            break;
        default: 
            break;
    }
    
    return Result;
}




/*
    Func: tinh dia chi luu record theo loai ban tin
        - type: TSVH A,B  ; LOG ....
        - RW: dia chi cho viec doc hay la ghi
*/
uint32_t Mem_Cacul_Addr_Mess (uint8_t type, uint8_t RW)
{
    uint32_t addr = 0;
    sMemRecordInfor *pRec;
    
    switch (type)   
    {
        case _MEM_DATA_TSVH: 
            pRec = &sRecTSVH;
            break;
        case _MEM_DATA_EVENT:
            pRec = &sRecEvent;
            break; 
        case _MEM_DATA_LOG:
            pRec = &sRecLog;
            break; 
        case _MEM_DATA_GPS:
            pRec = &sRecGPS;
            break; 
        case _MEM_DATA_OTHER:
            addr = 0xFFFFFFFF;
            break; 
        default:
            break;
    }
    
    if (type != _MEM_DATA_OTHER) {
        if (RW == __MEM_READ) {
            addr = pRec->AddrStart_u32 + pRec->iSend_u16 * pRec->Size_u16; 
        } else {
            addr = pRec->AddrStart_u32 + pRec->iSave_u16 * pRec->Size_u16;
        }
    }
    
    return addr;
}




//void AppMem_Send_To_Queue_Read (sQueueMemRead *qRead)
//{
//    qQueue_Send(&qMemRead, (sQueueMemRead *) qRead, _TYPE_SEND_TO_END); 
//}


void Mem_Send_To_Queue_Write (sMemQueueWrite *qWrite)
{
    //check mem ok hay loi
    if (sMemVar.Status_u8 == true) {
        qQueue_Send(&qMemWrite, (sMemQueueWrite *) qWrite, _TYPE_SEND_TO_END); 
        fevent_active( sEventMem, _EVENT_MEM_CTRL_WRITE);
    } else {
        if (sMemVar.pForward_Mess != NULL) {
            sMemVar.pForward_Mess(qWrite->TypeData_u8, qWrite->aData, qWrite->Length_u16);
        }
    }
}


//func init
/*
    Func: Init Record Index
        + Index Send, index save
*/
void Mem_Init_Index_Record (void)
{
    uint8_t TempU8 = 0;
    uint8_t aTEMP[24] = {0};
    uint16_t TempU16 = 0;
    
#ifdef BOARD_QN_V5_0
    //Doc vi tri send va luu ra
    TempU8 = *(__IO uint8_t*) ADDR_INDEX_REC;
    
	if (TempU8 != FLASH_BYTE_EMPTY)
	{
        OnchipFlashReadData ((ADDR_INDEX_REC + 2), aTEMP, 24);   
        //TSVH send
        TempU16 = (aTEMP[0] << 8) | aTEMP[1];
        sRecTSVH.iSend_u16 = MIN(TempU16, sRecTSVH.Max_u16);
        
        //TSVH Save
        TempU16 = (aTEMP[2] << 8) | aTEMP[3];        
        sRecTSVH.iSave_u16 = MIN(TempU16, sRecTSVH.Max_u16);
        
        //Event send
        TempU16 = (aTEMP[4] << 8) | aTEMP[5];
        sRecEvent.iSend_u16 = MIN(TempU16, sRecEvent.Max_u16);

        //Event Save
        TempU16 = (aTEMP[6] << 8) | aTEMP[7];
        sRecEvent.iSave_u16 = MIN(TempU16, sRecEvent.Max_u16);
        
        //GPS send
        TempU16 = (aTEMP[8] << 8) | aTEMP[9];
        sRecGPS.iSend_u16 = MIN(TempU16, sRecGPS.Max_u16);

        //GPS Save
        TempU16 = (aTEMP[10] << 8) | aTEMP[11];
        sRecGPS.iSave_u16 = MIN(TempU16, sRecGPS.Max_u16);
        
        //log send
        TempU16 = (aTEMP[12] << 8) | aTEMP[13];
        sRecLog.iSend_u16 = MIN(TempU16, sRecLog.Max_u16);
        
        //log Save
        TempU16 = (aTEMP[14] << 8) | aTEMP[15];
        sRecLog.iSave_u16 = MIN(TempU16, sRecLog.Max_u16);
    } else
    {
    	Mem_Save_Index_Rec();
    }
#endif

    
#ifdef BOARD_LC_V1_1
    if (CAT24Mxx_Read_Array(CAT_ADDR_INDEX_REC, aTEMP, 24) == true) {
        //doc 64 byte tu eeprom
        TempU8 = aTEMP[0];
        //Check Byte EMPTY
        if (TempU8 == BYTE_WRITEN) {
            //TSVH send
            TempU16 = (aTEMP[2] << 8) | aTEMP[3];
            sRecTSVH.iSend_u16 = MIN(TempU16, sRecTSVH.Max_u16);
            
            //TSVH Save
            TempU16 = (aTEMP[4] << 8) | aTEMP[5];        
            sRecTSVH.iSave_u16 = MIN(TempU16, sRecTSVH.Max_u16);
            
            //Event send
            TempU16 = (aTEMP[6] << 8) | aTEMP[7];
            sRecEvent.iSend_u16 = MIN(TempU16, sRecEvent.Max_u16);

            //Event Save
            TempU16 = (aTEMP[8] << 8) | aTEMP[9];
            sRecEvent.iSave_u16 = MIN(TempU16, sRecEvent.Max_u16);
            
            //GPS send
            TempU16 = (aTEMP[10] << 8) | aTEMP[11];
            sRecGPS.iSend_u16 = MIN(TempU16, sRecGPS.Max_u16);

            //GPS Save
            TempU16 = (aTEMP[12] << 8) | aTEMP[13];
            sRecGPS.iSave_u16 = MIN(TempU16, sRecGPS.Max_u16);
            
            //log send
            TempU16 = (aTEMP[14] << 8) | aTEMP[15];
            sRecLog.iSend_u16 = MIN(TempU16, sRecLog.Max_u16);
            
            //log Save
            TempU16 = (aTEMP[16] << 8) | aTEMP[17];
            sRecLog.iSave_u16 = MIN(TempU16, sRecLog.Max_u16); 
        }
    }
#endif
}


void Mem_Save_Index_Rec (void)
{
    uint8_t aTEMP[24] = {0};
    
    aTEMP[0] = BYTE_TEMP_FIRST;
    aTEMP[1] = 12;

    aTEMP[2] = (sRecTSVH.iSend_u16 >> 8) & 0xFF;
    aTEMP[3] = sRecTSVH.iSend_u16 & 0xFF;
    
    aTEMP[4] = (sRecTSVH.iSave_u16 >> 8) & 0xFF;
    aTEMP[5] = sRecTSVH.iSave_u16 & 0xFF;
    
    aTEMP[6] = (sRecEvent.iSend_u16 >> 8) & 0xFF;
    aTEMP[7] = sRecEvent.iSend_u16 & 0xFF;
    
    aTEMP[8] = (sRecEvent.iSave_u16 >> 8) & 0xFF;
    aTEMP[9] = sRecEvent.iSave_u16 & 0xFF;
    
    aTEMP[10] = (sRecGPS.iSend_u16 >> 8) & 0xFF;
    aTEMP[11] = sRecGPS.iSend_u16 & 0xFF;
    
    aTEMP[12] = (sRecGPS.iSave_u16 >> 8) & 0xFF;
    aTEMP[13] = sRecGPS.iSave_u16 & 0xFF;
    
    aTEMP[14] = (sRecLog.iSend_u16 >> 8) & 0xFF;
    aTEMP[15] = sRecLog.iSend_u16 & 0xFF;
    
    aTEMP[16] = (sRecLog.iSave_u16 >> 8) & 0xFF;
    aTEMP[17] = sRecLog.iSave_u16 & 0xFF;

#ifdef BOARD_QN_V5_0
    Erase_Firmware(ADDR_INDEX_REC, 1);
    OnchipFlashWriteData(ADDR_INDEX_REC, &aTEMP[0], 24);
#endif
    
#ifdef BOARD_LC_V1_1
    CAT24Mxx_Write_Buff(CAT_ADDR_INDEX_REC, aTEMP, 24); 
#endif
}
    

/*
    Func: Call back external flash write
        + Increase Index and addr: Active Event Check New Mess Modem
*/
void Mem_Cb_Write_OK (uint32_t addr)
{
    qQueue_Receive(&qMemWrite, NULL, 1);
    fevent_active( sEventMem, _EVENT_MEM_CTRL_WRITE);
    fevent_active( sEventMem, _EVENT_MEM_CTRL_READ);
    sMemVar.wPending_u8 = false;
        
    switch ( Mem_Get_Type_From_Addr(addr) )
    {
        case _MEM_DATA_TSVH:
            Mem_Inc_Index_Save(&sRecTSVH);
            break;
        case _MEM_DATA_EVENT:
            Mem_Inc_Index_Save(&sRecEvent);
            break;
        case _MEM_DATA_LOG:
            Mem_Inc_Index_Save(&sRecLog);
            break;
        case _MEM_DATA_GPS:
            Mem_Inc_Index_Save(&sRecGPS);
            break;
        default:
            break;
    }
}

/*
    Func: Check Mess read from flash invalid
        + Increase Index send
        + Or MEM_REC_EX_FLASH: Push data to write to part B
    +note: Neu trong queue ton tai nhieu read: sau do invalid thi
            Can check xem bang indexsave truoc khi++
*/
void Mem_Handle_Invalid (uint8_t tdata)
{
    switch (tdata)
    {
        case _MEM_DATA_TSVH:
            Mem_Inc_Index_Send(&sRecTSVH, 1);
            break;
        case _MEM_DATA_EVENT:
            Mem_Inc_Index_Send(&sRecEvent, 1);
            break;
        case _MEM_DATA_GPS:
            Mem_Inc_Index_Send(&sRecGPS, 1);
            break;
        default:
            break;
    }
}




/*
    Func: Call back external flash read
        + Push Record to qFlashSim and active event Check new mess
*/

void Mem_Cb_Read_OK (uint8_t Kind, uint32_t Addr, uint8_t *pData, uint16_t length)
{
    uint16_t reallen = 0, i = 0;  
    uint8_t tdata = Mem_Get_Type_From_Addr(Addr);
    sMemRecordInfor *pRecord;   
    uint8_t TempCrc = 0, IsValidData = true;
    
    qQueue_Receive(&qMemRead, NULL, 1);
    reallen = (*(pData + 1) << 8) + *(pData + 2);

    switch (tdata)
    {
        case _MEM_DATA_TSVH:
            pRecord = &sRecTSVH;
            break;
        case _MEM_DATA_EVENT:
            pRecord = &sRecEvent;
            break;
        case _MEM_DATA_GPS:
            pRecord = &sRecGPS;
            break;
        default:
            break;
    }

    //Check Crc
    if (reallen < pRecord->Size_u16)
    {
        for (i = 0; i < (reallen + 3); i++) {
            TempCrc ^= *(pData + i);
        }
        
        if ( TempCrc != *(pData + reallen + 3) ) {
            IsValidData = false;
        }
    } else {
        IsValidData = false;
    }
    
    //Copy sang buff App sim Data
    if (Kind == _MEM_READ_NEW_MESS) {
        if (IsValidData == true) {
            UTIL_Printf_Str( DBLEVEL_M, "u_mem: data read from memory:\r\n" );
            UTIL_Printf_Hex( DBLEVEL_H, pData + 3, reallen);
            UTIL_Printf_Str( DBLEVEL_H, "\r\n" );
            
            if (sMemVar.pForward_Mess != NULL) {
                sMemVar.pForward_Mess(tdata, pData + 3, reallen);
            }
        } else {
            UTIL_Log_Str( DBLEVEL_M, "u_mem: invalid data read!\r\n" );
            Mem_Handle_Invalid(tdata);
        }    
    } else {
        if (sMemVar.pRespond_Req != NULL) {
            sMemVar.pRespond_Req(tdata, pData + 3, reallen);
        }
        sMemVar.rPending_u8 = false;
    }  
}



/*
    Func: tinh dia chi luu record theo loai ban tin
        - addr: 
*/
uint8_t Mem_Get_Type_From_Addr (uint32_t addr)
{
    uint8_t type = 0;
    
    if ( (sRecTSVH.AddrStart_u32 <= addr) && (addr < sRecTSVH.AddrStop_u32) ) {
        type = _MEM_DATA_TSVH;
    } else if ( (sRecEvent.AddrStart_u32 <= addr) && (addr < sRecEvent.AddrStop_u32) ) {
        type = _MEM_DATA_EVENT;
    } else if ( (sRecGPS.AddrStart_u32 <= addr) && (addr < sRecGPS.AddrStop_u32) ) {
        type = _MEM_DATA_GPS;
    } else if ( (sRecLog.AddrStart_u32 <= addr) && (addr < sRecLog.AddrStop_u32) ) {
        type = _MEM_DATA_LOG;
    }
       
    return type;
}



uint8_t Mem_Inc_Index_Save (sMemRecordInfor *sRec)
{
    sRec->iSave_u16 = (sRec->iSave_u16 + 1) % sRec->Max_u16;
    if (sRec->iSave_u16 == sRec->iSend_u16) {
        sRec->iSend_u16 = (sRec->iSend_u16 + 1) % sRec->Max_u16;
    }
    
    Mem_Save_Index_Rec(); 
    
    return true;
}




uint8_t Mem_Inc_Index_Send (sMemRecordInfor *sRec, uint8_t num)
{       
    sMemVar.rPending_u8 = false;
    
//    uint8_t aTemp[50] = "iSend_u16: ";   //13 ki tu dau tien
//    sData StrResp = {&aTemp[0], 11}; 
    
    if (sRec->iSend_u16 != sRec->iSave_u16) {
        sRec->iSend_u16 = (sRec->iSend_u16 + num) % sRec->Max_u16;
        Mem_Save_Index_Rec(); 
        
//        Convert_Uint64_To_StringDec (&StrResp, (uint64_t) (sRec->iSend_u16), 0);
//        Insert_String_To_String(StrResp.Data_a8, &StrResp.Length_u16, (uint8_t*)" end\r\n",0 , 6);
//        Modem_Respond(_UART_DEBUG, StrResp.Data_a8, StrResp.Length_u16, 0);
    }
    
    return true;
}
         

/*
    Func: Cb write fail: Do chua dc xoa vung nho. Active Check Pos Init
*/
void Mem_Cb_Wrtie_ERROR (uint32_t Addr, uint8_t *pdata, uint16_t length)
{
    uint8_t tdata = Mem_Get_Type_From_Addr(Addr);
    
    UTIL_Printf_Str( DBLEVEL_M, "u_mem: write error!\r\n");
    qQueue_Receive(&qMemWrite, NULL, 1);
    fevent_active( sEventMem, _EVENT_MEM_CTRL_WRITE);
    sMemVar.wPending_u8 = false;
    
    //Forward packet to sim
    if (sMemVar.pForward_Mess != NULL) {
        sMemVar.pForward_Mess(tdata, pdata + 3, length - 4);
    }
    //increase index
    switch ( tdata )
    {
        case _MEM_DATA_TSVH:
            Mem_Inc_Index_Save(&sRecTSVH);
            break;
        case _MEM_DATA_EVENT:
            Mem_Inc_Index_Save(&sRecEvent);
            break;
        case _MEM_DATA_LOG:
            Mem_Inc_Index_Save(&sRecLog);
            break;
        case _MEM_DATA_GPS:
            Mem_Inc_Index_Save(&sRecGPS);
            break;
        default:
            break;
    }

    if (sMemVar.pRestart != NULL) {
        sMemVar.pRestart();
    }
}


/*
    Func: Cb write fail: Do chua dc xoa vung nho. Active Check Pos Init
*/
void Mem_Cb_Erase_Chip_OK(void)
{
#ifdef INFOR_EX_EEPROM
    __disable_irq();
    NVIC_SystemReset(); // Reset MCU
#endif
    
#ifdef INFOR_ON_FLASH
    sRecTSVH.iSend_u16 = 0;
    sRecTSVH.iSave_u16 = 0;
    sRecEvent.iSend_u16 = 0;
    sRecEvent.iSave_u16 = 0;
    sRecLog.iSend_u16 = 0;
    sRecLog.iSave_u16 = 0;
    sRecGPS.iSend_u16 = 0;
    sRecGPS.iSave_u16 = 0;
    
    Mem_Save_Index_Rec();
    //Reset MCU
    if (sMemVar.pReset_MCU != NULL) {
        sMemVar.pReset_MCU();
    }
#endif
    
#ifdef INFOR_EX_FLASH
    __disable_irq();
    NVIC_SystemReset(); 
#endif
}


/*
    Func: Kiem tra xem con record can gui di k
*/

uint8_t Mem_Is_New_Record (void)
{
    if (sMemVar.Status_u8 == true) {
        if ( ( sRecTSVH.iSend_u16 != sRecTSVH.iSave_u16 ) 
            || ( sRecEvent.iSend_u16 != sRecEvent.iSave_u16 ) )
            return true;
    }
    
    return false;
}


void Mem_Cb_Write_Retry(void)
{
    sMemVar.wPending_u8 = false;
}




#ifdef USING_AT_CONFIG
void Mem_SER_rErase_Memory (sData *strRecei, uint16_t Pos)
{
    fevent_enable( sEventMem, _EVENT_MEM_ERASE);
    sMemVar.pRespond_Str(PortConfig, "PENDING...", 0);
}


void Mem_SER_Get_Last_For_LOG (sData *strRecei, uint16_t Pos)
{        
    Mem_Get_Req_AT(_MEM_DATA_LOG, (uint16_t) UTIL_Get_Num_From_Str(strRecei, &Pos));
}

void Mem_SER_Get_Last_For_OPERA (sData *strRecei, uint16_t Pos)
{
    Mem_Get_Req_AT(_MEM_DATA_TSVH, (uint16_t) UTIL_Get_Num_From_Str(strRecei, &Pos));
}

void Mem_SER_Get_Last_For_EVENT (sData *strRecei, uint16_t Pos)
{
    Mem_Get_Req_AT(_MEM_DATA_EVENT, (uint16_t) UTIL_Get_Num_From_Str(strRecei, &Pos));
}

void Mem_SER_Get_Last_For_GPS (sData *strRecei, uint16_t Pos)
{
    Mem_Get_Req_AT(_MEM_DATA_GPS, (uint16_t) UTIL_Get_Num_From_Str(strRecei, &Pos));
}

void Mem_SER_Get_Index_Log (sData *strRecei, uint16_t Pos)
{    
    char aData[128] = {0};
    
    sprintf(aData, "tsvh: %d, %d, %d\r\n", sRecTSVH.iSend_u16, sRecTSVH.iSave_u16, sRecTSVH.Max_u16);
    sprintf(aData + strlen(aData), "event: %d, %d, %d\r\n", sRecEvent.iSend_u16, sRecEvent.iSave_u16, sRecEvent.Max_u16);
    sprintf(aData + strlen(aData), "log: %d, %d, %d\r\n", sRecLog.iSend_u16, sRecLog.iSave_u16, sRecLog.Max_u16);
    
    sMemVar.pRespond_Str(PortConfig, aData, 0);
}

/*
    Func: Set Infor index read parameter:
        + Read n rec
*/

uint8_t Mem_Get_Req_AT (uint8_t type, uint16_t NumRec)
{   
    if ( ( NumRec != 0) && (sMemVar.Status_u8 == true) )
    {
        switch (type)
        {
            case _MEM_DATA_TSVH:
                sMemVar.sMemReq.pRecord = &sRecTSVH;
                break;
            case _MEM_DATA_EVENT:
                sMemVar.sMemReq.pRecord = &sRecEvent;
                break;
            case _MEM_DATA_LOG:
                sMemVar.sMemReq.pRecord = &sRecLog;
                break;
            case _MEM_DATA_GPS:
                sMemVar.sMemReq.pRecord = &sRecGPS;
                break;
            default:
                break;
        }
        
        if (NumRec < sMemVar.sMemReq.pRecord->Max_u16)
        {
            sMemVar.sMemReq.Landmark_u32 = RtCountSystick_u32;
            sMemVar.sMemReq.Status_u8 = true;
            sMemVar.sMemReq.Port_u8 = PortConfig;
            sMemVar.sMemReq.Type_u8 = type; 
            sMemVar.sMemReq.iStop_u16 = sMemVar.sMemReq.pRecord->iSave_u16;
                
            if (sMemVar.sMemReq.iStop_u16 >= NumRec) {
                sMemVar.sMemReq.iStart_u16 = sMemVar.sMemReq.iStop_u16 - NumRec;
            } else {
                sMemVar.sMemReq.iStart_u16 = (sMemVar.sMemReq.pRecord->Max_u16 + sMemVar.sMemReq.iStop_u16 - NumRec);
            }
            
            //active event check read resp at
            fevent_active( sEventMem, _EVENT_MEM_CTRL_REQ);
            if (PortConfig == _UART_DEBUG) {
                sMemVar.sMemReq.Delay_u32 = 100;
            } else {
                sMemVar.sMemReq.Delay_u32 = 2000;
            }
            
            UTIL_Printf_Str( DBLEVEL_M, "\r\nOK\r\n" );
            
            return 1;
        }
    }
    
    UTIL_Printf_Str( DBLEVEL_M, "u_app_mem: get request at error!\r\n" );
    
    return 0;
}


#endif






