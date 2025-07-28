#pragma once

#include "cmsis_os.h"
#include "spi.h"
#include "usart.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Data types */
typedef enum w25qxx_Transfer_Status_e {
    W25QXX_TRANSFER_SUCCESS,
    W25QXX_TRANSFER_ERROR,
} w25qxx_Transfer_Status_t;

typedef enum w25qxx_CS_State_e { W25QXX_CS_LOW, W25QXX_CS_HIGH } w25qxx_CS_State_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief SPI receive function template
 * @param handle pointer to the SPI handle
 * @param pDataRx pointer to data buffer
 * @param size amount of data to be received
 * @param timeout timeout duration
 * @return Status of the data transfer request operation
 * @note Timeout argument is optional, so the function return can be forced to success
 */
w25qxx_Transfer_Status_t w25qxx_SPI_Receive(void *handle, uint8_t *pDataRx, uint16_t size, uint32_t timeout);

/**
 * @brief SPI transmit function template
 * @param handle pointer to the SPI handle
 * @param pDataTx pointer to data buffer
 * @param size amount of data to be sent
 * @param timeout timeout duration
 * @return Status of the data transfer request operation
 * @note Timeout argument is optional, so the function return can be forced to success
 */
w25qxx_Transfer_Status_t w25qxx_SPI_Transmit(void *handle, const uint8_t *pDataTx, uint16_t size, uint32_t timeout);

/**
 * @brief The one of chip select function templates
 * @param newState new CS pin state
 */
void w25qxx_SPI1_CS0_Set(w25qxx_CS_State_t newState);

/**
 * @brief Provides a delay (in milliseconds)
 * @param ms specifies the delay time length, in milliseconds
 * @return The actual delay time achieved, in milliseconds
 */
uint32_t w25qxx_Delay(uint32_t ms);

/**
 * @brief Function used to print any debug messages
 * @param message the message to print
 * @note Do not use async print methods without queue
 */
void w25qxx_Print(const char *message);

#ifdef __cplusplus
}
#endif