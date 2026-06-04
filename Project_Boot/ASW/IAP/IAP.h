#ifndef _IAP_H
#define _IAP_H
/* include -------------------------------------------------------------------*/
#include <stdbool.h>
#include <string.h>
#include "main.h"
#include "AT24Cxx.h"
#include "NM25Qxx.h"
#include "check.h"

/* macro ---------------------------------------------------------------------*/
#define APP_START_ADDRESS               0x08008000
#define APP_END_ADDRESS                 0x0807FFFF
#define APP_FLASH_PAGENUM               ((APP_END_ADDRESS - APP_START_ADDRESS + 1u) / FLASH_PAGE_SIZE)
#define APP_STACK_ADDRESS               0x20000000

#define APP_UPDATE_FLAG                 0x5AA55AA5u

#define RXAPPBUF_MAX_SIZE				512

/* enum ----------------------------------------------------------------------*/
typedef void (*pFunction)(void);

typedef enum
{
    BootState_Init,
    BootState_AppErase,
    BootState_AppWrite,
    BootState_Finally,
    BootState_JumpApp,
}BootSysState_t;

/* types ---------------------------------------------------------------------*/
typedef struct
{
    BootSysState_t BootSysState;
    uint32_t FlashWriteOff;
    uint32_t WriteSum;
	uint8_t OverFlag;
}BootInfo_t;

typedef struct
{
	uint8_t Buf[RXAPPBUF_MAX_SIZE];
	uint16_t DataLen;
	uint8_t Ready;
}FIFO_t;

/* constants -----------------------------------------------------------------*/
extern BootInfo_t Boot_Info;
extern FIFO_t Boot_FIFO;

/* global variable -----------------------------------------------------------*/

/* functions prototypes ------------------------------------------------------*/
void Bootloader_Init(void);
void BootloaderTask(void);
void IAP_RxProcess(uint8_t *pData, uint16_t Size);

#endif /* _IAP_H */
