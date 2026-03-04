





#ifndef USER_SD_H_
#define USER_SD_H_

#define USING_APP_SD_CARD


#include "user_util.h"
#include "event_driven.h"
#include "user_mem_comm.h"


/*======== Struct var ===========*/

typedef enum
{
    _EVENT_SD_TEST,
	_EVENT_SD_WRITE,
	_EVENT_SD_READ,
    
    _EVENT_SD_CHECK,
    
	_EVENT_SD_END, 
}eEVENT_SD;


extern sEvent_struct sEventSD[];
extern uint32_t      SD_Free_u32;



/*================ Func =================*/
void SD_Card_Init (void);

void Init_AppSDcard (void);
uint8_t SD_Card_Task (void);

uint8_t SD_Check(void);

#endif





