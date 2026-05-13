


#ifndef _USER_LPTIM_H
#define _USER_LPTIM_H 	

#include "user_util.h"
#include "lptim.h"
#include "tim.h"

/*================Define=====================*/

//#ifndef LPTIM_ENCODER_MODE
//	#define LPTIM_ENCODER_MODE
//#endif

//#ifndef LPTIM2_ENCODER_MODE
//	#define LPTIM2_ENCODER_MODE
//#endif

/*================Var struct=====================*/
typedef enum
{
    _DIRECT_FORWARD = 1,
    _DIRECT_REVERSE,
    _DIRECT_END,
}Kind_Direct_WM;


typedef struct
{
	int64_t	    Number_i64;             //counter pulse
    int64_t	    NumberInit_i64;         //init counter pulse
    uint8_t     Direct_u8;              //0: quay thuan, 1: quay nguoc
    
    float	    Flow_f;                 //luu luong    
    float	    Quantity_f;             //San luong 
 
    uint8_t     IsOver5Pulse_u8;        //Da chay qua 5 xung chua
    
    double      Total_lf;              //Chi so thuc te
    double      nPositive_lf;          //chieu thuan
    double      nNegative_lf;          //chieu nguoc
    double      Start_lf;              //Chi so ban dau
    
    int16_t     FactorInt_i16;          //He so xung phan nguyen  
    uint8_t     FactorDec_u8;           //He so xung:  
    
    uint8_t     Status_u8;              //trang thai hoạt dong
}Struct_Pulse;



/*================Extern var struct=====================*/
extern void (*pLPTIM_Func_Direct_Change_Handler) (uint8_t Type);


/*================Function=====================*/
void        LPTIM_Encoder_Get_Pulse (LPTIM_HandleTypeDef *hlptim, Struct_Pulse *sPulse);

uint16_t    LPTIM_Check_Pulse_Period (uint16_t Last_Pulse , uint16_t First_Pulse);
void        LPTIM_Check_DIR_PIN (LPTIM_HandleTypeDef *hlptim, Struct_Pulse *sPulse);

void        LPTIM_Init_Feature (void);
void        LPTIM_Counter_Get_Pulse (LPTIM_HandleTypeDef *hlptim, Struct_Pulse *sPulse);

void        TIM_Counter_Get_Pulse (TIM_HandleTypeDef *htim, Struct_Pulse *sPulse);

#endif
