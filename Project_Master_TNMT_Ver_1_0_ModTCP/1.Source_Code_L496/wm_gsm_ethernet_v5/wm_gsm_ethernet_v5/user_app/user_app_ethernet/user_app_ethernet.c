
#include "user_app_ethernet.h"
#include "socket.h"

#include "user_modem.h"

#include "user_string.h"
#include "spi.h"


/*================== Static func ===================*/
static uint8_t wizchip_read(void);
static void wizchip_write(uint8_t wb);
static void wizchip_read_buff(uint8_t* buf, datasize_t len);
static void wizchip_write_buff(uint8_t* buf, datasize_t len);

static void wizchip_deselect(void);
static void wizchip_select(void);
static void wizchip_network_infor(void);


static uint8_t _Cb_Check_Bus (uint8_t event);
static uint8_t _Cb_Hard_Reset (uint8_t event);

static uint8_t _Cb_Check_PHY_LINK (uint8_t event);
static uint8_t _Cb_Running_DHCP (uint8_t event);
static uint8_t _Cb_DNS_Process (uint8_t event);

static uint8_t _Cb_Socket_Control (uint8_t event);

static uint8_t _Cb_Send_Ping (uint8_t event);
static uint8_t _Cb_Send_Data (uint8_t event);

static uint8_t _Cb_Send (uint8_t event);
static uint8_t _Cb_Send_OK (uint8_t event);
static uint8_t _Cb_Send_Timeout (uint8_t event);
static uint8_t _Cb_Recv_Data (uint8_t event);

static uint8_t _Cb_Control_Sim (uint8_t event);
static uint8_t _Cb_INTn_Process (uint8_t event);
static uint8_t _Cb_Power_Off (uint8_t event);

// Func callback cmd
static uint8_t _Cb_Pasv_Port_OK (sData *pData);
static uint8_t _Cb_Get_OK (sData *pData);
static uint8_t _Cb_List_OK (sData *pData);
static uint8_t _Cb_Get_Size_OK (sData *pData); 
static uint8_t _Cb_URC_Ready (sData *pData);
static uint8_t _Cb_Success_Def (sData *pData);
static uint8_t _Cb_Mqtt_Sub_OK (sData *pData);
static uint8_t _Cb_Mqtt_Pub_OK (sData *pData);

static uint8_t _fMQTT_Connect (void);
static uint8_t _fMQTT_Subcribe (void);
static uint8_t _fMQTT_Publish (void);

static uint8_t _fSet_User (void);
static uint8_t _fSet_Pass (void);
static uint8_t _fSet_Pasv_Port (void);
static uint8_t _fSet_Get_File (void);
static uint8_t _fSet_Type_ASCII (void);
static uint8_t _fSet_Type_BIN (void);
static uint8_t _fSet_CWD (void);
static uint8_t _fQuery_List (void);
static uint8_t _fQuery_Size_File (void);

//cmd recv
static uint8_t _Cb_URC_Server (sData *pData);
/*===================== Struct var ===================== */

uint8_t gDATABUF[DATA_BUF_SIZE];
uint8_t aRECEIVE_DATA[DATA_BUF_SIZE];
uint16_t LengthRecv = 0;

/* Network Configuration default */
wiz_NetInfo gWIZNETINFO = { .mac = {0x00,0x08,0xdc,0x12,0x34,0x56},
							.ip = {192,168,177,120},
							.sn = {255, 255, 255, 0},
							.gw = {192, 168, 177, 1},
							.dns = {168, 126, 63, 1},
							
							.lla={0xfe,0x80,0x00,0x00,
									0x00,0x00, 0x00,0x00,
									0x02,0x08, 0xdc,0xff,
									0xfe,0x57, 0x12,0x34},   ///< Source Link Local Address
							.gua={0x00,0x00,0x00,0x00,
									0x00,0x00,0x00,0x00,
									0x02,0x08,0xdc,0xff,
									0xfe,0x57,0x12,0x34},   ///< Source Global Unicast Address
							.sn6={0xff,0xff,0xff,0xff,
									0xff,0xff,0xff,0xff,
									0x00,0x00,0x00, 0x00,
									0x00,0x00,0x00,0x00 },   ///< IPv6 Prefix
							.gw6={0xfe, 0x80, 0x00,0x00,
									0x00,0x00,0x00,0x00,
									0x02,0x00, 0x87,0xff,
									0xfe,0x08, 0x4c,0x81},   ///< Gateway IPv6 Address

                            .ipmode = NETINFO_DHCP_ALL,
};

uint8_t DHCPipver = AS_IPV4;
uint8_t ip_ver = AS_IPV4;
//uint8_t ip_ver = AS_IPV6;
//uint8_t ip_ver = AS_IPDUAL;


uint8_t dns_server[4] = {168, 126, 63, 1};           // Secondary DNS server IP
uint8_t dns_server_ip6[16] = {0x20,0x01,0x48,0x60,
								0x48,0x60,0x00,0x00,
								0x00,0x00,0x00,0x00,
								0x00,0x00,0x88,0x88
							 };

/*===================== Extern struct var ===================== */

sEvent_struct sEventAppEth[] =
{    
    { _EVENT_ETH_CONTROL_SIM, 	    1, 0, 1000, 		_Cb_Control_Sim },
	{ _EVENT_ETH_HARD_RESET, 		1, 0, 1000, 	    _Cb_Hard_Reset },           
	{ _EVENT_ETH_CHECK_BUS, 	    0, 0, 1000, 		_Cb_Check_Bus }, 
	{ _EVENT_ETH_PHY_LINK, 		    0, 0, 1000, 		_Cb_Check_PHY_LINK },
    
	{ _EVENT_ETH_DNS_PROCESS, 		0, 0, 50, 			_Cb_DNS_Process },
    { _EVENT_ETH_RUN_DHCP, 		    0, 0, 0, 			_Cb_Running_DHCP },   //Must set period = 0
    { _EVENT_ETH_SOCK_CTRL, 	    0, 0, 10, 			_Cb_Socket_Control },
    
    { _EVENT_ETH_SEND_PING, 		0, 0, 30000, 		_Cb_Send_Ping },    
    { _EVENT_ETH_SEND_DATA, 		0, 0, 1000, 		_Cb_Send_Data },     //Quet send data lien tuc

    { _EVENT_ETH_SEND, 		        0, 0, 1000, 	    _Cb_Send },
    { _EVENT_ETH_SEND_OK, 		    0, 0, 1000, 	    _Cb_Send_OK },
    { _EVENT_ETH_SEND_TIMEOUT, 		0, 0, 5000, 		_Cb_Send_Timeout },
    { _EVENT_ETH_RECV_DATA, 		0, 0, 0, 		    _Cb_Recv_Data },
    
    { _EVENT_ETH_INTn_PROCESS, 		0, 0, 1, 			_Cb_INTn_Process },
    
    { _EVENT_ETH_POWER_OFF, 		0, 0, 0, 			_Cb_Power_Off }, 
};

sEthernetVariable sAppEthVar = 
{
    .cSoftReset_u16 = 0xFF,
};

//cmd
uint8_t aETH_MQTT_CONN[2] = 
{
	_ETH_MQTT_CONN,
    _ETH_MQTT_SUB,
}; 

uint8_t aETH_FTP_CONN[7] = 
{
	_ETH_FTP_SET_USER,
    _ETH_FTP_SET_PASS,
    _ETH_FTP_SET_TYPE_BIN,
    _ETH_FTP_SET_DIR,
    _ETH_FTP_SET_PASV_PORT,
    _ETH_FTP_QUER_SIZE,
    _ETH_FTP_SET_GET_FILE,
}; 


static uint8_t ConACK[4] = {MY_CONNACK, 0x02, 0x00, 0x00};
static uint8_t SubACK[6] = {MY_SUBACK, 0x04, 0x00, 0x0a, 0x00, 0x00};
static uint8_t PubACK[4] = {MY_PUBACK, 0x02, 0x00, 0x01};
static uint8_t PingResp[2] = {MY_PINGRESP, 0x00};


sCommandEth     sEthCmd[] = 
{
    {   _ETH_MQTT_CONN,             _fMQTT_Connect,     _Cb_Success_Def,    (char *) ConACK     },  
    {   _ETH_MQTT_SUB,              _fMQTT_Subcribe,    _Cb_Mqtt_Sub_OK,    (char *) SubACK     },
    {   _ETH_MQTT_PUB_NACK,         _fMQTT_Publish,     _Cb_Mqtt_Pub_OK,    NULL                },
    {   _ETH_MQTT_PUB_ACK,          _fMQTT_Publish,     _Cb_Mqtt_Pub_OK,    (char *) PubACK     },
    {   _ETH_MQTT_PING,             _fMQTT_Publish,     _Cb_Mqtt_Pub_OK,    (char *) PingResp   },
    
    {   _ETH_FTP_SET_USER,          _fSet_User,         _Cb_Success_Def,    "331"     },
    {   _ETH_FTP_SET_PASS,          _fSet_Pass,         _Cb_Success_Def,    "230"     },
    
    {   _ETH_FTP_SET_TYPE_ASCII,    _fSet_Type_ASCII,   _Cb_Success_Def,    "200"     },
	{   _ETH_FTP_SET_TYPE_BIN,      _fSet_Type_BIN,     _Cb_Success_Def,    "200"     },
    
	{   _ETH_FTP_QUER_SER_DIR,      _fQuery_List,       _Cb_List_OK,        "150"     },
    
    {   _ETH_FTP_QUER_SIZE,         _fQuery_Size_File,  _Cb_Get_Size_OK,    "213"     },
    
    {   _ETH_FTP_SET_DIR,           _fSet_CWD,          _Cb_Success_Def,    "250"     },
    {   _ETH_FTP_SET_PASV_PORT,     _fSet_Pasv_Port,    _Cb_Pasv_Port_OK,   "227"     },
    {   _ETH_FTP_SET_GET_FILE,      _fSet_Get_File,     _Cb_Get_OK,         "150"     },  // 150
};

