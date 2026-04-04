

#ifndef CAT24MXX_H
#define CAT24MXX_H _CAT24MXX_H

#include "user_util.h"

#include "event_driven.h"
#include "queue_p.h"
#include "user_mem_comm.h"
#include "i2c.h"


/*============== Define ====================*/
#define USING_EX_EEPROM

//#define CAT24_128KBIT
#define CAT24_256KBIT

#define I2C_CAT24                   hi2c1

#define CAT24_START_LOG             0

#ifdef CAT24_128KBIT
#define CAT24_END_LOG               16384 /* Cat24 128kbs  byte */  
#endif // CAT24_128KBIT

#ifdef CAT24_256KBIT
#define CAT24_END_LOG               32768 
#endif // CAT24_256KBIT

#define CAT_BYTE_EMPTY              0xFF

#define NUMBER_RETRY_BUSY           20
#define TIME_RETRY_BUSY             10
#define NUMBER_RETRY_R_W            20

/* =============== Drivers ============== */
#define I2C_CAT24Mxx_ADDR_7BIT          0xA0        /* Don't care operation bit (R/W bit) - 1010A1A2A30b */
#define I2C_CAT24Mxx_MAX_BUFF           64          /* Bytes */

/*=========== Define =================*/
#ifdef CAT24_128KBIT

#define CAT_SIZE_DATA_TSVH	            128        
#define CAT_SIZE_DATA_EVENT	            64         
#define CAT_SIZE_DATA_LOG	            64   
#define CAT_SIZE_DATA_GPS	            64   

#define	CAT_MAX_MESS_TSVH_SAVE			18
#define	CAT_MAX_MESS_EVENT_SAVE			10
#define CAT_MAX_MESS_LOG_SAVE           130 
#define CAT_MAX_MESS_GPS_SAVE           1  

/*Addr Record*/
#define	CAT_ADDR_MESS_A_START   		64   
#define	CAT_ADDR_MESS_A_STOP			7232

#define	CAT_ADDR_EVENT_START   		    7296   
#define	CAT_ADDR_EVENT_STOP			    7936

#define	CAT_ADDR_LOG_START   		    8000    
#define	CAT_ADDR_LOG_STOP			    16384

#define	CAT_ADDR_GPS_START   		    0      
#define	CAT_ADDR_GPS_STOP			    768
#endif // #ifdef CAT24_128KBIT




#ifdef CAT24_256KBIT

#define CAT_SIZE_DATA_TSVH	            128        
#define CAT_SIZE_DATA_EVENT	            64         
#define CAT_SIZE_DATA_LOG	            64   
#define CAT_SIZE_DATA_GPS	            64   

#define	CAT_MAX_MESS_TSVH_SAVE			160
#define	CAT_MAX_MESS_EVENT_SAVE			10
#define CAT_MAX_MESS_LOG_SAVE           100 
#define CAT_MAX_MESS_GPS_SAVE           1  

/*Addr Record*/
#define	CAT_ADDR_INDEX_REC 		        64         //64 byte
#define	CAT_ADDR_METER_NUMBER			128        //256 byte
#define	CAT_ADDR_MODBUS_INFOR			384        //64 byte 
#define	CAT_ADDR_WM_CONFIG			    448        //1024 byte   -> 1472  
#define	CAT_ADDR_TNMT_CONFIG			1536       //2048 byte   -> 1536 -> 3574  

#define	CAT_ADDR_MESS_A_START   		3584   
#define	CAT_ADDR_MESS_A_STOP			24832    //182 * 128 = 23296 ->160

#define	CAT_ADDR_EVENT_START   		    24832   
#define	CAT_ADDR_EVENT_STOP			    25600   //12 * 64 = 768

#define	CAT_ADDR_LOG_START   		    25600    
#define	CAT_ADDR_LOG_STOP			    32640   //110 * 64 = 5760

#define	CAT_ADDR_GPS_START   		    32640   //2 * 64 = 128  
#define	CAT_ADDR_GPS_STOP			    32768

#endif // #ifdef CAT24_256KBIT



/*========== Var struct ===============*/

typedef enum
{
	_EVENT_CAT24Mxx_WRITE = 0,
	_EVENT_CAT24Mxx_READ,
	_EVENT_CAT24Mxx_END, // don't use
}eEVENT_CAT24;


/* ===================================== Struct =========================================== */
typedef struct
{
    uint8_t status_u8;
    uint8_t Count_err_u8;
    uint8_t Count_retry_u8;
}sFROM_Manager_Struct;


extern sFROM_Manager_Struct	sFROM_Manager;
//
extern sEvent_struct        sEventCAT24[];
extern sMemoryVariable      sCAT24Var;


/* ===================================== Function =========================================== */
uint8_t CAT24Mxx_Write_Array (uint16_t writeAddr, uint8_t* dataBuff, uint16_t dataLength);
uint8_t CAT24Mxx_Read_Array (uint16_t readAddr, uint8_t* dataBuff, uint16_t dataLength);

uint8_t CAT24Mxx_Write_Buff (uint16_t addr, uint8_t *pdata, uint16_t length);

int32_t TestEEPROM(void); /* Test EEPROM */
uint8_t CAT24Mxx_Erase (void);
uint8_t CAT24Mxx_Write_Byte (uint16_t writeAddr, uint8_t *dataBuff);
uint8_t CAT24Mxx_Write_Word (uint16_t writeAddr, uint16_t *dataBuff);
uint8_t CAT24Mxx_Write_DoubleWord (uint16_t writeAddr, uint32_t *dataBuff);
uint8_t CAT24Mxx_Write_QuadWord (uint16_t writeAddr, uint64_t *dataBuff);
uint8_t CAT24Mxx_Read_Byte (uint16_t writeAddr, uint8_t *dataBuff);
uint8_t CAT24Mxx_Read_Word (uint16_t writeAddr, uint16_t *dataBuff);
uint8_t CAT24Mxx_Read_DoubleWord (uint16_t writeAddr, uint32_t *dataBuff);
uint8_t CAT24Mxx_Read_QuadWord (uint16_t writeAddr, uint64_t *dataBuff);
uint8_t CAT24Mxx_Check_Busy(uint8_t  addrA16bit, uint8_t Retry, uint8_t Timeout);
//*================ Func ============================*/

void    CAT24Mxx_Init (void);
uint8_t CAT24Mxx_Task (void);



#endif
