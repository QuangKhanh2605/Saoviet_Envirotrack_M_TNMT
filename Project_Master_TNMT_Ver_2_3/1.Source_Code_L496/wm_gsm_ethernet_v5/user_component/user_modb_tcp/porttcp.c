/*
* FreeModbus Libary: lwIP Port
* Copyright (C) 2006 Christian Walter <wolti@sil.at>
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*
* File: $Id: porttcp.c,v 1.1 2006/08/30 23:18:07 wolti Exp $
*/

/* ----------------------- System includes ----------------------------------*/
#include <stdio.h>
#include <string.h>
#include "port.h"
#include "w6100.h"
#include "socket.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

#include "user_util.h"

/* ----------------------- MBAP Header --------------------------------------*/
#define MB_TCP_UID          6
#define MB_TCP_LEN          4
#define MB_TCP_FUNC         7
#define HTTP_SOCKET         0

/* ----------------------- Prototypes ---------------------------------------*/

uint8_t ucTCPRequestFrame [300]; // 
uint16_t ucTCPRequestLen;
uint8_t ucTCPResponseFrame [300]; //
uint16_t ucTCPResponseLen;
uint8_t bFrameSent = FALSE;

BOOL 
xMBTCPPortInit(USHORT usTCPPort)
{
    BOOL bOkay = FALSE;

#ifdef USING_ETHERNET
    USHORT usPort;
    if (usTCPPort == 0)
    {
        usPort = MB_TCP_DEFAULT_PORT;
    }
    else
    {
        usPort = (USHORT)usTCPPort;
    }
    UTIL_Printf_Str( DBLEVEL_L, "Creating socket...\r\n");
    uint8_t http_socket = HTTP_SOCKET;
    uint8_t code = socket(http_socket, Sn_MR_TCP, usPort, 0);
    if (code != http_socket)
    {
        UTIL_Printf_Str( DBLEVEL_L, "socket() failed\r\n");
        return FALSE;
    }
    listen(HTTP_SOCKET);
#endif // USING_ETHERNET
    
    bOkay = TRUE;
    return bOkay;
}

void vMBTCPPortClose()
{
}

void vMBTCPPortDisable(void)
{
}

BOOL 
xMBTCPPortGetRequest(UCHAR **ppucMBTCPFrame, USHORT *usTCPLength)
{
    *ppucMBTCPFrame = (uint8_t *) &ucTCPRequestFrame[0];
    *usTCPLength = ucTCPRequestLen;
    /* Reset the buffer. */
    ucTCPRequestLen = 0;
    return TRUE;
}

BOOL 
xMBTCPPortSendResponse(const UCHAR *pucMBTCPFrame, USHORT usTCPLength)
{
    memcpy(ucTCPResponseFrame, pucMBTCPFrame, usTCPLength);
    ucTCPResponseLen = usTCPLength;
    bFrameSent = TRUE; 
    return bFrameSent;
}

uint8_t mbrtu_status = false;
#include "user_message.h"


uint8_t modbus_tcps(void) 
{
    uint8_t result = false;
    static uint8_t step = 0;
    static uint32_t landmark = 0;
    
    switch (step)
    {
        case 0:
            xMBPortEventPost(EV_FRAME_RECEIVED);          
            eMBPoll();  
            if (eMBPoll() == MB_ENOERR) {
                mbrtu_status = pending;
            } else {
                mbrtu_status = true;
            }
            step++;
            landmark = RtCountSystick_u32;
            break;
        case 1:
            //cho xu ly mod rtu
            if (Check_Time_Out(landmark, 5000) == true) {
                step++;
            } else if (mbrtu_status == true) {    
                eMBPoll();   
                if (bFrameSent) {
                    bFrameSent = FALSE;
//                    AppSo_Send_485(ucTCPResponseFrame, ucTCPResponseLen);
                    //gui lai copy vao buff cua intan va gui len
                    sMessage.aMESS_PENDING[DATA_INTAN] = TRUE;
                }
                step++;
            } else if (mbrtu_status == false) {
                step++;
            }
            break;
        default:
            step = 0;
            result = true;
            break;
    }
    
    return result;
}


uint8_t ucTCP_TEMP [300]; 
uint8_t ethernet_pending = false;

void modbus_tcps_2 (uint8_t sn, uint16_t port) 
{
    static uint8_t step = 0;
    static uint32_t landmark = 0;
    
    switch (getSn_SR(sn)) 
    {
        case SOCK_CLOSED:                    
            socket(sn, Sn_MR_TCP, port, 0x00); 
            break;
        case SOCK_INIT:        
            listen(sn);    
            break;
        case SOCK_ESTABLISHED: 
//            if (getSn_IR(sn) & Sn_IR_CON)
//            {
//                setSn_IR(sn, Sn_IR_CON);
//            }
//            ucTCPRequestLen = getSn_RX_RSR(sn); 
//            
//            if (ucTCPRequestLen > 0) {
//                if (ethernet_pending == false) {
//                    ethernet_pending = true;
//                    recv(sn, ucTCPRequestFrame, ucTCPRequestLen); 
//                    UTIL_Printf(DBLEVEL_M, ucTCPRequestFrame, ucTCPRequestLen);
//                    step = 0;
//                } else {
//                    recv(sn, ucTCP_TEMP, ucTCPRequestLen); 
//                }
//            }
//                 
//            if (ethernet_pending == true) {
//                switch (step)
//                {
//                    case 0:
//                        xMBPortEventPost(EV_FRAME_RECEIVED);          
//                        eMBPoll();  
//                        if (eMBPoll() == MB_ENOERR) {
//                            mbrtu_status = pending;
//                        } else {
//                            mbrtu_status = true;
//                        }
//                        step++;
//                        landmark = RtCountSystick_u32;
//                        break;
//                    case 1:
//                        //cho xu ly mod rtu
//                        if (Check_Time_Out(landmark, 5000) == true) {
//                            step++;
//                        } else if (mbrtu_status == true) {    
//                            eMBPoll();   
//                            if (bFrameSent) {
//                                bFrameSent = FALSE;
//                                send(sn, ucTCPResponseFrame, ucTCPResponseLen);
//                            }
//                            step++;
//                        } else if (mbrtu_status == false) {
//                            step++;
//                        }
//                        break;
//                    default:
//                        step = 0;
//                        ethernet_pending = false;
//                        break;
//                }
//            }
            break;
        case SOCK_CLOSE_WAIT: 
            disconnect(sn);     
            break;
        default:
            break;
    }
}









