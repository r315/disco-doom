
#include "board.h"
#include "d_main.h"
#include "i_video.h"
#include "doomdef.h"

#include "fatfs.h"

#include <errno.h>


i2cbus_t ext_i2cbus = {
    .write = INPUT_I2C_Write,
    .read = INPUT_I2C_Read
};

const char *fs_errors [] = 
{"FR_OK",
"FR_DISK_ERR",			
"FR_INT_ERR",				
"FR_NOT_READY",			
"FR_NO_FILE",				
"FR_NO_PATH",				
"FR_INVALID_NAME",		
"FR_DENIED",				
"FR_EXIST",				
"FR_INVALID_OBJECT",		
"FR_WRITE_PROTECTED",		
"FR_INVALID_DRIVE",		
"FR_NOT_ENABLED",			
"FR_NO_FILESYSTEM",		
"FR_MKFS_ABORTED",		
"FR_TIMEOUT",				
"FR_LOCKED",				
"FR_NOT_ENOUGH_CORE",		
"FR_TOO_MANY_OPEN_FILES",	
"FR_INVALID_PARAMETER"};

static void SystemClock_Config(void);
void Serial_Init(void);
void vc_putchar(char c);
uint32_t memavail(void);
void dumpBuf(uint8_t *buf, uint32_t off, uint32_t size);

void OnError_Handler(uint32_t condition)
{
    if (condition)
    {
        BSP_LED_On(LED1);
        while (1)
        {
            ;
        } /* Blocking on error */
    }
}

int32_t fatFsInit(void)
{

    if (FATFS_LinkDriver(&SD_Driver, SDPath) != 0)
    {
        printf("FATFS Link Driver fail\n");
        return -1;
    }

    FRESULT fr = f_mount(&SDFatFS, (TCHAR const *)SDPath, 1);

    if (fr != FR_OK)
    {
        printf("FATFS Fail to mount: %s [%s]\n",SDPath, fs_errors[fr]);
        return -1;
    }

    printf("FatFs succsessfully mounted, type: %d\n", SDFatFS.fs_type);

    //char buff[256];
    //strcpy(buff, "/");
    //scan_files(buff);

    //FATFS_UnLinkDriver(SDPath);
    return 0;
}


int32_t sdCardInit(void)
{   
    HAL_SD_CardInfoTypeDef ci;

    switch(BSP_SD_Init()){
        case MSD_OK:
            break;
        case MSD_ERROR_SD_NOT_PRESENT:
            printf("SD card not present\n");
            return -1;
        default:
            printf("Fail to init card\n");
            return -2;
    }
    printf("\nSd card successfully initialized\n");
    BSP_SD_GetCardInfo(&ci);
    printf("\tType: %x\n", (int)ci.CardType);
    printf("\tVersion: %x\n", (int)ci.CardVersion);
    printf("\tClass: %x\n", (int)ci.Class);
    printf("\tRelative address: %x\n", (int)ci.RelCardAdd);
    printf("\tNumber of blocks: %x, (%d)\n", (int)ci.BlockNbr, (int)ci.BlockNbr);
    printf("\tBlock Size: %d\n", (int)ci.BlockSize);
    printf("\tLogical Number of blocks: %x, (%d)\n", (int)ci.LogBlockNbr, (int)ci.LogBlockNbr);
    printf("\tLogical Block Size: %d\n\n", (int)ci.LogBlockSize);
    return 0;
}


int main(void)
{
    SCB_EnableICache();
    SCB_EnableDCache();

    HAL_Init();

    SystemClock_Config();

    Serial_Init();
    printf("\e[2J\r");

    BSP_LED_Init(LED1);
    BSP_LED_Init(LED2);

    if(sdCardInit()){
        OnError_Handler(true);
    }

    //SD_DumpSector(0);

    if(fatFsInit()){
        OnError_Handler(true);
    }

    INPUT_Init();

    uint32_t *dram = (uint32_t*)SDRAM_DEVICE_ADDR;

    printf("Memory available: %d\n", (int)memavail());
    printf("Memmory region %08X:%08X\n", (int)dram, (int)*dram);
    printf("Starting DOOM...\n\n");
    
    D_DoomMain();

    FATFS_UnLinkDriver(SDPath);

    while (1)
    {
    }
    return 0;
}

