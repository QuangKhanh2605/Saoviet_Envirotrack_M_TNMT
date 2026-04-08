


#include "user_sd.h"
#include "sd_functions.h"
#include "stdio.h"
#include "sd_benchmark.h"
#include "sd_spi.h"
#include "user_define.h"
#include "user_convert_variable.h"

static uint8_t _Cb_Sd_Write (uint8_t event);
static uint8_t _Cb_Sd_Read (uint8_t event);
static uint8_t _Cb_Sd_Test (uint8_t event);

static uint8_t _Cb_Sd_Check (uint8_t event);

/*======== struct ===============*/
sEvent_struct sEventSD[] =
{
    { _EVENT_SD_TEST, 	    0, 0, 2000,  			_Cb_Sd_Test   },
	{ _EVENT_SD_WRITE, 	    0, 0, 200,  			_Cb_Sd_Write  },
	{ _EVENT_SD_READ, 	    0, 0, 200,  			_Cb_Sd_Read   },
    
    { _EVENT_SD_CHECK,      1, 5, 10000,            _Cb_Sd_Check  },
};


//static uint8_t aDATA_SD_READ[MAX_MEM_DATA];
extern UART_HandleTypeDef huart1;

uint32_t SD_Free_u32 = 0;

/*------------ Func Callback --------------*/

int _write(int fd, unsigned char *buf, int len) {
  if (fd == 1 || fd == 2) {                     // stdout or stderr ?
    HAL_UART_Transmit(&huart1, buf, len, 999);  // Print to the UART
  }
  return len;
}

    uint32_t t1 = 0;
    uint32_t t2 = 0;
static uint8_t _Cb_Sd_Write (uint8_t event)
{
    char buffer[256];
    UINT bytes_read;
    int result;
    int space_kb;

    Send_Uart("\r\n===== SD TEST START =====\r\n");

//    // 1. Mount SD
//    result = sd_mount();
//    if (result != 0)
//    {
//        Send_Uart("Mount failed!\r\n");
//        return 1;
//    }
//    Send_Uart("Mount OK\r\n");

    // 2. Write file
    result = sd_write_file("test.txt", "Hello STM32 SD Card!\r\n");

    if (result == 0)
        Send_Uart("Write OK\r\n");
    else
        Send_Uart("Write failed\r\n");

    // 3. Append file
    result = sd_append_file("test.txt", "Append line...\r\n");
    if (result == 0)
        Send_Uart("Append OK\r\n");
    else
        Send_Uart("Append failed\r\n");

    // 4. Read file
    result = sd_read_file("test.txt", buffer, sizeof(buffer), &bytes_read);
    if (result == 0)
    {
        buffer[bytes_read] = '\0';
        Send_Uart("Read OK:\r\n");
        Send_Uart(buffer);
    }
    else
        Send_Uart("Read failed\r\n");


    // 5. Create directory
    if (sd_create_directory("logs") == FR_OK)
        Send_Uart("Directory created\r\n");
    else
        Send_Uart("Directory create failed\r\n");

    // 6. Rename file
    result = sd_rename_file("test.txt", "logs/test2.txt");
    if (result == 0)
        Send_Uart("Rename OK\r\n");
    else
        Send_Uart("Rename failed\r\n");

    // 7. List files
    Send_Uart("\r\nListing files:\r\n");
    sd_list_files();

//    BYTE work[4096];
//    FRESULT res;

    // Unmount tru?c
//    f_mount(NULL, "", 1);
//
//    // Format
//    res = f_mkfs("", FM_ANY, 0, work, sizeof(work));
//
//    if (res == FR_OK)
//    {
//        Send_Uart("Format OK\r\n");
//    }
//    else
//    {
//        Send_Uart("Format failed\r\n");
//    }
    // 8. Get free space
    space_kb = sd_get_space_kb();
    sprintf(buffer, "Free space: %d KB\r\n", space_kb);
    Send_Uart(buffer);

    // 9. Delete file
    result = sd_delete_file("logs/test2.txt");
    if (result == 0)
        Send_Uart("Delete OK\r\n");
    else
        Send_Uart("Delete failed\r\n");

    // 10. Unmount
    sd_unmount();
    Send_Uart("Unmount OK\r\n");

    Send_Uart("===== SD TEST END =====\r\n");

    return 1;
}