sCommandEth sEthResCode[] = 
{
    {   _ETH_MQTT_URC_SERVER,       NULL,               _Cb_URC_Server,     "AMI/"    },
    
    {   _ETH_FTP_URC_READY,         NULL,               _Cb_URC_Ready,      "220"    },
    {   _ETH_FTP_URC_CLOSING,       NULL,               NULL,               "226"    },
};


static uint8_t aQEthStepControl[ETH_MAX_CMD_QUEUE];
Struct_Queue_Type qEthStep;

static uint8_t Pending_u8 = false;
static uint32_t LandMarkSend_u32 = 0;

sEthFTPvariable sFTPvar;

/*========================Function ===============================*/
//Func Callback Event
static uint8_t _Cb_Hard_Reset (uint8_t event)
{
    uint8_t i = 0;
    static uint16_t cLastHardReset_u16 = 0;
    
    //Disable all event > PHY
    for (i = _EVENT_ETH_HARD_RESET + 1; i < _EVENT_APP_ETH_END; i++) {
        fevent_disable(sEventAppEth, i);
    }
    
    if (UTIL_var.ModePower_u8 == _POWER_MODE_SAVE)
    {
        if ( (abs(cLastHardReset_u16 - sAppEthVar.cHardReset_u16) >= 2) 
        #ifdef USING_APP_SIM
            || ( (sSimCommVar.ServerReady_u8 == true) && (Mem_Is_New_Record() != true) )
        #endif
              ) {
            cLastHardReset_u16 = sAppEthVar.cHardReset_u16;
            //power off
            fevent_active(sEventAppEth, _EVENT_ETH_POWER_OFF);
            
            return 1;
        }
        
        if (AppEth_W5500_Hard_Reset() == true) {
            UTIL_Printf_Str ( DBLEVEL_M, "u_app_eth: w5500 start!\r\n" );
            
            sAppEthVar.cHardReset_u16++;
            AppEth_Init_Default();
            fevent_active(sEventAppEth, _EVENT_ETH_CHECK_BUS);
        } else {
            fevent_enable(sEventAppEth, event);
        }
    } else {
        //Cho thiet bi reset cach nhau 10 phut
        sAppEthVar.cSoftReset_u16++;
        if (sAppEthVar.cSoftReset_u16 <= 5) {  //120
            AppEth_Init_Default();
            fevent_active(sEventAppEth, _EVENT_ETH_CHECK_BUS);
            return 1;
        }

        if (AppEth_W5500_Hard_Reset() == true) {
            UTIL_Printf_Str ( DBLEVEL_M, "u_app_eth: w5500 start!\r\n" );
            
            sAppEthVar.cSoftReset_u16 = 0;
            sAppEthVar.cHardReset_u16++;
            AppEth_Init_Default();
            fevent_active(sEventAppEth, _EVENT_ETH_CHECK_BUS);
        } else {
            fevent_enable(sEventAppEth, event);
        }
    }

	return 1;
}


static uint8_t _Cb_Check_Bus (uint8_t event)
{
    static uint8_t RetryBus = 0;
    
    UTIL_Printf_Str ( DBLEVEL_H, "u_app_eth: test bus spi\r\n" );
    APP_LOG(TS_OFF, DBLEVEL_H, "u_app_eth: version w5500: %d\r\n", getVER()); 
    
    sAppEthVar.Status_u8 = _ETH_CHECK_SPI;
    
    if ( (getVER() == 0x4661) && (getVER() == 0x4661) && (getVER() == 0x4661) ) { 
        AppEth_W5500_Init();
        sAppEthVar.PHYstatus_u8 = false;
        fevent_active(sEventAppEth, _EVENT_ETH_PHY_LINK);
        RetryBus = 0;
    } else {
        if (RetryBus++ >= ETH_MAX_RETRY_BUS) {
            RetryBus = 0;
            fevent_active(sEventAppEth, _EVENT_ETH_HARD_RESET);
        } else {
            fevent_enable(sEventAppEth, event);
        }
    }
    
	return 1;
}

/*
    Func: check Phy link
        + 1s chay vao check 1 lan
        + doc lien tiep 5 lan -> neu fail lien tiep 5 lan -> PHY fail
        + 
*/
static uint8_t _Cb_Check_PHY_LINK (uint8_t event)
{
    static uint8_t CountFalse = 0;
    uint8_t i = 0;

    if ( AppEth_Get_PHYStatus() == false ) {
        if (CountFalse++ >= ETH_MAX_FAIL_PHY) {
            CountFalse = 0; 
            
            sAppEthVar.cHardReset_u16++;
            sAppEthVar.Status_u8 = _ETH_CHECK_PHY;
            
            UTIL_Printf_Str ( DBLEVEL_M, "u_app_eth: phy link remove\r\n" );
            
            sAppEthVar.PHYstatus_u8 = false;

            //Disable all event > PHY
            for (i = _EVENT_ETH_PHY_LINK + 1; i < _EVENT_APP_ETH_END; i++) {
                fevent_disable(sEventAppEth, i);
            }
            
            fevent_active(sEventAppEth, _EVENT_ETH_HARD_RESET); 
        }
    } else {
        CountFalse = 0;
        
        if (sAppEthVar.PHYstatus_u8 == false)
        {
            UTIL_Printf_Str ( DBLEVEL_M, "u_app_eth: phy link become ready\r\n" );
             
            sAppEthVar.Status_u8 = _ETH_CHECK_PHY;
            //doi mode connect neu co req
            if (UTIL_var.ModeConnFuture_u8 != 0) {
                if ((UTIL_var.ModeConnNow_u8 == _CONNECT_DATA_MAIN) 
                    || (UTIL_var.ModeConnNow_u8 == _CONNECT_DATA_BACKUP)) {
                    UTIL_var.ModeConnLast_u8 = UTIL_var.ModeConnNow_u8;    
                } else if (UTIL_var.ModeConnNow_u8 == _CONNECT_FTP_UPLOAD) {
                    UTIL_var.ModeConnLast_u8 = _CONNECT_DATA_MAIN;
                }
                
                UTIL_var.ModeConnNow_u8 = UTIL_var.ModeConnFuture_u8;
                UTIL_var.ModeConnFuture_u8 = 0; 
            }
            
            Modem_Set_Server_Infor_To_App();
            
            //Start DHCP or DNS
            if ( (UTIL_var.ModeConnNow_u8 == _CONNECT_DATA_MAIN)
                    || (UTIL_var.ModeConnNow_u8  == _CONNECT_DATA_BACKUP) 
                    || (UTIL_var.ModeConnNow_u8  == _CONNECT_FTP_UPLOAD) ) {
                      
                if (*sAppEthVar.sServer.DomainOrIp_u8== __SERVER_DOMAIN) {
                    fevent_active(sEventAppEth, _EVENT_ETH_DNS_PROCESS);
                } else {
                    if (DHCPipver == AS_IPV4) {
                        DHCPv4_stop();  
                    } else {
                        DHCP_stop();
                    }                
                    fevent_active(sEventAppEth, _EVENT_ETH_RUN_DHCP);
                }
                //reset retry update
                sFTPvar.Retry_u8 = 0;
            } else {      
                if (*sAppEthVar.sUpdateVar.sServer.DomainOrIp_u8== __SERVER_DOMAIN) {
                    fevent_active(sEventAppEth, _EVENT_ETH_DNS_PROCESS);
                } else {
                    if (DHCPipver == AS_IPV4) {
                        DHCPv4_stop();  
                    } else {
                        DHCP_stop();
                    }
                    fevent_active(sEventAppEth, _EVENT_ETH_RUN_DHCP);
                }
            }
        }
        
        sAppEthVar.PHYstatus_u8 = true;
        
        //check timeout running
        if (Check_Time_Out (sAppEthVar.LandMarkRun_u32, 120000) == true) {
            fevent_active(sEventAppEth, _EVENT_ETH_HARD_RESET);
            return 1;
        }
    }
        
    fevent_enable(sEventAppEth, event);

    return 1;
}



static uint8_t _Cb_DNS_Process (uint8_t event)
{
	static uint8_t step = 0;
	int32_t ret = -1;
    uint8_t *pIP;
    uint8_t *sIP;
    char aTEMP[64] = {0};
    static uint32_t LandMark_u32 = 0;
    
    sAppEthVar.Status_u8 = _ETH_DNS_RUNNING;
    sAppEthVar.LandMarkRun_u32 = RtCountSystick_u32;
    
    if ( (UTIL_var.ModeConnNow_u8 == _CONNECT_DATA_MAIN)
            || (UTIL_var.ModeConnNow_u8  == _CONNECT_DATA_BACKUP)
            || (UTIL_var.ModeConnNow_u8  == _CONNECT_FTP_UPLOAD) ) {
        sIP = (uint8_t *) sAppEthVar.sServer.sIP;
        pIP = sAppEthVar.sServer.aIP;
    } else {
        sIP = (uint8_t *) sAppEthVar.sUpdateVar.sServer.sIP;
        pIP = sAppEthVar.sUpdateVar.sServer.aIP;
    }
    
    
	switch (step)
	{
		case 0:
			DNS_init(gDATABUF);  // SOCK_DNS
            LandMark_u32 = RtCountSystick_u32;
            step++;
			break;
		default:
            if (ip_ver == AS_IPV4) {
                ret = DNS_run (SOCK_DNS, dns_server, sIP, pIP, AS_IPV4);
            } else {
                ret = DNS_run(SOCK_DNS, dns_server_ip6, sIP, pIP, AS_IPV6);
            }
            
            if (ret == 1) {
                sprintf (aTEMP, "u_app_eth: DNS Success: %d.%d.%d.%d\r\n", pIP[0], pIP[1], pIP[2], pIP[3]);
                step = 0;
                fevent_active(sEventAppEth, _EVENT_ETH_RUN_DHCP);
            }
			break;
	}
    
    //check timeout
    if (Check_Time_Out (LandMark_u32, 10000) == true) {
        step = 0;
        fevent_active(sEventAppEth, _EVENT_ETH_CHECK_BUS);
        return 1;
    }

	fevent_enable(sEventAppEth, event);

	return 1;
}


