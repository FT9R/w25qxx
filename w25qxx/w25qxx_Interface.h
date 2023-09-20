#ifndef W25QXX_INTERFACE_H
#define W25QXX_INTERFACE_H

#include "SPI.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

void w25qxx_SPI_Transmit(SPI_HandleTypeDef *hspix, uint8_t *pData, uint16_t size, uint32_t timeout);
void w25qxx_SPI_Receive(SPI_HandleTypeDef *hspix, uint8_t *pData, uint16_t size, uint32_t timeout);
void w25qxx_Delay(uint32_t ms);

#endif
