#include "main.h"

const uint8_t bufferWrite[] =
{1, 2, 3, 4, 5, 6, 7, 8, 9, 0,\
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J'};
uint8_t bufferRead[sizeof(bufferWrite)];
bool FLASH_access = false;


void main(void)
{
	asm("CPSID i");
	Sys_Init();
	IO_Init();
	SPIx_Init(SPI1, SPI_Mode_Master, SPI_BaudRatePrescaler_256);
	if (w25qxx_Init(SPI1) == SUCCESS) FLASH_access = true;
	asm("CPSIE i");

	if (FLASH_access)
	{
		//		w25qxx_Erase(SPI1, chipErase, NULL, busyWait);
		memset(bufferRead, NULL, sizeof(bufferRead));
		printf("\r\n First approach to read \r\n");
		w25qxx_Read(SPI1, bufferRead, sizeof(bufferRead), PAGE_ADDRESS);
		if (strncmp((const char*)bufferRead, (const char*)bufferWrite, sizeof(bufferRead)) == 0)
		{
			printf("Data already exist at page %i boundaries \r\n", PAGE);
		}
		else
		{
			printf("Data doesn't exist at page %i boundaries \r\n", PAGE);
			printf("Page programming...\r\n");
			w25qxx_Write(SPI1, bufferWrite, sizeof(bufferWrite), PAGE_ADDRESS, delayWait);
			printf("\r\n Second approach to read \r\n");
			w25qxx_Read(SPI1, bufferRead, sizeof(bufferRead), PAGE_ADDRESS);
			if (strncmp((const char*)bufferRead, (const char*)bufferWrite, sizeof(bufferRead)) == 0)
			{
				printf("Writing process success \r\n");
			}
			else printf("Writing process failure \r\n");
		}
	}
	else printf("Couldn't get any response from device \r\n");

	while(1);
}