static uint8_t _Cb_Running_DHCP (uint8_t event)
{
    uint8_t result = 0;
	static uint8_t step = 0;
    static uint8_t Retry = 0, cfail = 0, cIP = 0;

    sAppEthVar.Status_u8 = _ETH_DHCP_RUNNING;
    sAppEthVar.LandMarkRun_u32 = RtCountSystick_u32;
    //Tat event duoi
    fevent_disable(sEventAppEth, _EVENT_ETH_SOCK_CTRL);
    
	switch (step)
	{
		case 0:
            if (Retry++ >= ETH_MAX_RETRY_DHCP) {
                Retry = 0;
                cfail = 0;
                if (DHCPipver == AS_IPV4) {
                    DHCPipver = AS_IPV6;
                } else {
                    DHCPipver = AS_IPV4;
                }
                
                cIP++;
                if (cIP >= 2) {
                    UTIL_Printf_Str ( DBLEVEL_H, "u_app_eth: DHCP failed\r\n" );
                    fevent_active(sEventAppEth, _EVENT_ETH_HARD_RESET);
                    return 1;
                }
            }
			/* DHCP client Initialization */
			if (gWIZNETINFO.ipmode >= NETINFO_DHCP_V4) {
                if (DHCPipver == AS_IPV4) {
                    DHCPv4_init(SOCKET_DEMO, gDATABUF);
                    reg_dhcpv4_cbfunc (NULL, NULL, AppEth_Cb_IP_Conflict);
                    step = 1;
                } else {
                    DHCP_init (SOCKET_DEMO, gDATABUF);
                    step = 2;
                }
			} else {
                ctlnetwork(CN_SET_NETINFO, (void*) &gWIZNETINFO);
                step = 3;
            }
			break;
        case 1:   //dhcp v4
            result = DHCPv4_run();
            switch (result)
            {
                case DHCP_IPV4_LEASED:
                    step = 3;
                    break;
                case DHCPV4_FAILED:
                    cfail++;
                    if (cfail > ETH_MAX_DHCP_RETRY) {
                        DHCPv4_stop();      
                        cfail = 0;
                    }
                    break;
                case DHCPV4_STOPPED:
                    step = 0;
                    break;
            }
            break;
		case 2:  //dhcp v6
            result = DHCP_run(&gWIZNETINFO); 
            switch (result)
            {
                case DHCP_IP_LEASED:
                    step = 3;
                    break;
                case DHCP_FAILED:
                    cfail++;
                    if (cfail > ETH_MAX_DHCP_RETRY) {
                        DHCPv4_stop();      
                        cfail = 0;
                    }
                    break;
                case DHCP_STOPPED:
                    step = 0;
                    break;
            } 
		default:    //success    
            step = 0; cIP = 0;
            wizchip_network_infor();
            //Active event Open Socket
            fevent_active(sEventAppEth, _EVENT_ETH_SOCK_CTRL);
            sAppEthVar.ReConnect_u8 = false;
            AppEth_FTP_Init_Default();
			return 1;
	}

    fevent_enable(sEventAppEth, event);
    
    return 1;
}

uint8_t aIP_Test[] = {192,168,7,97};

uint16_t Transaction_ID = 1;

static uint8_t _Cb_Socket_Control (uint8_t event)
{ 
    uint8_t SockCtrlState = 0;
    uint8_t SockDataState = 0;
    static uint16_t AnyPortCtrl = 50000;
    static uint16_t AnyPortData = 35000;        
    static uint32_t LandMarkEstab_u32 = 0;
    uint8_t rReConn_u8 = false;
    
    uint8_t aData[20] = {0};
    uint8_t length = 0;
    
    sAppEthVar.LandMarkRun_u32 = RtCountSystick_u32;
//    //Ctrl socket run
//    switch (UTIL_var.ModeConnNow_u8)
//    {
//        case _ETH_MODE_CONN_MAIN:
//        case _ETH_MODE_CONN_BACKUP:
            sEventAppEth[_EVENT_ETH_SOCK_CTRL].e_period = 10;
            
//            SockCtrlState = AppEth_Socket_Control (SOCKET_DEMO, aIP_Test, 
//                                                    502, &AnyPortCtrl);
            SockCtrlState = AppEth_Socket_Control (SOCKET_DEMO, sModemInfor.sServerModTCP.IPnum, 
                                        sModemInfor.sServerModTCP.Port_u16, &AnyPortCtrl);

            if (SockCtrlState == SOCK_ESTABLISHED)
            {
                LandMarkEstab_u32 = RtCountSystick_u32;
//                //Lan dau moi active lai Send
//                if ( (!sAppEthVar.ReConnect_u8) || (sAppEthVar.Status_u8 < _ETH_TCP_CONNECT) )
//                {
//                    APP_LOG(TS_OFF, DBLEVEL_M, "u_app_eth: socket established: %d:%d:%d:%d : %d\r\n",  
//                                                sAppEthVar.sServer.aIP[0], sAppEthVar.sServer.aIP[1], sAppEthVar.sServer.aIP[2],
//                                                sAppEthVar.sServer.aIP[3], *sAppEthVar.sServer.Port_u16);
//
//                    sAppEthVar.Status_u8 = _ETH_TCP_CONNECT;
//                    sAppEthVar.ReConnect_u8 = true;
//                    //Push step connect mqtt
//                    qQueue_Clear(&qEthStep);
//                    Pending_u8 = false;
//                    AppEth_Push_Block_To_Queue( aETH_MQTT_CONN, sizeof(aETH_MQTT_CONN) );
//                }
              fevent_active(sEventAppEth, _EVENT_ETH_SEND_DATA);
              sAppEthVar.Status_u8 = _ETH_MQTT_CONNECTED;
//                            aData[length++] = Transaction_ID >> 8;
//              aData[length++] = Transaction_ID;
//              aData[length++] = 0x00;
//              aData[length++] = 0x00;
//              aData[length++] = 0x00;
//              aData[length++] = 0x06;
//              aData[length++] = 0x01;
//              aData[length++] = 0x04;
//              aData[length++] = 0x00;
//              aData[length++] = 0x20;
//              aData[length++] = 0x00;
//              aData[length++] = 0x08;
//              Transaction_ID++;
//                
////              //Send qua ethernet
////                UTIL_Printf_Str(DBLEVEL_M, "\r\nu_app_eth: message tranfer: \r\n");
////                UTIL_Printf (DBLEVEL_M, sMessage.str.Data_a8, length);
////                UTIL_Printf_Str(DBLEVEL_M, "\r\n");
//                                
//                send (SOCKET_DEMO, aData, length);
////                    return false;
//                sEventAppEth[_EVENT_ETH_SOCK_CTRL].e_period = 1000;
            } else if (SockCtrlState == SOCK_CLOSED)
            {
                if (Check_Time_Out(LandMarkEstab_u32, ETH_MAX_TIME_CTRL_SOCK) == true) 
                {
                    LandMarkEstab_u32 = RtCountSystick_u32;
                    rReConn_u8 = true;
                }
            }
//            break;
//        case _ETH_MODE_CONN_FTP:
//            if (sFTPvar.Retry_u8 >= ETH_MAX_RETRY_UPDATE)
//            {
//                AppEth_Finish_Update(__FAIL_CONNECT);
//                return true;
//            }
//            
//            sEventAppEth[_EVENT_ETH_SOCK_CTRL].e_period = 0;
//                
//            SockCtrlState = AppEth_Socket_Control (CTRL_SOCK, sAppEthVar.sUpdateVar.sServer.aIP, 
//                                                              *sAppEthVar.sUpdateVar.sServer.Port_u16, &AnyPortCtrl);
//        
//            if (SockCtrlState == SOCK_ESTABLISHED)
//            {
//                LandMarkEstab_u32 = RtCountSystick_u32;
//                if (!sAppEthVar.ReConnect_u8)
//                {
//                    APP_LOG(TS_OFF, DBLEVEL_M, "u_app_eth: socket established: %d:%d:%d:%d : %d\r\n",  
//                                                sAppEthVar.sUpdateVar.sServer.aIP[0], sAppEthVar.sUpdateVar.sServer.aIP[1], sAppEthVar.sUpdateVar.sServer.aIP[2],
//                                                sAppEthVar.sUpdateVar.sServer.aIP[3], *sAppEthVar.sUpdateVar.sServer.Port_u16);    
//                    
//                    sAppEthVar.Status_u8 = _ETH_FTP_CONNECTED;
//                    sAppEthVar.ReConnect_u8 = true;
//                    
//                    AppEth_Default_Struct_UpdateVar();
//                    
//                    qQueue_Clear(&qEthStep);
//                    Pending_u8 = false;
//                    AppEth_Push_Block_To_Queue( aETH_FTP_CONN, sizeof(aETH_FTP_CONN) ); 
//                }
//                
//                //Polling recv
//                if (getSn_RX_RSR(CTRL_SOCK) > 0)
//                { 
//                   fevent_active(sEventAppEth, _EVENT_ETH_RECV_DATA);
//                } 
//            } else if (SockCtrlState == SOCK_CLOSED)
//            {
//                if (Check_Time_Out(LandMarkEstab_u32, ETH_MAX_TIME_CTRL_SOCK) == true) 
//                {
//                    LandMarkEstab_u32 = RtCountSystick_u32;
//                    AnyPortCtrl = 50000;
//                    rReConn_u8 = true;
//                }            
//            }  
//            
//            //Data socket run
//            if (sFTPvar.dsock_state == DATASOCK_READY)
//            {
//                if (sFTPvar.DataSockConnected_u8 != true)
//                {
//                    SockDataState = AppEth_Socket_Control (DATA_SOCK, sFTPvar.RemoteIP,
//                                                           sFTPvar.RemotePort, &AnyPortData);
//                } else
//                {
//                    SockDataState = getSn_SR(DATA_SOCK);
//                }
//                
//                if (SockDataState == SOCK_ESTABLISHED)
//                {
//                    sFTPvar.DataSockConnected_u8 = true;
//                }
//                
//                if (sFTPvar.DataSockConnected_u8 == true)
//                {
//                    //Neu chua lay xong fw ma mat ket noi: thu lai
//                    if ( (AppEth_FTP_DataSock_Run() == false)
//                        || (SockDataState != SOCK_ESTABLISHED))
//                    {
//                        close (CTRL_SOCK);
//                        close (DATA_SOCK);
//                        rReConn_u8 = true;
//                    }
//                }
//            }
//            
//            //Neu 60s ma k connect duoc: se retry lai
//            if (Check_Time_Out(sFTPvar.LandMark_u32, 60000) == true)
//            {
//                rReConn_u8 = true;
//            }
//            
//            //Kiem tra xem update thanh cong khong
//            if ((sAppEthVar.sUpdateVar.FwSizeGet_u32 != 0)
//                && (sAppEthVar.sUpdateVar.Total_u8 >= sAppEthVar.sUpdateVar.FwSizeGet_u32))
//            {
//                //Check crc
//                if (sAppEthVar.sUpdateVar.CrcCalcu_u8 == sAppEthVar.sUpdateVar.CrcFile_u8)
//                {
//                    AppEth_Finish_Update(__SUCCESSFUL);
//                    return true;
//                } else
//                {
//                    AppEth_Finish_Update(__FAIL_CRC);
//                    return false;
//                }
//            }
//            break;
//        case _ETH_MODE_CONN_HTTP:
//            //Neu 60s ma k connect duoc: se retry lai
//            if (Check_Time_Out(sFTPvar.LandMark_u32, 60000) == true)
//            {
//                rReConn_u8 = true;
//                sAppEthVar.Retry_u8 = ETH_MAX_SOCK_RETRY;
//            }
//            break;
//        default:
//            break;
//    }
 
    if (rReConn_u8 == true)
    {
        sAppEthVar.Retry_u8++;
        if (sAppEthVar.Retry_u8 >= ETH_MAX_SOCK_RETRY)
        {
            fevent_active(sEventAppEth, _EVENT_ETH_HARD_RESET);
        } else
        {
            fevent_active(sEventAppEth, _EVENT_ETH_RUN_DHCP);
        }
        
        return true;
    }
    
    fevent_enable(sEventAppEth, event);
    
    
    return true;
}


