#include <errno.h>
#include "board.h"
#include "doomdef.h"
#include "common.h"
#include "i_video.h"
#include "d_main.h"
#include "fatfs.h"
#include "serial.h"

i2cbus_t ext_i2cbus = {
    .write = INPUT_I2C_Write,
    .read = INPUT_I2C_Read
};

static void SystemClock_Config(void);
static void disco_MpuConfig (void);
uint32_t memavail(void);
FRESULT fatFsInit(void);
void Serial_Init(void);

static void init_board(void)
{
    SCB_EnableICache();
    SCB_EnableDCache();

    HAL_Init();

    SystemClock_Config();

    BSP_SDRAM_Init();

    disco_MpuConfig();

    SERIAL_Init();
    printf("\e[2J\r");
    printf("\nCPU Clock: %dMHz \n", (int)(SystemCoreClock/1000000));

    printf("Memory region %08X:%08X\n", (int)SDRAM_DEVICE_ADDR, (int)(SDRAM_DEVICE_ADDR | SDRAM_DEVICE_SIZE));
    printf("Memory available: %d\n", (int)memavail());

    BSP_LED_Init(LED1);
    BSP_LED_Init(LED2);
}

static int count_args(const char *args){
    int n = 0;
    int len = 0;

    if(args == NULL){
        return 0;
    }

    while(*args){
        len++;
        if(*args == ' ' || *args == '\t'){
            n++;
            do{
                args++;
            }while(*args == ' ' || *args == '\t');            
        }else{
            args++;
        }
    }

    if(len){
        n++;
    }

    return n;
}

int main(void)
{
    FILE *fp;
    int argc = 0;
    char **argv = NULL;
    char *s;
#if 0
    char *argv[] = {
        "disco_doom",
        "-basedir",
        "doom",
        "doom1.wad",
        "-shdev",
        "-devparm"
        //"-autostart",
        //"-nomonsters"
    };

    argc = sizeof(argv) / sizeof(char*);
#endif

    init_board();

    if(fatFsInit()){
        OnError_Handler(true);
    }
    
    fp = fopen("doom.arg", "rb");

    if(fp){
        
        int size = __getsize(fp->_file);
        char *param = (char*)calloc(size, 1);

        if(param){
           fread(param, size, 1, fp);
        }
        
        fclose(fp);

        size = (count_args((const char*)param) + 1);
        argv = (char**)calloc(size, sizeof(char*));
        
        if(argv){
            argv[argc++] = (char*)"disco_doom";

            s = param;
            
            if(*s){
                argv[argc++] = s;
            }

            while(*s){
                if(*s == ' '){
                    *s++ = '\0';

                    while(*s == ' ' || *s == '\t'){
                        s++;
                    }

                    if(*s != '\0'){
                        argv[argc++] = s;
                    }
                }else{
                    s++;
                }
            }
        }
        
    }
    
    if(argv == NULL){
        argv = (char*[]){
            "disco_doom"           
        };
        argc = 1;
    }

    D_DoomMain(argc, argv);

    FATFS_UnLinkDriver(SDPath);

    while (1)
    {
    }
    return 0;
}

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

FRESULT fatFsInit(void)
{

    if (FATFS_LinkDriver(&SD_Driver, SDPath) != 0)
    {
        printf("FATFS Link Driver fail\n");
        return FR_DISK_ERR;
    }

    FRESULT fr = f_mount(&SDFatFS, (TCHAR const *)SDPath, 1);

    if (fr != FR_OK)
    {
        printf("FATFS Fail to mount: %s\n",SDPath);
        return fr;
    }

    printf("FatFs succsessfully mounted, type: %d\n", SDFatFS.fs_type);

    //char buff[256];
    //strcpy(buff, "/");
    //scan_files(buff);

    //FATFS_UnLinkDriver(SDPath);
    return FR_OK;
}

static void SystemClock_Config(void)
{
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_OscInitTypeDef RCC_OscInitStruct;

    /* Enable HSE Oscillator and activate PLL with HSE as source */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 25;
    RCC_OscInitStruct.PLL.PLLN = 400;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 8;
    RCC_OscInitStruct.PLL.PLLR = 7;

    OnError_Handler(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK);
    
    /* Activate the OverDrive to reach the 216 MHz Frequency */
    OnError_Handler(HAL_PWREx_EnableOverDrive() != HAL_OK);
    
    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
    RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

     OnError_Handler(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6) != HAL_OK);
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

    fr = f_stat(file, &fno);

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

void Error_Handler(void){
  //printf("%s, %s\n",__FILE__, __FUNCTION__);
  while(1){
      //asm("nop");
  }
}

int T_GetTick(void){
    return HAL_GetTick();
}

void T_Delay(int ms){
    HAL_Delay(ms);
}


static void disco_MpuConfig (void) 
{
  MPU_Region_InitTypeDef MPU_InitStruct;

  /* Disable the MPU */
  HAL_MPU_Disable();

  /* Configure the MPU attributes for SDRAM */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = SDRAM_DEVICE_ADDR;
  MPU_InitStruct.Size = MPU_REGION_SIZE_16MB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE; //MPU_ACCESS_NOT_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE; //MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE; //MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* Enable the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
  /* Enable div zero hard fault */
  SCB->CCR |= 0x10;
}