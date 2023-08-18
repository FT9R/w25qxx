#ifndef W25QXX_INTERFACE_H
#define W25QXX_INTERFACE_H

#include "main.h" // target MCU header
#include <stdlib.h>

/* HAL SPIx definitions */
#ifdef USE_HAL_DRIVER
#define USE_SPI1
// #define USE_SPI2
// #define USE_SPI3
#endif

/* Driver interface switch */
#ifdef USE_HAL_DRIVER
#include "spi.h" // SPI_HandleTypeDef reference
#else
#include "Delay.h"
#include "SPI.h"
#endif

void w25qxx_SPI_Transmit(SPI_TypeDef *SPIx, const uint8_t *pBuffer, uint32_t lengthTX);
void w25qxx_SPI_Receive(SPI_TypeDef *SPIx, uint8_t *pBuffer, uint32_t lengthRX);
void w25qxx_Delay(const uint32_t _delay_ms);

#endif