static uint8_t _Cb_Sd_Read (uint8_t event)
{
    return 1;
}

char pathtest[20] = "vemo";
uint32_t cFile = 0;

static uint8_t _Cb_Sd_Test (uint8_t event)
{
    char aName[20] = {0};
    char bufr[80] = {0};
    UINT br;
    
    cFile++;
    sprintf (aName, "%s_%d.txt", pathtest, cFile);
    
//  sd_mount();
//  sd_list_files();
//  sd_unmount();

//  sd_mount();
//  sd_read_file("F1/F1F2/File5.TXT", bufr, 50, &br);
//  printf("DATA from File:::: %s\n\n",bufr);
//  sd_unmount();

//  sd_mount();
//  sd_read_file("File1.TXT", bufr, 80, &br);
//  printf("DATA from File:::: %s\n\n",bufr);
  sd_unmount();
  sd_mount();
    t1 = HAL_GetTick();

  UTIL_Printf_Str(DBLEVEL_M, "begin...\r\n");
//  sd_write_file(aName, "This file is created by the cubeIDE\n");
  sd_write_file(aName, "This file is created by\n");
  sd_read_file(aName, bufr, 80, &br);
  UTIL_Printf_Str(DBLEVEL_M, bufr);
  UTIL_Printf_Str(DBLEVEL_M, "\r\n");
//  sd_list_files();
//  sd_unmount();

    fevent_enable(sEventSD, event);
    t2 = HAL_GetTick();

    t2 = t2 - t1;

    return 1;
}

static uint8_t _Cb_Sd_Check (uint8_t event)
{
	FATFS *pfs;
	DWORD fre_clust, fre_sect, free_kb;
	FRESULT res = f_getfree(sd_path, &fre_clust, &pfs);
	if (res == FR_OK && SD_Check() == 1) 
    {
        fre_sect = fre_clust * pfs->csize;
        free_kb = fre_sect / 2;
        
        SD_Free_u32 = (uint32_t)(free_kb/1024);
    }
    else
    {
        SD_Free_u32 = 0;
        sd_unmount();
        sd_mount();
    }

    fevent_enable(sEventSD, event);
    return 1;
}



uint8_t SD_Check(void)
{
    uint8_t buffer[512];

    if (SD_ReadBlocks(buffer, 0, 1) == SD_OK)
        return 1;   // Module + the OK
    else
        return 0;   // Mat module hoac mat the
}

/*------------ Func Handle ------------*/

void SD_Card_Init (void)
{
    sd_mount();
    
}
/*======================Handle Define AT command===================*/
#ifdef USING_AT_CONFIG
void AT_CMD_Get_SD_Card_Free(sData *str, uint16_t Pos)
{
    uint8_t aTemp[50] = "SD_Card_Free: ";   //13 ki tu dau tien
    sData StrResp = {&aTemp[0], 14}; 

    Convert_Uint64_To_StringDec (&StrResp, (uint64_t) (SD_Free_u32), 0);
    
    Insert_String_To_String(StrResp.Data_a8, &StrResp.Length_u16, (uint8_t*)" MB",0 , 3);

	Modem_Respond(PortConfig, StrResp.Data_a8, StrResp.Length_u16, 0);
}
#endif
/*======================Handle Task and Init app====================*/
void Init_AppSDcard (void)
{
#ifdef USING_AT_CONFIG
    /* regis cb serial */
    sATCmdList[_GET_SD_CARD_FREE].CallBack = AT_CMD_Get_SD_Card_Free;
#endif
}

uint8_t SD_Card_Task (void)
{
    uint8_t i = 0;
	uint8_t Result = false;

	for (i = 0; i < _EVENT_SD_END; i++)
	{
		if (sEventSD[i].e_status == 1)
		{
            Result = true;
            
			if ((sEventSD[i].e_systick == 0) ||
					((HAL_GetTick() - sEventSD[i].e_systick)  >=  sEventSD[i].e_period))
			{
                sEventSD[i].e_status = 0; 
				sEventSD[i].e_systick = HAL_GetTick();
				sEventSD[i].e_function_handler(i);
			}
		}
	}

	return Result;
}










