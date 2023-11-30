#ifndef W25QXX_INTERFACE_H
#define W25QXX_INTERFACE_H

#include "SPI.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// typedef enum { SUCCESS = 0U, ERROR = !SUCCESS } ErrorStatus;
typedef enum w25qxx_CS_State_e { W25QXX_CS_LOW = 0U, W25QXX_CS_HIGH } w25qxx_CS_State_t;

ErrorStatus w25qxx_SPI1_Receive(uint8_t *pDataRx, uint16_t size, uint32_t timeout);
ErrorStatus w25qxx_SPI1_Transmit(uint8_t *pDataTx, uint16_t size, uint32_t timeout);
void w25qxx_CS1_Set(w25qxx_CS_State_t newState);
void w25qxx_Delay(uint32_t ms);

#endif
