
#include "board.h"
#include "ff.h"

const char *ff_errors [] = {
    "FR_OK",
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
    "FR_INVALID_PARAMETER"
};

void DBG_Init(void){

    __HAL_RCC_GPIOC_CLK_ENABLE();

    HAL_GPIO_WritePin(DBG_PIN_PORT, DBG_PIN, GPIO_PIN_RESET);

	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = DBG_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(DBG_PIN_PORT, &GPIO_InitStruct);    
}

void DGB_FF_Error(const char *fname, FRESULT err){
    if(err){
        printf("%s: (%d) %s\n", fname, err, ff_errors[err]);
    }
}

void DBG_DumpBuf(uint8_t *buf, uint32_t off, uint32_t size){
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


/**
 * Util stuff
 */
void DBG_DumpSector(uint32_t sector)
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

    DBG_DumpBuf(sector_data, sector, BLOCKSIZE);  
}
