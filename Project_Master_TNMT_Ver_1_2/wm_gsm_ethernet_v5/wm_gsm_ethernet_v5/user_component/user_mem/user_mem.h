
#ifndef USER_MEM_H_
#define USER_MEM_H_

#include "user_util.h"
#include "user_internal_mem.h"
#include "user_mem_comm.h"
#include "cat24mxx.h"
#include "user_sd.h"
#include "S25FL216K.h"

#include "user_at_serial.h"

/*=== Memory using save record ===*/
//#define MEM_REC_EX_EEPROM  
//#define MEM_REC_IN_FLASH    
#define MEM_REC_EX_FLASH    

#define MAX_TIME_RETRY_READ     10000   
#define MAX_BYTE_CUT_GPS        18    
#define MEM_MAX_TIMEOUT_READ    24*3600    //1 ngay


/*======== Struct var ===========*/

typedef enum
{
    _EVENT_MEM_TEST_WRITE,
    _EVENT_MEM_ERASE,
    
    _EVENT_MEM_CTRL_READ,  
    _EVENT_MEM_CTRL_WRITE,   
    _EVENT_MEM_CTRL_REQ,    
    
	_EVENT_END_MEM,
}eMemEvent;


typedef enum
{
    _MEM_DATA_TSVH,
    _MEM_DATA_EVENT,
    _MEM_DATA_LOG,
    _MEM_DATA_GPS,
    _MEM_DATA_OTHER,
    _MEM_DATA_END,
}eMemDataType;



typedef enum
{
    _MEM_READ_NEW_MESS,
    _MEM_READ_RESP_AT,
}sMemKindRead;

typedef enum
{
    __EXTERNAL_EEPROM,
    __INTERNAL_FLASH,
    __EXTERNAL_FALSH,
}eMemType;


typedef struct
{  
    uint32_t AddrStart_u32;
    uint32_t AddrStop_u32;

    uint16_t Size_u16;          //Size record
    uint16_t Max_u16;           //Max record Save in Flash

    uint16_t iSend_u16;         //vi tri doc ra va gui
    uint16_t iSave_u16;         //vi tri luu tiep theo  
    
    uint8_t  cReaded_u8;
}sMemRecordInfor;



/* queue write to mem information
    - aData : chua du lieu
    - Addr
    - Length data
*/
typedef struct
{
    uint8_t         aData[MAX_MEM_DATA];
    uint32_t        Addr_u32;
    uint16_t        Length_u16;             //Length data
    uint8_t         TypeData_u8;
    uint8_t         Type_u8;
}sMemQueueWrite;


typedef struct
{
    uint32_t        Addr_u32;        //Address
    uint16_t        Length_u16;      //Length data
    uint8_t         TypeData_u8;
    uint8_t         Type_u8;
}sMemQueueRead;


typedef struct
{
    uint8_t     Status_u8;
    uint8_t     Port_u8;
    uint8_t     Type_u8;
    uint16_t    iStart_u16;
    uint16_t    iStop_u16;
    uint32_t    Delay_u32;
    uint32_t    Landmark_u32;
    sMemRecordInfor  *pRecord;               //Record infor request
}sMemRequest;


//
typedef struct
{
    uint8_t     Onoff_u8;
    uint8_t     Status_u8;
    uint8_t     Type_u8;

    uint8_t     wPending_u8;
    uint8_t     rPending_u8;
    
    uint32_t    cHardReset_u32;
    uint8_t     iInit_u8;
    
    sMemRequest sMemReq;
    
    uint8_t     ( *pReset_MCU ) (void);
    void        ( *pRespond_Str) (uint8_t portNo, const char *str, uint8_t ack);
    void        ( *pRestart) (void);
    void        ( *pReset_Buff_Sim) (void);
    void        ( *pForward_Mess) (uint8_t tdata, uint8_t *pdata, uint16_t length);
    void        ( *pRespond_Req) (uint8_t tdata, uint8_t *pdata, uint16_t length);
}sMemVariable;


extern sEvent_struct sEventMem []; 

extern sMemRecordInfor     sRecTSVH;
extern sMemRecordInfor     sRecEvent;
extern sMemRecordInfor     sRecLog;
extern sMemRecordInfor     sRecGPS;

extern sMemVariable sMemVar;




/*================ Func =================*/

void    Mem_Init(void);
uint8_t Mem_Task(void);

uint8_t Mem_Test_Write(void);
uint8_t Mem_Queue_Write_Empty (void);
uint8_t Mem_Queue_Read_Empty (void);


uint8_t Mem_Read_Data (uint8_t tmem, uint8_t tdata, uint8_t kind, uint32_t addr, uint16_t length);
uint32_t Mem_Cacul_Addr_Mess (uint8_t type, uint8_t RW);
void    Mem_Write_Data (uint8_t tmem, uint8_t tdata, uint32_t addr, 
                        uint8_t *pData, uint16_t Length, uint16_t MAX_LENGTH);

void    Mem_Send_To_Queue_Write (sMemQueueWrite *qWrite);
void    Mem_Forward_Mess (uint32_t Addr, uint8_t *pData, uint16_t Length);

void    Mem_Init_Index_Record (void);
void    Mem_Save_Index_Rec (void);

uint8_t Mem_Inc_Index_Save (sMemRecordInfor *sRec);
uint8_t Mem_Inc_Index_Send (sMemRecordInfor *sRec, uint8_t num);

uint8_t Mem_Get_Type_From_Addr (uint32_t addr);
void    Mem_Cb_Write_OK (uint32_t addr);
void    Mem_Cb_Read_OK (uint8_t Kind, uint32_t Addr, uint8_t *pData, uint16_t length);
void    Mem_Cb_Erase_Chip_OK(void);
void    Mem_Cb_Wrtie_ERROR (uint32_t Addr, uint8_t *pdata, uint16_t length);

void    Mem_Handle_Invalid (uint8_t tdata);
uint8_t Mem_Is_New_Record (void);
void    Mem_Cb_Write_Retry(void);

#ifdef USING_AT_CONFIG
void Mem_SER_rErase_Memory (sData *strRecei, uint16_t Pos);
void Mem_SER_Get_Last_For_LOG (sData *strRecei, uint16_t Pos);
void Mem_SER_Get_Last_For_OPERA (sData *strRecei, uint16_t Pos);
void Mem_SER_Get_Last_For_EVENT (sData *strRecei, uint16_t Pos);
void Mem_SER_Get_Last_For_GPS (sData *strRecei, uint16_t Pos);
void Mem_SER_Get_Index_Log (sData *strRecei, uint16_t Pos);
uint8_t Mem_Get_Req_AT (uint8_t type, uint16_t NumRec);

#endif

#endif






