#include "IAP.h"

#define BOOT_UPDATE_FLAG_EEADRESS	0x00
#define BOOT_UPDATE_FLAG_KEY		0xA52002A5

uint8_t Boot_OTAFlag = 0x00;

void IAP_Test(void)
{
	uint32_t Data = BOOT_UPDATE_FLAG_KEY;
	AT24Cxx_Write(BOOT_UPDATE_FLAG_EEADRESS,(uint8_t *)&Data,4);
}
	
void IAP_CheckUpdata(void)
{
	uint32_t UpdataFlag;
	AT24Cxx_Read(BOOT_UPDATE_FLAG_EEADRESS,(uint8_t *)&UpdataFlag,4);
	if ( UpdataFlag == 0xA52002A5 )
	{
		Boot_OTAFlag = 1;
	}
}