//Func Callback Event
static uint8_t _Cb_Send (uint8_t event)
{
    uint8_t cmd_u8 = AppEth_Get_Queue(0); 
    
    sAppEthVar.LandMarkRun_u32 = RtCountSystick_u32;
    
    if (cmd_u8 < _ETH_CMD_END)
    {
        Pending_u8 = true;
        LandMarkSend_u32 = RtCountSystick_u32;
        
        //send
        if (sEthCmd[cmd_u8].pFunc_Send != NULL)
        {
            if (sEthCmd[cmd_u8].pFunc_Send() == false)
            {   
                fevent_active(sEventAppEth, _EVENT_ETH_SEND_TIMEOUT);
                Pending_u8 = false;
                return 1;
            }
        }
        //cho timeout or excute success
        if (sEthCmd[cmd_u8].sResp == NULL)
        {
            sEthCmd[cmd_u8].pCb_Success(NULL);
            fevent_active(sEventAppEth, _EVENT_ETH_SEND_OK);
        } else
        {
            fevent_enable(sEventAppEth, _EVENT_ETH_SEND_TIMEOUT);
        }
    } else
    {
        Pending_u8 = false;
    }

	return 1;
}

static uint8_t _Cb_Send_OK (uint8_t event)
{
    //Nhay tiep lenh tiep theo
    AppEth_Get_Queue(1);
    fevent_disable(sEventAppEth, _EVENT_ETH_SEND_TIMEOUT);
    fevent_enable(sEventAppEth, _EVENT_ETH_SEND);

	return 1;
}

static uint8_t _Cb_Send_Timeout (uint8_t event)
{
    uint8_t cmd_u8 = AppEth_Get_Queue(0); 
    
    UTIL_Printf_Str ( DBLEVEL_M, "u_app_eth: send timeout\r\n");
    
    sAppEthVar.cHardReset_u16++;
    mSet_default_MQTT();
    
    switch (cmd_u8)
    {
        case _ETH_MQTT_CONN:
        case _ETH_MQTT_SUB:
            //chuyen xuong event DHCP
            qQueue_Clear(&qEthStep);
            Pending_u8 = false;
            fevent_active(sEventAppEth, _EVENT_ETH_RUN_DHCP);
            break;
        case _ETH_MQTT_PUB_ACK:
        case _ETH_MQTT_PING:
            //clear queue and push conn again
            qQueue_Clear(&qEthStep);
            Pending_u8 = false;
            AppEth_Push_Block_To_Queue( aETH_MQTT_CONN, sizeof(aETH_MQTT_CONN) );
            break;
        case _ETH_FTP_SET_USER:
        case _ETH_FTP_SET_PASS:
        case _ETH_FTP_SET_TYPE_ASCII:
        case _ETH_FTP_SET_TYPE_BIN:
        case _ETH_FTP_QUER_SER_DIR:
        case _ETH_FTP_QUER_SIZE:
        case _ETH_FTP_SET_DIR:
        case _ETH_FTP_SET_PASV_PORT:
        case _ETH_FTP_SET_GET_FILE:
            //chuyen xuong event DHCP
            send (CTRL_SOCK, (uint8_t *)"QUIT\r\n", strlen("QUIT\r\n"));
            qQueue_Clear(&qEthStep);
            Pending_u8 = false;
            fevent_active(sEventAppEth, _EVENT_ETH_RUN_DHCP);
            break;
        default:
            break;
    }
    
	return 1;
}

uint32_t Test_GetTick = 0;
uint32_t Result_GetTick = 0;

static uint8_t _Cb_Recv_Data (uint8_t event)
{ 
    int Pos = -1;
    uint8_t cmd_u8 = AppEth_Get_Queue(0); 
    uint16_t i = 0;
    uint16_t size = 0;
    
    sData   strSource = {&aRECEIVE_DATA[0], 0};
    sData   strFix = {NULL, 0};

    if ( (UTIL_var.ModeConnNow_u8 == _CONNECT_DATA_MAIN)
          || (UTIL_var.ModeConnNow_u8  == _CONNECT_DATA_BACKUP) 
          || (UTIL_var.ModeConnNow_u8  == _CONNECT_FTP_UPLOAD) )
    {
        if ((size = getSn_RX_RSR(SOCKET_DEMO)) > 0)
        {
            if (size > DATA_BUF_SIZE) 
                size = DATA_BUF_SIZE - 1;
            
            LengthRecv = recv (SOCKET_DEMO, aRECEIVE_DATA, size);
            strSource.Length_u16 = LengthRecv;
            
            if (LengthRecv != size)
            {
                return 0;
            }
        }
    } else
    {
        if ((size = getSn_RX_RSR(CTRL_SOCK)) > 0)
        { 
            memset(aRECEIVE_DATA, 0, sizeof(aRECEIVE_DATA));
            
            if (size > sizeof(aRECEIVE_DATA)) 
                size = sizeof(aRECEIVE_DATA) - 1;
            
            LengthRecv = recv(CTRL_SOCK, aRECEIVE_DATA, size);

            if (LengthRecv != size)
            {
                return 0;
            }
            
            aRECEIVE_DATA[LengthRecv] = '\0';
            strSource.Length_u16 = LengthRecv + 1;
        }
    }

    if (LengthRecv > 0)
    {
        UTIL_Printf_Str ( DBLEVEL_M, "u_app_eth: intn recv: \r\n");          
        UTIL_Printf_Hex ( DBLEVEL_M, strSource.Data_a8, strSource.Length_u16);
        UTIL_Printf_Str(DBLEVEL_M, "\r\n");
        
        for(uint8_t i = 0; i < strSource.Length_u16; i++)
            sDataRecvTCP.Data_a8[i] = strSource.Data_a8[i];
        
        sDataRecvTCP.Length_u16 = strSource.Length_u16;
        
        Result_GetTick = HAL_GetTick() - Test_GetTick;

////        if (sMessage.Status_u8 == PENDING)
////        {            
//            strFix.Data_a8 = (uint8_t*) sEthCmd[cmd_u8].sResp;
//            strFix.Length_u16 = strlen( sEthCmd[cmd_u8].sResp );   
//    
//            if (strFix.Data_a8 != NULL)
//            {
//                Pos = Find_String_V2(&strFix, &strSource);
//                if (Pos >= 0)
//                {
//                    sEthCmd[cmd_u8].pCb_Success(&strSource);
//                    fevent_active(sEventAppEth, _EVENT_ETH_SEND_OK);
//                }
//            }
////        }
//        
//        //Check Recv Server
//        for (i = _ETH_MQTT_URC_SERVER; i <= _ETH_MQTT_URC_SERVER; i++) 
//        {
//            strFix.Data_a8 = (uint8_t *) sEthResCode[i].sResp;
//            strFix.Length_u16 = strlen(sEthResCode[i].sResp);
//            //
//            Pos = Find_String_V2(&strFix, &strSource);
//            if (Pos >= 0)
//            {
//                sEthResCode[i].pCb_Success(&strSource);
//            }
//        }
    }
    
    //Clear
    LengthRecv = 0;
    memset(aRECEIVE_DATA, 0, sizeof(aRECEIVE_DATA));

	return 1;
}


static uint8_t _Cb_Send_Ping (uint8_t event)
{
    if (sAppEthVar.Status_u8 != _ETH_MQTT_CONNECTED) {
        return 1;
    }
    
    if (UTIL_var.ModePower_u8 == _POWER_MODE_ONLINE) {
        sMessage.aMESS_PENDING[DATA_PING_2] = TRUE;
        fevent_enable(sEventAppEth, event);   
        fevent_active(sEventAppEth, _EVENT_ETH_SEND_DATA);   
    }
    
    return 1;
}