static void SystemClock_Config(void)
{
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_OscInitTypeDef RCC_OscInitStruct;
    HAL_StatusTypeDef ret = HAL_OK;

    /* Enable HSE Oscillator and activate PLL with HSE as source */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 25;
    RCC_OscInitStruct.PLL.PLLN = 400;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 9;
    RCC_OscInitStruct.PLL.PLLR = 7;

    ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
    if (ret != HAL_OK)
    {
        while (1)
        {
            ;
        }
    }

    /* Activate the OverDrive to reach the 216 MHz Frequency */
    ret = HAL_PWREx_EnableOverDrive();
    if (ret != HAL_OK)
    {
        while (1)
        {
            ;
        }
    }

    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
    RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7);
    if (ret != HAL_OK)
    {
        while (1)
        {
            ;
        }
    }
}

/**
 * Check if file is available and print information about it
 * called from D_IDVersion()
 */
int access(char *file, int mode)
{
    FRESULT fr;
    FILINFO fno;

    printf("Accessing file: %s\n",file);

    fr = f_stat(file+2, &fno);

    switch (fr)
    {

    case FR_OK:
        printf("Size: %lu\n", fno.fsize);
        printf("Timestamp: %u/%02u/%02u, %02u:%02u\n",
               (fno.fdate >> 9) + 1980, fno.fdate >> 5 & 15, fno.fdate & 31,
               fno.ftime >> 11, fno.ftime >> 5 & 63);
        printf("Attributes: %c%c%c%c%c\n\n",
               (fno.fattrib & AM_DIR) ? 'D' : '-',
               (fno.fattrib & AM_RDO) ? 'R' : '-',
               (fno.fattrib & AM_HID) ? 'H' : '-',
               (fno.fattrib & AM_SYS) ? 'S' : '-',
               (fno.fattrib & AM_ARC) ? 'A' : '-');
        return 0;

    case FR_NO_FILE:
        printf("does not exist.\n");
        break;

    default:
        printf("An error occured. (%d)\n", fr);
    }
    return -1;
}


/**
 * Util stuff
 */
void dumpSector(uint32_t sector)
{
    uint8_t sector_data[BLOCKSIZE];
    uint8_t sdRes;

    //res = BSP_SD_ReadBlocks_DMA((uint32_t*)sector_data, 0, 1);
    sdRes = BSP_SD_ReadBlocks((uint32_t *)sector_data, sector, 1, 1024);
    while (BSP_SD_GetCardState() != SD_TRANSFER_OK)
        ;

    if (sdRes != MSD_OK)
    {
        printf("Fail to read: %x\n", sdRes);
        return;
    }  

    dumpBuf(sector_data, sector, BLOCKSIZE);  
}


void dumpBuf(uint8_t *buf, uint32_t off, uint32_t size){
    for (uint32_t i = 0; i < size; i++)
    {
        if ((i & 0x0F) == 0)
        {
            putchar('\n');
            printf("%08X: ", (unsigned int)(off + i));
        }
        printf("%02X ", buf[i]);
    }
    putchar('\n');
}

FRESULT scanFiles (char* path)
{
    FRESULT res;
    DIR dir;
    UINT i;
    static FILINFO fno;

    res = f_opendir(&dir, path);                       /* Open the directory */
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            if (fno.fattrib & AM_DIR) {                    /* It is a directory */
                i = strlen(path);
                sprintf(&path[i], "/%s", fno.fname);
                res = scanFiles(path);                    /* Enter the directory */
                if (res != FR_OK) break;
                path[i] = 0;
            } else {                                       /* It is a file. */
                printf("%s/%s\n", path, fno.fname);
            }
        }
        f_closedir(&dir);
    }

    return res;
}