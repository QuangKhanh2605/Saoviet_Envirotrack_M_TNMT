/*
    8/2021
    Thu vien xu ly Uart
*/
#ifndef USER_AT_SERIAL_H
#define USER_AT_SERIAL_H

#include "user_util.h"
#include "usart.h"

/*======================== Define ======================*/

#define USING_AT_CONFIG

#define LENGTH_AT_SERIAL    128
/*======================== Structs var======================*/
typedef enum
{
	_SET_DEV_SERIAL,
	_QUERY_DEV_SERIAL,
    
    _QUERY_SIM_ID,
    
	_SET_SERVER_MAIN,
	_QUERY_SERVER_MAIN,
    
    _SET_SERVER_BACKUP,
    _QUERY_SERVER_BACKUP,
    
    _REQ_SWITCH_SERVER,
    
    _SET_DUTY_READ_DATA,
    _QUERY_DUTY_READ_DATA,
    
    _SET_FREQ_ONLINE,
    _QUERY_FREQ_ONLINE,
    
    _SET_DEFAULT_PULSE,
    _SET_SAVE_BOX,
    _SET_NUM_WAKEUP,
    _READ_OLD_DATA,
    
    _SET_FLOW_THRESH,
    _QUERY_FLOW_THRESH,
    
    _SET_QUAN_THRESH,
    _QUERY_QUAN_THRESH,
    
    _SET_BAT_THRESH,
    _QUERY_BAT_THRESH,
    
    _SET_PULSE_VALUE,
    _QUERY_PULSE_VALUE,
    
    _SET_PULSE_FACTOR,
    _QUERY_PULSE_FACTOR,
    
    _SET_PULSE_START,
    _QUERY_PULSE_START,

    _SET_RTC,
    _QUERY_RTC,
    
    _SET_LEVEL_DEBUG,
    _QUERY_LEVEL_DEBUG,
    
    _RESET_MODEM,
    _ERASE_MEMORY,
    _QUERY_FIRM_VER,
    
    _QUERY_UP_FIRM_HTTP,
    _QUERY_UP_FIRM_FTP,
    
    _QUERY_INDEX_LOG,
    _TEST_LOG,
    _QUERY_READ_ALL_LOG,
    _QUERY_READ_LAST_LOG,
    _QUERY_LAST_FOR_LOG,
    
    _QUERY_LAST_FOR_OPERA,
    _QUERY_LAST_FOR_EVENT,
    
    _SET_CALIB_LEVEL,
    _QUERY_CALIB_LEVEL,
    
    _SET_LEVEL_THRESH,
    _QUERY_LEVEL_THRESH,
    
    _QUERY_GPS_LOC,
    _QUERY_LAST_FOR_GPS,
    
    _SET_LIST_ID_SLAVE,
    _QUERY_LIST_ID_SLAVE,
    
    _SET_EX_BAT_THRESH,
    _QUERY_EX_BAT_THRESH,
    
    _SET_POWER_ON_TEMH,
    _SET_POWER_OFF_TEMH,
    
    _SET_NUM_CHANNEL,
    _QUERY_NUM_CHANNEL,
    
    _SET_MODBUS_INFOR,
    _QUERY_MODBUS_INFOR,
    
    _SET_PRESS_INFOR,
    _QUERY_PRESS_INFOR,
    
    _SET_LEVEL_INFOR,
    _QUERY_LEVEL_INFOR,
    
    _QUERY_PRESS_VAL,
    _QUERY_PRESS_CALIB,
    _QUERY_PRESS_CAL_VAL,
    
    _SET_MODEM_RESET,
    _QUERY_MODEM_RESET,
    
    _SET_TIME_ALARM,
    _QUERY_TIME_ALARM,
    
    _SET_TNMT_INFOR,
    _QUERY_TNMT_INFOR,
    
    _SET_TNMT_PACK_M,
    _QUERY_TNMT_PACK_M,
    
    _SET_FTP_SER_MAIN,
    _QUERY_FTP_SER_MAIN,
    
    _GET_STATE_SENSOR,
    _GET_MEASURE_VALUE,
    _GET_MEASURE_FILTER,
    
    _GET_FREQ_WARNING_SENSOR,
    _SET_FREQ_WARNING_SENSOR,

    _GET_USER_SENSOR,
    _SET_USER_SENSOR,
    
    _GET_OFFSET_SENSOR,
    _SET_OFFSET_SENSOR,
    
    _GET_STATE_CALIB_SS,
    _SET_STATE_CALIB_SS,
    
    _GET_SD_CARD_FREE,
    
    _END_AT_CMD,
}Type_Command_AT_PC;


typedef void (*_func_callback_f)(sData *str_Receiv, uint16_t Pos);

typedef struct {
	int 			    idStep;
	_func_callback_f	CallBack;
	sData	            sCmd;	
} sATCommandList;


/*======================== External Var struct ======================*/
extern sATCommandList sATCmdList[];
extern uint8_t PortConfig;

/*======================== Function ======================*/


#endif