static uint8_t _Cb_Send_Data (uint8_t event)
{
  
  uint8_t aData[20] = {0};
  uint8_t length = 0;
    if (sAppEthVar.Status_u8 != _ETH_MQTT_CONNECTED) {
        return 1;
    }
    
//    if (AppEth_Send_Mess() == true)
//    {        
//        fevent_enable(sEventAppEth, _EVENT_ETH_SEND_PING);
//    } else {
//        if (UTIL_var.ModePower_u8 == _POWER_MODE_ONLINE) {
//            if (sEventAppEth[_EVENT_ETH_SEND_PING].e_status == 0)
//                fevent_enable(sEventAppEth, _EVENT_ETH_SEND_PING);   
//        } else {
//            //Neu con ban tin moi: Enable lai check new rec
//            if (Mem_Is_New_Record() != true) {
//                fevent_active(sEventAppEth, _EVENT_ETH_POWER_OFF);
//                return 1;
//            }
//        }
//    }
    
//                  aData[length++] = Transaction_ID >> 8;
//              aData[length++] = Transaction_ID;
//              aData[length++] = 0x00;
//              aData[length++] = 0x00;
//              aData[length++] = 0x00;
//              aData[length++] = 0x06;
//              aData[length++] = 0x01;
//              aData[length++] = 0x04;
//              aData[length++] = 0x00;
//              aData[length++] = 0x20;
//              aData[length++] = 0x00;
//              aData[length++] = 0x08;
                
//              //Send qua ethernet
//                UTIL_Printf_Str(DBLEVEL_M, "\r\nu_app_eth: message tranfer: \r\n");
//                UTIL_Printf (DBLEVEL_M, sMessage.str.Data_a8, length);
//                UTIL_Printf_Str(DBLEVEL_M, "\r\n");
//              Transaction_ID++;
                                
    if(sTransModTCP.Flag == 1)
    {
        send (SOCKET_DEMO, sTransModTCP.aData, sTransModTCP.length);
        sTransModTCP.Flag = 0;
        UTIL_Printf_Str(DBLEVEL_M, "\r\nu_app_eth: message tranfer: \r\n");
        UTIL_Printf_Hex (DBLEVEL_M, sTransModTCP.aData, sTransModTCP.length);
        UTIL_Printf_Str(DBLEVEL_M, "\r\n");
        Test_GetTick = HAL_GetTick();
    }

//                    return false;
    sEventAppEth[_EVENT_ETH_SOCK_CTRL].e_period = 1000;
    
    fevent_enable(sEventAppSim, event);
    
    return 1;
}



uint8_t AppEth_Send_Mess (void)
{
    uint16_t i = 0;
    uint8_t Result = FALSE;
    static uint32_t LandMarkSendMess_u32 = 0;

    if (Check_Time_Out (LandMarkSendMess_u32, 120000) == true) {
        sMessage.Status_u8 = false;
    } 
    //Kiem tra xem co ban tin nao can gui di khong
    for (i = TOPIC_NOTIF; i < END_MQTT_SEND; i++)
    {
        if (sMessage.aMESS_PENDING[i] == TRUE)
        {
            if (UTIL_var.ModeConnNow_u8 == _CONNECT_FTP_UPLOAD) {
                if (i != DATA_GPS) {
                    continue;
                }
            } else {
                if (i == DATA_GPS) {
                    continue;
                }
            }

            if (i == DATA_PING) {  //bo qua ping cua sim
                continue;
            }
            
            Result = TRUE;
            //
            if (sMessage.Status_u8 != PENDING)
            {
                //Danh dau Mess type hien tai ->neu OK clear di
                sMessage.Type_u8 = i;
                //Dong goi ban tin vao buff aPAYLOAD bang cach thuc hien callback
                if (sMessSend[i].CallBack(i) == TRUE)
                {                           
                    sMessage.Status_u8 = PENDING;
                    LandMarkSendMess_u32 = RtCountSystick_u32;
                    
                    //Push cmd Pub or Ping
                    if (sMessage.Type_u8 == DATA_PING_2) {
                        AppEth_Push_Cmd_To_Queue(_ETH_MQTT_PING);
                    } else {
                        if (sMessage.PubQos != 0) {
                            AppEth_Push_Cmd_To_Queue(_ETH_MQTT_PUB_ACK);
                        } else {
                            AppEth_Push_Cmd_To_Queue(_ETH_MQTT_PUB_NACK);
                        }
                    }
                } else {
                    sMessage.aMESS_PENDING[i] = FALSE;
                }
            } 
        }
    }
    
    return Result;
}


static uint8_t _Cb_Control_Sim (uint8_t event)
{
#ifdef USING_APP_SIM
    if ( ( (UTIL_var.ModeConnNow_u8 != _CONNECT_DATA_MAIN)
            && (UTIL_var.ModeConnNow_u8  != _CONNECT_DATA_BACKUP) 
            && (UTIL_var.ModeConnNow_u8  != _CONNECT_FTP_UPLOAD) )
        || (UTIL_var.ModePower_u8 == _POWER_MODE_SAVE) )
    {
        if (sAppEthVar.Status_u8 < _ETH_TCP_CONNECT)
        {
            AppSim_Restart_If_PSM();
        } else
        {
            //Power Off SIM
            if (sSimCommVar.State_u8 != _SIM_POWER_OFF)
                fevent_active(sEventAppSim, _EVENT_SIM_POWER_OFF);
        }
    }

    fevent_enable(sEventAppEth, event);
#endif

    return 1;
}

static uint8_t _Cb_INTn_Process (uint8_t event)
{
    static uint8_t Retry = 0;
    
    if (AppEth_IRQ_Handler() == false)
    {
        Retry++;
        if ( Retry < 5)
        {
            fevent_enable(sEventAppEth, event);
            return 1;
        }
    } 
    
    Retry = 0;
        
    return 1;
}


static uint8_t _Cb_Power_Off (uint8_t event)
{
    UTIL_Printf_Str ( DBLEVEL_M, "u_app_eth: power off!\r\n"); 
    
    //Set trang thai
    sAppEthVar.Status_u8 = _ETH_POWER_OFF;
    
    W5500_OFF;
    
    //Disable all event
    for (uint8_t i = _EVENT_ETH_CONTROL_SIM; i < _EVENT_ETH_POWER_OFF; i++)
        fevent_disable(sEventAppEth, i);
    
    if (UTIL_var.ModePower_u8 == _POWER_MODE_ONLINE)
    { 
        fevent_active(sEventAppEth, _EVENT_ETH_HARD_RESET); 
        fevent_enable(sEventAppEth, _EVENT_ETH_CONTROL_SIM); 
    }

    return 1;
}




/*===================== Funcion ===================== */

static void  wizchip_select(void)
{
	W5500_SCS_LOW;
}

static void  wizchip_deselect(void)
{
	W5500_SCS_HIGH;
}

static uint8_t wizchip_read(void)
{
	uint8_t retVal = 0;
    uint8_t aDUMMY_BYTE = 0xA5;

	HAL_SPI_TransmitReceive(&hspi3, &aDUMMY_BYTE, &retVal, 1, 1000);

	return retVal;
}

static void wizchip_write(uint8_t wb)
{
	uint8_t retVal = 0;

	HAL_SPI_TransmitReceive(&hspi3, &wb, &retVal, 1, 1000); 
}


static void wizchip_read_buff(uint8_t* buf, datasize_t len)
{
    for (uint16_t i = 0; i < len; i++)
        buf[i] = wizchip_read(); 
}

static void wizchip_write_buff(uint8_t* buf, datasize_t len)
{
    for (uint16_t i = 0; i < len; i++)
    	wizchip_write(buf[i]);
}

static void wizchip_network_infor(void)
{
    char aTEMP[512] = {0};
    
    wizchip_getnetinfo(&gWIZNETINFO);
    
    sprintf(aTEMP, "Mac address: %02x:%02x:%02x:%02x:%02x:%02x\n\r",gWIZNETINFO.mac[0],gWIZNETINFO.mac[1],
                                                    gWIZNETINFO.mac[2],gWIZNETINFO.mac[3],gWIZNETINFO.mac[4],gWIZNETINFO.mac[5]);
    sprintf(aTEMP + strlen(aTEMP), "IP address : %d.%d.%d.%d\n\r",gWIZNETINFO.ip[0],gWIZNETINFO.ip[1],gWIZNETINFO.ip[2],gWIZNETINFO.ip[3]);
    sprintf(aTEMP + strlen(aTEMP), "SM Mask	   : %d.%d.%d.%d\n\r",gWIZNETINFO.sn[0],gWIZNETINFO.sn[1],gWIZNETINFO.sn[2],gWIZNETINFO.sn[3]);
    sprintf(aTEMP + strlen(aTEMP), "Gate way   : %d.%d.%d.%d\n\r",gWIZNETINFO.gw[0],gWIZNETINFO.gw[1],gWIZNETINFO.gw[2],gWIZNETINFO.gw[3]);
    sprintf(aTEMP + strlen(aTEMP), "DNS Server : %d.%d.%d.%d\n\r",gWIZNETINFO.dns[0],gWIZNETINFO.dns[1],gWIZNETINFO.dns[2],gWIZNETINFO.dns[3]);
    
    UTIL_Printf_Str ( DBLEVEL_M, aTEMP); 
}


/*============== Func Handler App Enthernet=============*/

/*
Func: Hard reset module
+ Active low RST atleast 500ms
+ Check PHY regis
*/

uint8_t AppEth_W5500_Hard_Reset(void)
{
	static uint8_t EthStepHardRS_u8 = 0;
	uint8_t Result = false;

	switch (EthStepHardRS_u8)
	{
		case 0:
			UTIL_Printf_Str ( DBLEVEL_M, "\r\nu_app_eth: hard reset w5500...\r\n" );
			W5500_OFF;
			EthStepHardRS_u8++;
			break;
		case 1:
			W5500_ON;
            EthStepHardRS_u8++;
			break;
        case 2:
			W5500_RST_LOW;
			EthStepHardRS_u8++;
			break;
		case 3:
			Result = true;
			W5500_RST_HIGH;
			EthStepHardRS_u8 = 0;
			break;
		default:
            EthStepHardRS_u8 = 0;
			break;
	}

	return Result;
}


void AppEth_Init(void)
{
    //Set callback handler SPI: select, deselect, read, write: 
	reg_wizchip_cs_cbfunc(wizchip_select, wizchip_deselect);   
	reg_wizchip_spi_cbfunc(wizchip_read, wizchip_write, wizchip_read_buff, wizchip_write_buff);   
    
    sMessage.aMESS_PENDING[SEND_SERVER_TIME_PENDING] = TRUE;
    
    //creat queue cmd
    qQueue_Create (&qEthStep, ETH_MAX_CMD_QUEUE, sizeof (uint8_t), (uint8_t *) &aQEthStepControl);
}
    


