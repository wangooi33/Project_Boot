#include "AT24Cxx.h"


void AT24Cxx_Init(void)
{
	IIC_Init();
}

uint8_t AT24Cxx_ReadByte(uint16_t addr)
{
	uint8_t xReturn = 0;
	IIC_Start();

	/* 根据不同的24CXX型号, 发送高位地址
	 * 1, 24C16以上的型号, 分2个字节发送地址
	 * 2, 24C16及以下的型号, 分1个低字节地址 + 占用器件地址的bit1~bit3位 用于表示高位地址, 最多11位地址
	 *    对于24C01/02, 其器件地址格式(8bit)为: 1  0  1  0  A2  A1  A0  R/W
	 *    对于24C04,    其器件地址格式(8bit)为: 1  0  1  0  A2  A1  a8  R/W
	 *    对于24C08,    其器件地址格式(8bit)为: 1  0  1  0  A2  a9  a8  R/W
	 *    对于24C16,    其器件地址格式(8bit)为: 1  0  1  0  a10 a9  a8  R/W
	 *    R/W      : 读/写控制位 0,表示写; 1,表示读;
	 *    A0/A1/A2 : 对应器件的1,2,3引脚(只有24C01/02/04/8有这些脚)
	 *    a8/a9/a10: 对应存储整列的高位地址, 11bit地址最多可以表示2048个位置,可以寻址24C16及以内的型号
	 */    
	if (EE_TYPE > AT24C16)							/* 24C16以上的型号, 分2个字节发送地址 */
	{
		IIC_SendByte(0XA0);							/* 发送写命令, IIC规定最低位是0, 表示写入 */
		IIC_WaitAck();								/* 每次发送完一个字节,都要等待ACK */
		IIC_SendByte(addr >> 8);					/* 发送高字节地址 */
	}
	else 
	{
		IIC_SendByte(0XA0 + ((addr >> 8) << 1));	/* 发送器件 0XA0 + 高位a8/a9/a10地址,写数据 */
	}
	IIC_WaitAck();
	IIC_SendByte(addr % 256);						/* 发送低位地址 */
	IIC_WaitAck();

	IIC_Start();									/* 重新发送起始信号 */ 
	IIC_SendByte(0XA1);								/* 进入接收模式, IIC规定最低位是0, 表示读取 */
	IIC_WaitAck();
	xReturn = IIC_ReadByte(0);
	IIC_Stop();
	return xReturn;
}

/**
 * @brief       在AT24CXX指定地址写入一个数据
 * @param       addr: 写入数据的目的地址
 * @param       data: 要写入的数据
 * @retval      无
 */
void AT24Cxx_WriteByte(uint16_t Addr, uint8_t Data)
{
	IIC_Start();

	if (EE_TYPE > AT24C16)
	{
		IIC_SendByte(0XA0);
		IIC_WaitAck();
		IIC_SendByte(Addr >> 8);
	}
	else 
	{
		IIC_SendByte(0XA0 + ((Addr >> 8) << 1));
	}

	IIC_WaitAck();
	IIC_SendByte(Addr % 256);
	IIC_WaitAck();

	/* 因为写数据的时候,不需要进入接收模式了,所以这里不用重新发送起始信号了 */
	IIC_SendByte(Data);
	IIC_WaitAck();
	IIC_Stop();
	HAL_Delay(10);
}
 
/**
 * @brief       检查AT24CXX是否正常
 *   @note      检测原理: 在器件的末地址写如0X55, 然后再读取, 如果读取值为0X55
 *              则表示检测正常. 否则,则表示检测失败.
 *
 * @param       无
 * @retval      检测结果
 *              0: 检测成功
 *              1: 检测失败
 */
uint8_t AT24Cxx_Check(void)
{
    uint8_t temp;
    uint16_t addr = EE_TYPE;
    temp = AT24Cxx_ReadByte(addr); /* 避免每次开机都写AT24CXX */

    if (temp == 0X55)
    {
        return 0;
    }
    else
    {
        AT24Cxx_WriteByte(addr, 0X55);
        temp = AT24Cxx_ReadByte(255);

        if (temp == 0X55)
		{
			return 0;
		}
    }

    return 1;
}


void AT24Cxx_Read(uint16_t Addr, uint8_t *pBuf, uint16_t Datalen)
{
	while (Datalen--)
	{
		*pBuf++ = AT24Cxx_ReadByte(Addr++);
	}
}

void AT24Cxx_Write(uint16_t Addr, uint8_t *pBuf, uint16_t Datalen)
{
	while (Datalen--)
	{
		AT24Cxx_WriteByte(Addr, *pBuf);
		Addr++;
		pBuf++;
	}
}


