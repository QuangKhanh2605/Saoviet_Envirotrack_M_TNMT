/*
 * myUart.c
 *
 *  Created on: Dec 7, 2021
 *      Author: lenovo
 */

#include "user_uart.h"
#include "user_sim.h"
/*==================var struct======================*/

/*==================Function======================*/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == uart_sim.Instance)
    {
        Sim_Rx_Callback(1);     
    } 

    if (huart->Instance == uart_debug.Instance)
    {
        AppComm_DBG_Rx_Callback(1);
    }
    
    if (huart->Instance == uart_rs485.Instance)
    {
        RS485_Rx_Callback(1);
    }
    
    if (huart->Instance == uart_rs485_2.Instance)
    {
        RS485_2_Rx_Callback(1);
    }
}


void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart->Instance == uart_debug.Instance)
    {
        AppComm_DBG_Rx_Callback(Size);
    }
    
    if (huart->Instance == uart_sim.Instance)
    {
        Sim_Rx_Callback(Size);
    }
    
    if (huart->Instance == uart_rs485.Instance)
    {
        RS485_Rx_Callback(Size);
    }
    
    if (huart->Instance == uart_rs485_2.Instance)
    {
        RS485_2_Rx_Callback(Size);
    }
}