uint8_t AppEth_Task(void)
{
	uint8_t Result = 0;
	uint8_t i = 0;

	for (i = 0; i < _EVENT_APP_ETH_END; i++)
	{
		if (sEventAppEth[i].e_status == 1)
		{
            if (i != _EVENT_ETH_CONTROL_SIM)
                Result = 1;
            
			if ((sEventAppEth[i].e_systick == 0) 
                || ((HAL_GetTick() - sEventAppEth[i].e_systick) >= sEventAppEth[i].e_period))
			{
                sEventAppEth[i].e_status = 0;   //Disable event
				sEventAppEth[i].e_systick = HAL_GetTick();
				sEventAppEth[i].e_function_handler(i);
			}
		}
	}

	return Result;
}




void AppEth_W5500_Init(void)
{
	uint8_t memsize[2][8] = 
    { 
        { 2, 2, 2, 2, 2, 2, 2, 2 }, 
        { 2, 2, 2, 2, 2, 2, 2, 2 }, 
    };
//    uint8_t temp = IK_DEST_UNREACH;
    uint8_t syslock = SYS_NET_LOCK;
    
	/* wizchip initialize*/
	if (ctlwizchip(CW_INIT_WIZCHIP, (void*) memsize) == -1) {   
		UTIL_Printf_Str ( DBLEVEL_H, "u_app_eth: error Set Tx/RX size\r\n" );
	}
    //New add other example
	UTIL_Printf_Str ( DBLEVEL_H, "u_app_eth: set RTR, RCR\r\n" );
    
    setRTR(0x07d0);
	setRCR(4);
    
    setSIMR (0x0F);         //IRQ 4 sock 0 -> 3
	setSn_IMR (0, 0x1F);  //Mark All IRQ Sn
    
//    if (ctlwizchip(CW_SET_INTRMASK, &temp) == -1) {
//		UTIL_Printf_Str ( DBLEVEL_H, "u_app_eth: interrupt mark\r\n" );
//	}
    
    ctlwizchip(CW_SYS_UNLOCK,& syslock);
}



int32_t AppEth_Socket_Control (uint8_t sn, uint8_t* destip, uint16_t destport, uint16_t *any_port)
{
    int32_t Result = SOCK_CLOSED;
    static uint8_t StepConn_u8 = 0;
    static uint32_t LandMark_u32 = 0;
    
    // Check the W5500 Socket n status register (Sn_SR, The 'Sn_SR' controlled by Sn_CR command or Packet send/recv status)
    Result = getSn_SR(sn);
    
    switch (getSn_SR(sn))
    {
      case SOCK_ESTABLISHED:
            StepConn_u8 = 0;
            break;
      case SOCK_CLOSE_WAIT:
            StepConn_u8 = 0;
            disconnect(sn);            
            break;
      case SOCK_CLOSED:
            StepConn_u8 = 0;
            // TCP socket open with 'any_port' port number
            APP_LOG(TS_OFF, DBLEVEL_H, "u_app_eth: sock %d: start\r\n", sn);
            close(sn);
            if (socket(sn, Sn_MR_TCP, (*any_port)++, 0x00) != sn)
            {
                if (*any_port == 0xffff) 
                    *any_port = 50000;
            } 
            break;
      case SOCK_INIT :
            switch (StepConn_u8)
            {
                case 0: 
                    APP_LOG(TS_OFF, DBLEVEL_H, "u_app_eth: sock %d: opened\r\n", sn);
                    //	Try to TCP connect to the TCP server (destination)
                    if (connect(sn, destip, destport, 4) != SOCK_OK) 
                    {
                        close(sn);
                        break;
                    }
                    
                    StepConn_u8++;
                    LandMark_u32 = RtCountSystick_u32;
                    break;
                default:
                    if (getSn_IR(sn) & Sn_IR_TIMEOUT)
                    {
                        setSn_IR(sn, Sn_IR_TIMEOUT);
                        APP_LOG(TS_OFF, DBLEVEL_M, "u_app_eth: %d:connect timeout!\r\n", sn);
                        StepConn_u8 = 0;
                    }
                    
                    if (Check_Time_Out(LandMark_u32, 10000) == true)
                    {
                        close(sn);
                    }
                    break;
            }            
            break;      
      default:
            close(sn);
            setSn_MR(sn, Sn_MR_TCP);
            break;
    }
    
    return Result;
}


uint8_t AppEth_IRQ_Handler(void)
{
	uint8_t IRQ_sign = 0;
    
    //Doc tiep thanh ghi SIR (ngat socket n) -> Xem co ngat S0?
    IRQ_sign = getSIR();
    IRQ_sign = IRQ_sign & (1 << SOCKET_DEMO);
    
    if (IRQ_sign != 0)  //Ngat o socket 0
    {
        IRQ_sign = getSn_IR(SOCKET_DEMO);   //Doc tiep S0_IR de check ngat gi
        setSn_IR (SOCKET_DEMO, IRQ_sign);   //Ghi gia tri 1 vao de xoa
        
        if (sAppEthVar.Status_u8 >= _ETH_MQTT_CONNECT)
        {
            //Recv IR
            if (IRQ_sign & Sn_IR_RECV)
            {
                fevent_active(sEventAppEth, _EVENT_ETH_RECV_DATA);
            }
            //TIMEOUT IR
            if (IRQ_sign & Sn_IR_TIMEOUT)
            {
                close(SOCKET_DEMO);
                fevent_active(sEventAppEth, _EVENT_ETH_RUN_DHCP);
            }
        }
    } else
    {
        //thanh ghi SIR cacs bit = '1' cho den khi cac bit Sn_IR duoc xoa. Nen UnMark cac Sn_IR khac k?
        return true;
    }
  
    return false;
}


/*=============== Func callback ======================*/

uint8_t AppEth_Get_PHYStatus(void)
{
	int8_t tmp = 0;

    ctlwizchip(CW_GET_PHYLINK, (void*) &tmp); 
    
    if (tmp == PHY_LINK_ON ) {
        return true;
    }

    return false;
}

/*
    Call back for ip Conflict
        + Quay lai hard reset module
*/

void AppEth_Cb_IP_Conflict (void)
{
    UTIL_Printf_Str ( DBLEVEL_H, "u_app_eth: dhcp ip conflict\r\n" );   
    //active Hard reset 
    fevent_active(sEventAppEth, _EVENT_ETH_HARD_RESET);
}


/*
    Func: Restart module sim neu nhu sim dang o che do PSM
*/
void AppEth_Restart_If_PSM(void)
{
    if ( sAppEthVar.Status_u8 == _ETH_POWER_OFF )   //co truong hop loi o day
    {
        sAppEthVar.Status_u8  = _ETH_START;
        fevent_active(sEventAppEth, _EVENT_ETH_HARD_RESET);
        fevent_enable(sEventAppEth, _EVENT_ETH_CONTROL_SIM);
    }
}



//Function handle

uint8_t AppEth_Get_Queue(uint8_t Type)
{
	uint8_t step = 0;

	if (qGet_Number_Items(&qEthStep) == 0)
		return _ETH_CMD_END;
    
	if (qQueue_Receive(&qEthStep, (uint8_t *) &step, Type) == 0)
        return _ETH_CMD_END;

	return step;
}



uint8_t AppEth_Push_Cmd_To_Queue(uint8_t cmd)
{
	if (cmd >= _ETH_CMD_END)
		return 0;

	if (qQueue_Send(&qEthStep, (uint8_t *) &cmd, _TYPE_SEND_TO_END) == 0)
		return 0;

    if ((Pending_u8 == false ) 
        || (Check_Time_Out(LandMarkSend_u32, 80000) == true) )
    {
		fevent_active(sEventAppEth, _EVENT_ETH_SEND);
    }

	return 1;
}


uint8_t AppEth_Push_Block_To_Queue(uint8_t *blockCmd, uint8_t size)
{
	uint8_t i = 0;

	for (i = 0; i < size; i++)
	{
		if (AppEth_Push_Cmd_To_Queue (blockCmd[i]) == 0)
			return 0;
	}

	return 1;
}



//Func callback cmd
static uint8_t _Cb_Success_Def (sData *pData)
{
    
    return 1;
}


static uint8_t _Cb_Mqtt_Sub_OK (sData *pData)
{
    UTIL_Printf_Str( DBLEVEL_M, "u_app_eth: mqtt connect ok!\r\n" );
    
    sMessage.aMESS_PENDING[DATA_HANDSHAKE] = TRUE;
    
    //active send pub
    mSet_default_MQTT(); 
    fevent_active(sEventAppEth, _EVENT_ETH_SEND_DATA);
    sAppEthVar.Status_u8 = _ETH_MQTT_CONNECTED;

    return 1;
}

static uint8_t _Cb_Mqtt_Pub_OK (sData *pData)
{
    UTIL_Printf_Str( DBLEVEL_M, "u_app_eth: mqtt publish ok!\r\n" );
    
    //Set status ve true
    sAppEthVar.Status_u8 = _ETH_MQTT_CONNECTED;
    sMessage.Status_u8 = TRUE;   
        
    switch (sMessage.Type_u8)
    {
        case DATA_TSVH_MULTI:
        case DATA_TSVH_FLASH:
        case DATA_TSVH_OPERA:
            Mem_Inc_Index_Send(&sRecTSVH, 1);
            mReset_Raw_Data(); 
            break;
        case DATA_EVENT:
            Mem_Inc_Index_Send(&sRecEvent, 1);
            mReset_Raw_Data();
            break;
        case DATA_GPS:
            Mem_Inc_Index_Send(&sRecGPS, 1);
            mReset_Raw_Data();
            break;
        case SEND_SAVE_BOX_OK:
            sMessage.aMESS_PENDING[sMessage.Type_u8] = FALSE; 
            
            if (sAppSimVar.pReq_Save_Box != NULL)
                sAppSimVar.pReq_Save_Box();
            break;
        case SEND_SHUTTING_DOWN:
            if (sAppSimVar.pReset_MCU_Imediate != NULL)
                sAppSimVar.pReset_MCU_Imediate();
            break;
        case SEND_UPDATE_FIRM_OK:
        case SEND_UPDATE_FIRM_FAIL:
            sAppSimVar.IsFwUpdateSuccess_u8 = TRUE;          
            sMessage.aMESS_PENDING[sMessage.Type_u8] = FALSE; 
            
            if (sAppSimVar.pReset_MCU_Imediate != NULL)
                sAppSimVar.pReset_MCU_Imediate();
            break;
        default:
            sMessage.aMESS_PENDING[sMessage.Type_u8] = FALSE;  
            break;
    }
    
    sAppEthVar.cHardReset_u16 = 0;
    sAppEthVar.LandMarkConnMQTT_u32 = RtCountSystick_u32;
    
    fevent_enable(sEventAppEth, _EVENT_ETH_SEND_DATA);
    
    return 1;
}

