#include "main.h"

w25qxx_HandleTypeDef w25qxx_Handle;
const uint8_t bufferWrite[] =
	{1, 2, 3, 4, 5, 6, 7, 8, 9, 0,
	 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J'};
uint8_t bufferRead[sizeof(bufferWrite)];

void main(void)
{
	asm("CPSID i");
	Sys_Init();
	IO_Init();
	SPIx_Init(SPI1, SPI_Mode_Master, SPI_BaudRatePrescaler_256);
	w25qxx_Init(&w25qxx_Handle, SPI1, CS0_GPIO_Port, CS0_Pin);
	memset(bufferRead, NULL, sizeof(bufferRead));
	asm("CPSIE i");

	if (w25qxx_Handle.status == SUCCESS)
	{
		// w25qxx_Erase(&w25qxx_Handle, chipErase, NULL, busyWait);
		printf("\r\n First approach to read \r\n");
		w25qxx_Read(&w25qxx_Handle, bufferRead, sizeof(bufferRead), PAGE_ADDRESS);
		if (strncmp((const char *)bufferRead, (const char *)bufferWrite, sizeof(bufferRead)) == 0)
		{
			printf("Data already exist at page %i boundaries \r\n", PAGE);
		}
		else
		{
			printf("Data doesn't exist at page %i boundaries \r\n", PAGE);
			printf("Page programming...");
			w25qxx_Write(&w25qxx_Handle, bufferWrite, sizeof(bufferWrite), PAGE_ADDRESS, delayWait);
			printf("\r\n Second approach to read \r\n");
			w25qxx_Read(&w25qxx_Handle, bufferRead, sizeof(bufferRead), PAGE_ADDRESS);
			if (strncmp((const char *)bufferRead, (const char *)bufferWrite, sizeof(bufferRead)) == 0)
			{
				printf("Writing process success \r\n");
			}
			else
				printf("Writing process failure \r\n");
		}
	}
	else
		printf("Couldn't get any response from device \r\n");

	while (true)
	{
	}
}