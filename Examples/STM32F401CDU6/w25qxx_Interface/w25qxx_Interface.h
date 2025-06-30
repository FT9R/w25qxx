#pragma once

#include "spi.h"
#include "usart.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define W25QXX_MIN_DELAY      2
#define UART_TRANSMIT_TIMEOUT 1000
#define SPI1_CS0_PIN          SPI1_CS0_Pin
#define SPI1_CS0_PORT         SPI1_CS0_GPIO_Port
#define SPI1_CS1_PIN          SPI1_CS1_Pin
#define SPI1_CS1_PORT         SPI1_CS1_GPIO_Port
#define SPI1_CS2_PIN          SPI1_CS2_Pin
#define SPI1_CS2_PORT         SPI1_CS2_GPIO_Port
#define SPI1_CS3_PIN          SPI1_CS3_Pin
#define SPI1_CS3_PORT         SPI1_CS3_GPIO_Port

/* Macro */
#define TOGGLE_BIT(REG, BIT)                ((REG) ^= (BIT))
#define SET_BIT(REG, BIT)                   ((REG) |= (BIT))
#define CLEAR_BIT(REG, BIT)                 ((REG) &= ~(BIT))
#define READ_BIT(REG, BIT)                  ((REG) & (BIT))
#define CLEAR_REG(REG)                      ((REG) = (0x0))
#define WRITE_REG(REG, VAL)                 ((REG) = (VAL))
#define READ_REG(REG)                       ((REG))
#define MODIFY_REG(REG, CLEARMASK, SETMASK) WRITE_REG((REG), (((READ_REG(REG)) & (~(CLEARMASK))) | (SETMASK)))

/* Data types */
typedef enum w25qxx_Transfer_Status_e {
    W25QXX_TRANSFER_SUCCESS,
    W25QXX_TRANSFER_ERROR,
} w25qxx_Transfer_Status_t;

typedef enum w25qxx_CS_State_e { W25QXX_CS_LOW, W25QXX_CS_HIGH } w25qxx_CS_State_t;

/**
 * @section Handle related functions
 */

/**
 * @brief The one of SPIx receive function templates
 * @param pDataRx: pointer to data buffer
 * @param size: amount of data to be received
 * @param timeout: timeout duration
 * @return Status of the data transfer request operation
 * @note Timeout argument is not necessary, so the function return can be forced to success
 */
w25qxx_Transfer_Status_t w25qxx_SPI1_Receive(uint8_t *pDataRx, uint16_t size, uint32_t timeout);

/**
 * @brief The one of SPIx transmit function templates
 * @param pDataTx: pointer to data buffer
 * @param size: amount of data to be sent
 * @param timeout: timeout duration
 * @return Status of the data transfer request operation
 * @note Timeout argument is not necessary, so the function return can be forced to success
 */
w25qxx_Transfer_Status_t w25qxx_SPI1_Transmit(uint8_t *pDataTx, uint16_t size, uint32_t timeout);

/**
 * @brief 1st device chip select function template
 * @param newState: new CS pin state
 */
void w25qxx_SPI1_CS0_Set(w25qxx_CS_State_t newState);

/**
 * @brief 2nd device chip select function template
 * @param newState: new CS pin state
 */
void w25qxx_SPI1_CS1_Set(w25qxx_CS_State_t newState);

/**
 * @brief 3rd device chip select function template
 * @param newState: new CS pin state
 */
void w25qxx_SPI1_CS2_Set(w25qxx_CS_State_t newState);

/**
 * @brief 4th device chip select function template
 * @param newState: new CS pin state
 */
void w25qxx_SPI1_CS3_Set(w25qxx_CS_State_t newState);

/**
 * @section General functions
 */

/**
 * @brief Provides minimum delay (in milliseconds) based on incremented variable
 * @param ms: specifies the delay time length, in milliseconds
 * @note In the default implementation, timer with 1kHz freq is the source of time base.
 * It is used to generate interrupts at regular time intervals where uwTick is incremented.
 * `volatile uint32_t uwTick` has to be declared with user source file
 */
void w25qxx_Delay(uint32_t ms); // FIXME: do not use external var

/**
 * @brief Function to print any debug messages
 * @param message: message to print
 */
void w25qxx_Print(const uint8_t *message);

#ifdef __cplusplus
}
#endif