static uint8_t _Cb_Pasv_Port_OK (sData *pData)
{
    if (pportc((char *) pData->Data_a8) == -1)
    {
        APP_LOG(TS_OFF, DBLEVEL_M, "Bad port syntax\r\n");
    }
    else
    {
        APP_LOG(TS_OFF, DBLEVEL_M, "Go Open Data Sock...\r\n ");
        
        sFTPvar.dsock_mode = PASSIVE_MODE;
        sFTPvar.dsock_state = DATASOCK_READY;
    }

    return 1;
}


static uint8_t _Cb_Get_OK (sData *pData)
{
    //bat dau cho lay firmware
    sFTPvar.DataGetStart_u8 = true;  

    return 1;
}


static uint8_t _Cb_Get_Size_OK (sData *pData)
{
    //
    sData strcheck;
    int Pos = -1;
    uint16_t PosFind = 0;
    int64_t temp_i64 = 0;
    
    strcheck.Data_a8 = (uint8_t*) sEthCmd[_ETH_FTP_QUER_SIZE].sResp;
    strcheck.Length_u16 = strlen( sEthCmd[_ETH_FTP_QUER_SIZE].sResp );   

    Pos = Find_String_V2(&strcheck, pData);
    if (Pos >= 0)
    {
        PosFind = Pos + strlen( sEthCmd[_ETH_FTP_QUER_SIZE].sResp);
        //lay gia tri so sau do
        temp_i64 = UTIL_Get_Num_From_Str(pData, &PosFind);
        if (temp_i64 == -1)
        {
            APP_LOG(TS_OFF, DBLEVEL_M, "u_app_eth: get size file error!\r\n ");
            return 0;
        }
        
        sAppEthVar.sUpdateVar.FwSizeGet_u32 = (uint32_t) temp_i64;
    }
    
    return 1;
}


static uint8_t _Cb_List_OK (sData *pData)
{

    return 1;
}



static uint8_t _Cb_URC_Ready (sData *pData)
{
    //bat dau truyen lenh
    sFTPvar.CtrlReady_u8 = true;  

    return 1;
}

static uint8_t _Cb_URC_Server (sData *pData)
{
    uint8_t var = 0;
    int PosFind = 0;
    
    for (var = REQUEST_RESET; var < END_MQTT_RECEI; ++var)
    {
        PosFind = Find_String_V2 ((sData*) &sMessRecv[var].sKind, pData);
        
        if ((PosFind >= 0) && (sMessRecv[var].CallBack != NULL))
            sMessRecv[var].CallBack(pData, PosFind);
    }

    return 1;
}


static uint8_t _fMQTT_Connect (void)
{
    char aCLIENT[40] = {"Eth"};
      
    sAppEthVar.Status_u8 = _ETH_MQTT_CONNECT;
    sprintf(aCLIENT + strlen(aCLIENT), "%s", sMessage.pModemId);
    
    mConnect_MQTT( aCLIENT,
                   sSimCommVar.sServer.sMQTTUserName, 
                   sSimCommVar.sServer.sMQTTPassword,
                   60 );

    //Send via Ethernet
    UTIL_Printf (DBLEVEL_M,  sMessage.str.Data_a8, sMessage.str.Length_u16);
    
    if (send (SOCKET_DEMO, sMessage.str.Data_a8, sMessage.str.Length_u16) == -1)
        return false;
    
    sMessage.Status_u8 = PENDING;
    
    return true;
}

static uint8_t _fMQTT_Subcribe (void)
{
    sAppEthVar.Status_u8 = _ETH_MQTT_SUBCRIBE;
    
    mSubcribe_MQTT();
    //Send via Ethernet
    UTIL_Printf (DBLEVEL_M,  sMessage.str.Data_a8, sMessage.str.Length_u16);
    
    if (send (SOCKET_DEMO, sMessage.str.Data_a8, sMessage.str.Length_u16) == -1)
        return false;
    
    sMessage.Status_u8 = PENDING;
    
    return true;
}

static uint8_t _fMQTT_Publish (void)
{
    //Send qua ethernet
    UTIL_Printf_Str(DBLEVEL_M, "\r\nu_app_eth: message tranfer: \r\n");
    UTIL_Printf (DBLEVEL_M, sMessage.str.Data_a8, sMessage.str.Length_u16);
    UTIL_Printf_Str(DBLEVEL_M, "\r\n");
                    
    if (send (SOCKET_DEMO, sMessage.str.Data_a8, sMessage.str.Length_u16) == -1)
        return false;
    
    return true;
}


static uint8_t _fSet_User (void)
{
    char dat[50]={0};
    
    sprintf(dat,"USER %s\r\n", sAppEthVar.sUpdateVar.sServer.UserName);   // saoviet
    
    APP_LOG(TS_OFF, DBLEVEL_M, "u_ftp: set user: \"%s\"", dat);

    send(CTRL_SOCK, (uint8_t *)dat, strlen(dat));
    
    return 1;
}


static uint8_t _fSet_Pass (void)
{
    char dat[50]={0};

    sprintf(dat,"PASS %s\r\n", sAppEthVar.sUpdateVar.sServer.Password);   // sv123@
    APP_LOG(TS_OFF, DBLEVEL_M,"u_ftp: set pasword: \"%s\"", dat);
    
    send(CTRL_SOCK, (uint8_t *)dat, strlen(dat));
    
    return 1;
}

static uint8_t _fSet_Type_ASCII (void)
{
    char dat[50]={0};

    sprintf(dat,"TYPE %c\r\n", TransferAscii);
    APP_LOG(TS_OFF, DBLEVEL_M,"u_ftp: type: \"%s\"", dat);
    
    sFTPvar.type = ASCII_TYPE;
    send(CTRL_SOCK, (uint8_t *)dat, strlen(dat));
    
    return 1;
}

static uint8_t _fSet_Type_BIN (void)
{
    char dat[50]={0};

    sprintf(dat,"TYPE %c\r\n", TransferBinary);
    APP_LOG(TS_OFF, DBLEVEL_M,"u_ftp: type: \"%s\"", dat);
    
    sFTPvar.type = ASCII_TYPE;
    send(CTRL_SOCK, (uint8_t *)dat, strlen(dat));
    
    return 1;
}


static uint8_t _fQuery_List (void)
{
    char dat[50]={0};

    sprintf(dat,"LIST\r\n");
    APP_LOG(TS_OFF, DBLEVEL_M,"u_ftp: List: \"%s\"", dat);
    send(CTRL_SOCK, (uint8_t *)dat, strlen(dat));
                            
    return 1;
}

static uint8_t _fQuery_Size_File (void)
{
    char dat[50]={0};

    if (strlen(sAppEthVar.sUpdateVar.pPath) == 0) {
        sprintf(dat,"SIZE /%s\r\n",  sAppEthVar.sUpdateVar.pFileName);
    } else {
        sprintf(dat,"SIZE /%s/%s\r\n", (char *) sAppEthVar.sUpdateVar.pPath,
                                        (char *) sAppEthVar.sUpdateVar.pFileName);
    }
    
    APP_LOG(TS_OFF, DBLEVEL_M, "u_ftp: size file: \"%s\"", dat);
    
    send(CTRL_SOCK, (uint8_t *)dat, strlen(dat));
                            
    return 1;
}

static uint8_t _fSet_CWD (void)
{
    char dat[50]={0};
    
    sprintf(dat,"CWD /%s\r\n", (char *) sAppEthVar.sUpdateVar.pPath);
    APP_LOG(TS_OFF, DBLEVEL_M, "> path file: %s\r\n", dat);
    
    send(CTRL_SOCK, (uint8_t *)dat, strlen(dat));
    
    return 1;
}



static uint8_t _fSet_Pasv_Port (void)
{
    wiz_NetInfo gWIZNETINFO;
    char dat[50]={0};
    uint16_t  local_port = 35000;
    
    if (sFTPvar.dsock_mode == PASSIVE_MODE)
    {
        sprintf(dat, "PASV\r\n");
    } else
    {                      
        ctlnetwork(CN_GET_NETINFO, (void*) &gWIZNETINFO);   
        
        sprintf(dat, "PORT %d,%d,%d,%d,%d,%d\r\n",
                    gWIZNETINFO.ip[0], gWIZNETINFO.ip[1],
                    gWIZNETINFO.ip[2], gWIZNETINFO.ip[3], 
                    (uint8_t)(local_port>>8), (uint8_t)(local_port&0x00ff));
    }
    
    APP_LOG(TS_OFF, DBLEVEL_M, "u_ftp: %s", dat);
    
    send (CTRL_SOCK, (uint8_t *)dat, strlen(dat));
                            
    return 1;
}



static uint8_t _fSet_Get_File (void)
{
    char dat[80]={0};
    
    sprintf(dat,"RETR /%s/%s\r\n", (char *) sAppEthVar.sUpdateVar.pPath,
                                        (char *) sAppEthVar.sUpdateVar.pFileName);
    
    APP_LOG(TS_OFF, DBLEVEL_M, "u_ftp: \"%s\"", dat);
    
    send(CTRL_SOCK, (uint8_t *)dat, strlen(dat));
                            
    return 1;
}


