#ifndef W25QXX_INTERFACE_H
#define W25QXX_INTERFACE_H

#include "SPI.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define W25QXX_MIN_DELAY 2

extern volatile uint32_t uwTick;

/* Data types */
typedef enum w25qxx_Transfer_Status_e {
    W25QXX_TRANSFER_SUCCESS,
    W25QXX_TRANSFER_ERROR,
} w25qxx_Transfer_Status_t;

typedef enum w25qxx_CS_State_e { W25QXX_CS_LOW, W25QXX_CS_HIGH } w25qxx_CS_State_t;

/**
 * @brief The one of SPIx receive function template
 * @param pDataRx: pointer to data buffer
 * @param size: amount of data to be received
 * @param timeout: timeout duration
 * @return Status of the data transfer request operation
 * @note Timeout argument is not necessary, so the function return can be forced to success
 */
w25qxx_Transfer_Status_t w25qxx_SPI1_Receive(uint8_t *pDataRx, uint16_t size, uint32_t timeout);

/**
 * @brief The one of SPIx transmit function template
 * @param pDataTx: pointer to data buffer
 * @param size: amount of data to be sent
 * @param timeout: timeout duration
 * @return Status of the data transfer request operation
 * @note Timeout argument is not necessary, so the function return can be forced to success
 */
w25qxx_Transfer_Status_t w25qxx_SPI1_Transmit(uint8_t *pDataTx, uint16_t size, uint32_t timeout);

/**
 * @brief The one of chip select function template
 * @param newState: new CS pin state
 */
void w25qxx_CS0_Set(w25qxx_CS_State_t newState);

/**
 * @brief Provides minimum delay (in milliseconds) based on incremented variable
 * @param ms: specifies the delay time length, in milliseconds
 * @note In the default implementation, timer with 1kHz freq is the source of time base.
 * It is used to generate interrupts at regular time intervals where uwTick is incremented.
 * volatile uint32_t uwTick has to be declared with user source file
 */
void w25qxx_Delay(uint32_t ms);

#endif
