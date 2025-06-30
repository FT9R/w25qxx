#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>

#ifdef __cplusplus
extern "C" {
#endif

#define W25QXX_MIN_DELAY 2
#define SPI_DEV          1
#define SPI_SPEED        500000
#define SPI1_CS0_PIN     5

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
w25qxx_Transfer_Status_t w25qxx_SPI1_Transmit(const uint8_t *pDataTx, uint16_t size, uint32_t timeout);

/**
 * @brief The one of chip select function templates
 * @param newState: new CS pin state
 */
void w25qxx_SPI1_CS0_Set(w25qxx_CS_State_t newState);

/**
 * @section General functions
 */

/**
 * @brief Provides minimum delay (in milliseconds)
 * @param ms: specifies the delay time length, in milliseconds
 */
void w25qxx_Delay(uint32_t ms);

/**
 * @brief Function used to print any debug messages
 * @param message: the message to print
 */
void w25qxx_Print(char *message);

#ifdef __cplusplus
}
#endif