//end cb cmd ethernet

void AppEth_Init_Default (void)
{
    sAppEthVar.Status_u8 = _ETH_START;
    sAppEthVar.PHYstatus_u8 = false;
    sAppEthVar.ReConnect_u8 = false;
    sAppEthVar.Retry_u8 = 0;
}


void AppEth_FTP_Init_Default (void)
{
    sFTPvar.DataSockConnected_u8 = false;
    sFTPvar.DataGetStart_u8 = false;
    sFTPvar.CtrlReady_u8 = false;
    sFTPvar.dsock_state = DATASOCK_IDLE;

    sFTPvar.LandMark_u32 = RtCountSystick_u32;
    sFTPvar.Retry_u8++;
}



int pportc(char * arg)
{
	int i;
	char* tok=0;
    
	strtok(arg,"(");
    
	for (i = 0; i < 4; i++)
	{
		if (i==0) 
            tok = strtok(NULL,",\r\n");
		else	 
            tok = strtok(NULL,",");
        
		sFTPvar.RemoteIP[i] = (uint8_t)atoi(tok);
        
		if (!tok)
        {
			APP_LOG(TS_OFF, DBLEVEL_M, "bad pport : %s\r\n", arg);
			return -1;
		}
	}
    
	sFTPvar.RemotePort = 0;
	for (i = 0; i < 2; i++)
    {
		tok = strtok(NULL,",\r\n");
        
		sFTPvar.RemotePort <<= 8;
		sFTPvar.RemotePort += atoi(tok);
        
		if (!tok)
        {
			APP_LOG(TS_OFF, DBLEVEL_M, "bad pport : %s\r\n", arg);
			return -1;
		}
	}
    
	APP_LOG(TS_OFF, DBLEVEL_M, "ip : %d.%d.%d.%d, port : %d\r\n", 
                    sFTPvar.RemoteIP[0], sFTPvar.RemoteIP[1],
                    sFTPvar.RemoteIP[2], sFTPvar.RemoteIP[3], sFTPvar.RemotePort);
	
    return 0;
}



uint8_t AppEth_FTP_DataSock_Run (void)
{       
    long ret = 0;
	uint32_t remain_datasize = 0;
    uint32_t recv_byte = 0;
    uint32_t LandMark_u32 = RtCountSystick_u32;
    
    if (sFTPvar.DataGetStart_u8 == true)
    {
        if ((remain_datasize = getSn_RX_RSR(DATA_SOCK)) > 0)
        {
            while (1)
            {
                if (remain_datasize > DATA_BUF_SIZE)
                    recv_byte = DATA_BUF_SIZE;
                else
                    recv_byte = remain_datasize;
                
                ret = recv(DATA_SOCK, aRECEIVE_DATA, recv_byte);
                
                if (ret < 0)
                    return false;
                
                UTIL_Printf( DBLEVEL_M, aRECEIVE_DATA, ret);
                
                if (AppEth_Save_Firmware(aRECEIVE_DATA, ret) == false)
                    return false;    
                
                remain_datasize -= ret;
                
                if (remain_datasize <= 0)
                    break;
                
                if (Check_Time_Out(LandMark_u32, 5000) == true)
                    return false;
            }
        } 
    }
    
    return true;
}

void AppEth_Default_Struct_UpdateVar (void)
{
    //Xoa vung nho update
    Erase_Firmware(ADDR_UPDATE_PROGRAM, FIRMWARE_SIZE_PAGE); 
    
    //Set default cho cac bien
    sAppEthVar.sUpdateVar.Addr_u32 = ADDR_UPDATE_PROGRAM;
    sAppEthVar.sUpdateVar.HeaderValid_u8 = false;
    sAppEthVar.sUpdateVar.cPacket_u8 = 0;
    sAppEthVar.sUpdateVar.Total_u8 = 0;
    sAppEthVar.sUpdateVar.FwSizeGet_u32 = 0;
    sAppEthVar.sUpdateVar.CrcFile_u8 = 0;
    sAppEthVar.sUpdateVar.CrcCalcu_u8 = 0;
    sAppEthVar.sUpdateVar.Status_u8 = 0;
}

uint8_t AppEth_Save_Firmware (uint8_t *pData, uint16_t length)
{
    static uint8_t aTEMP_BYTE[8] = {0};
    static uint8_t cByte = 0;
    sData strHeader;
    int Pos = -1;
    sData sSource = {pData, length};
    uint16_t i = 0, nByteAdd = 0;
    uint8_t aTEMP_DATA[8] = {0};
    
    if (sAppEthVar.sUpdateVar.cPacket_u8 == 0)
    {
        UTIL_MEM_set(aTEMP_BYTE, 0, 8);
        cByte = 0;
        
        //Check header fw sao viet
        strHeader.Data_a8 = (uint8_t *) sCbSimComm.pGet_Fw_Version();
        strHeader.Length_u16 = LENGTH_HEADER_FW;
        
        Pos = Find_String_V2((sData*)&strHeader, &sSource);  
        if (Pos >= 0)
            sAppEthVar.sUpdateVar.HeaderValid_u8 = true;
          
        if (sAppEthVar.sUpdateVar.HeaderValid_u8 == false)  
        {
            AppEth_Finish_Update(__FAIL_HEADER);
            
            return false;
        }
        
        //Lay byte cuoi cung ra: byte 32 cua Packdau tien. Sau header 31 byte
        sAppEthVar.sUpdateVar.CrcFile_u8 = *(sSource.Data_a8 + 31);
        //Fix string data
        sSource.Data_a8 += 32;
        sSource.Length_u16 -= 32;
        
        sAppEthVar.sUpdateVar.Total_u8 += 32;
    }

    //Cacal Crc
    for (i = 0; i < sSource.Length_u16; i++)
        sAppEthVar.sUpdateVar.CrcCalcu_u8 += *(sSource.Data_a8 + i);
              
    //Increase Count Packet
    sAppEthVar.sUpdateVar.cPacket_u8++;
    sAppEthVar.sUpdateVar.Total_u8 += sSource.Length_u16;
    
    //Neu so byte lon hon size: cong them cac byte le
    if ( sAppEthVar.sUpdateVar.Total_u8 >= sAppEthVar.sUpdateVar.FwSizeGet_u32 )
    {
        if ((sAppEthVar.sUpdateVar.Total_u8 % FLASH_BYTE_WRTIE) != 0)
        {
            nByteAdd = 2 * FLASH_BYTE_WRTIE - (sAppEthVar.sUpdateVar.Total_u8 % FLASH_BYTE_WRTIE);   //so byte de chan
            for (i = 0; i < nByteAdd; i++)
                *(sSource.Data_a8 + sSource.Length_u16++) = FLASH_BYTE_EMPTY;
            
            sAppEthVar.sUpdateVar.Total_u8 += nByteAdd;
        }
    }

    while (sSource.Length_u16 >= FLASH_BYTE_WRTIE)
    {
        //Lay file le truoc do truoc
        if (cByte != 0)
        {
            for (i = 0; i < cByte; i++)
                aTEMP_DATA[i] = aTEMP_BYTE[i];
            
            UTIL_MEM_set(aTEMP_BYTE, 0, 8);
        }
        
        for (i = cByte; i < FLASH_BYTE_WRTIE; i++)
        {
            aTEMP_DATA[i] = *(sSource.Data_a8++);
        }
        
        sSource.Length_u16 -= (FLASH_BYTE_WRTIE - cByte);
        cByte = 0;
        
        if(OnchipFlashWriteData(sAppEthVar.sUpdateVar.Addr_u32, &aTEMP_DATA[0], FLASH_BYTE_WRTIE) != HAL_OK)
        {
            AppEth_Finish_Update(__FAIL_FLASH);
            return false;
        } else
        {
            sAppEthVar.sUpdateVar.Addr_u32 += FLASH_BYTE_WRTIE;
        }
    }
    
    //copy toan bo byte le vao buff temp
    for (i = 0; i < sSource.Length_u16; i++)
        aTEMP_BYTE[cByte++] = *(sSource.Data_a8 + i);
    
    return true;
}


void AppEth_Finish_Update (uint8_t status)
{
    switch (status)
    {
        case __SUCCESSFUL:
            // Update thanh cong
        #ifdef USING_INTERNAL_MEM
            Erase_Firmware(ADDR_FLAG_HAVE_NEW_FW, 1);
            //Ghi Flag update va Size firm vao Inflash
            HAL_FLASH_Unlock();
            HAL_Delay(10);
            
        #if (FLASH_BYTE_WRTIE == 8)
            HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, 
                              ADDR_FLAG_HAVE_NEW_FW, 0xAA);
            HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, 
                              ADDR_FLAG_HAVE_NEW_FW + FLASH_BYTE_WRTIE, sAppEthVar.sUpdateVar.Total_u8);
        #else
            HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, 
                              ADDR_FLAG_HAVE_NEW_FW, 0xAA);
            HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, 
                              ADDR_FLAG_HAVE_NEW_FW + FLASH_BYTE_WRTIE, sAppEthVar.sUpdateVar.Total_u8);
        #endif
            
            HAL_Delay(10);
            HAL_FLASH_Lock();
        #endif
            
            sAppEthVar.sUpdateVar.Status_u8 = status;
            sMessage.aMESS_PENDING[SEND_UPDATE_FIRM_OK] = TRUE;
            //Init connect again
            UTIL_var.ModeConnNow_u8 = UTIL_var.ModeConnLast_u8;
            fevent_active(sEventAppEth, _EVENT_ETH_HARD_RESET);
            break;
        case __FAIL_CONNECT:
        case __FAIL_LENGTH:
        case __FAIL_HEADER:
        case __FAIL_FLASH:
            sAppEthVar.sUpdateVar.Status_u8 = status;
            sMessage.aMESS_PENDING[SEND_UPDATE_FIRM_FAIL] = TRUE;
            UTIL_var.ModeConnNow_u8 = UTIL_var.ModeConnLast_u8;
            fevent_active(sEventAppEth, _EVENT_ETH_HARD_RESET);
            break;
        default:
            break;
    }
}


