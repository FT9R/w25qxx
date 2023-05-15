#ifndef SPI_INTERFACE_H
#define SPI_INTERFACE_H

#include "stm32f4xx.h"
#include "SPI.h"
#include <stdlib.h>

void SPI_Transmit_Interface(SPI_TypeDef *SPIx, const uint8_t *pBuffer, uint32_t lengthTX);
void SPI_Receive_Interface(SPI_TypeDef *SPIx, uint8_t *pBuffer, uint32_t lengthRX);

#endif