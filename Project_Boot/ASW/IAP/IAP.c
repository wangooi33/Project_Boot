/* include -------------------------------------------------------------------*/
#include "IAP.h"

/* macro ---------------------------------------------------------------------*/

/* global variable -----------------------------------------------------------*/
BootInfo_t Boot_Info = 
{
	.FlashWriteOff = APP_START_ADDRESS,
};
FIFO_t Boot_FIFO;

/* local prototypes ----------------------------------------------------------*/

/* functions implementation --------------------------------------------------*/
void Bootloader_Init(void)
{
    Boot_Info.BootSysState = BootState_Init;
}
void IAP_ChangeUpdateFlag(uint8_t isEnable)
{
    uint32_t Data = (isEnable != 0U) ? APP_UPDATE_FLAG : 0U;
    AT24Cxx_Write(0,(uint8_t *)&Data,4);
}
uint8_t IAP_CheckUpdateFlag(void)
{
    uint32_t Flag = 0;
    AT24Cxx_Read(0,(uint8_t *)&Flag,4);
    return (Flag == APP_UPDATE_FLAG) ? 1 : 0;
}
void stmFlash_ErasePage(uint32_t Addr, size_t Size)
{
    FLASH_EraseInitTypeDef EraseHandle;
    uint32_t PageError = 0;

    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
    EraseHandle.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseHandle.PageAddress = Addr;
    EraseHandle.NbPages = (Size + FLASH_PAGE_SIZE - 1) / FLASH_PAGE_SIZE;

    if (HAL_FLASHEx_Erase(&EraseHandle, &PageError) == HAL_OK)
    {
        
    }
    
    HAL_FLASH_Lock();
}
void stmFlash_Write(uint32_t Addr, uint8_t *pBuf, uint16_t Length)
{
    uint16_t Data;
    if (Addr < APP_START_ADDRESS || Addr > APP_END_ADDRESS)
    {
        return;
    }

    HAL_FLASH_Unlock();
    for (uint16_t i = 0; i < Length; i += 2)
    {
        Data = pBuf[i];
        if ((i + 1) < Length)
        {
            Data |= ((uint16_t)pBuf[i + 1] << 8);
        }
        else
        {
            Data |= 0xFF00;      // 补齐最后一个字节
        }
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,Addr,Data);
        Addr += 2;
    }
    HAL_FLASH_Lock();
}
void IAP_JumpToApp(void)
{
    uint32_t appStack = *(uint32_t *)APP_START_ADDRESS;
    uint32_t appReset = *(uint32_t *)(APP_START_ADDRESS + 4);

    __disable_irq();

    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

    for(int i=0;i<8;i++)
    {
        NVIC->ICER[i] = 0xFFFFFFFF;
        NVIC->ICPR[i] = 0xFFFFFFFF;
    }

    SCB->VTOR = APP_START_ADDRESS;

    __set_MSP(appStack);

    __enable_irq();

    ((pFunction)appReset)();
}

void BootloaderTask(void)
{
    switch (Boot_Info.BootSysState)
    {
        case BootState_Init:
            if (IAP_CheckUpdateFlag())
            {
                Boot_Info.BootSysState = BootState_AppErase;
            }
            else
            {
                Boot_Info.BootSysState = BootState_JumpApp;
            }
            break;

        case BootState_AppErase:
            stmFlash_ErasePage(APP_START_ADDRESS,(APP_END_ADDRESS - APP_START_ADDRESS + 1));
            Boot_Info.BootSysState = BootState_AppWrite;
            break;
            
        case BootState_AppWrite:
			if (Boot_Info.OverFlag == 1)
			{
				Boot_Info.BootSysState = BootState_Finally;
			}
			if (Boot_FIFO.Ready == 1)
			{
				stmFlash_Write(Boot_Info.FlashWriteOff,Boot_FIFO.Buf,Boot_FIFO.DataLen);
				Boot_Info.FlashWriteOff += Boot_FIFO.DataLen;
				Boot_Info.WriteSum += Boot_FIFO.DataLen;
				Boot_FIFO.Ready = 0;
			}
            break;

        case BootState_Finally:
			IAP_ChangeUpdateFlag(0);
			Boot_Info.BootSysState = BootState_JumpApp;
            break;

        case BootState_JumpApp:
            IAP_JumpToApp();
            break;

        default:
            Boot_Info.BootSysState = BootState_Init;
            break;
    }
}
void IAP_RxProcess(uint8_t *pData, uint16_t Size)
{
    if (pData[0] == 0x3A && pData[1] == 0x3A)
    {
        switch (pData[2])
        {
            case 0xF2:
                Boot_Info.OverFlag = 1;
                break;
        }
    }
}

