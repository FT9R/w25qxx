#include "SPI_Interface.h"

void SPI_Transmit_Interface(SPI_TypeDef *SPIx, const uint8_t *pBuffer, uint32_t lengthTX)
{
	SPI_Transmit(SPIx, pBuffer, lengthTX);
}

void SPI_Receive_Interface(SPI_TypeDef *SPIx, uint8_t *pBuffer, uint32_t lengthRX)
{
	SPI_Receive(SPIx, pBuffer, lengthRX);
}