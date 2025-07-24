#pragma once

#include "w25qxx_Interface.h"

/* Macro */
#define W25QXX_PAGE_TO_SECTOR(PAGE)         ((PAGE) / (W25QXX_SECTOR_SIZE_4KB / W25QXX_PAGE_SIZE))
#define W25QXX_PAGE_TO_BLOCK_32KB(PAGE)     ((PAGE) / (W25QXX_BLOCK_SIZE_32KB / W25QXX_PAGE_SIZE))
#define W25QXX_PAGE_TO_BLOCK_64KB(PAGE)     ((PAGE) / (W25QXX_BLOCK_SIZE_64KB / W25QXX_PAGE_SIZE))
#define W25QXX_PAGE_TO_ADDRESS(PAGE)        ((uint32_t) (PAGE) * (W25QXX_PAGE_SIZE))
#define W25QXX_SECTOR_TO_ADDRESS(SECTOR)    ((uint32_t) (SECTOR) * W25QXX_SECTOR_SIZE_4KB)
#define W25QXX_BLOCK_32KB_TO_ADDRESS(BLOCK) ((uint32_t) (BLOCK) * W25QXX_BLOCK_SIZE_32KB)
#define W25QXX_BLOCK_64KB_TO_ADDRESS(BLOCK) ((uint32_t) (BLOCK) * W25QXX_BLOCK_SIZE_64KB)
#define W25QXX_KB_TO_BYTE(KB)               ((uint32_t) (KB) * 1024)

/* Device constants */
#define W25QXX_MANUFACTURER_ID 0xEF
#define W25QXX_PAGE_SIZE       256
#define W25QXX_SECTOR_SIZE_4KB (W25QXX_KB_TO_BYTE(4))
#define W25QXX_BLOCK_SIZE_32KB (W25QXX_KB_TO_BYTE(32))
#define W25QXX_BLOCK_SIZE_64KB (W25QXX_KB_TO_BYTE(64))

enum w25qxx_Device_e { W25Q80 = 0x13, W25Q16, W25Q32, W25Q64, W25Q128 };

/* Data types */
typedef enum w25qxx_WaitForTask_e { W25QXX_WAIT_NO, W25QXX_WAIT_DELAY, W25QXX_WAIT_BUSY } w25qxx_WaitForTask_t;

typedef enum w25qxx_CRC_e { W25QXX_CRC_NO, W25QXX_CRC } w25qxx_CRC_t;

typedef enum w25qxx_FastRead_e { W25QXX_FASTREAD_NO, W25QXX_FASTREAD } w25qxx_FastRead_t;

typedef enum w25qxx_SR_Behaviour_e { W25QXX_SR_NONVOLATILE, W25QXX_SR_VOLATILE } w25qxx_SR_Behaviour_t;

typedef enum w25qxx_EraseInstruction_e {
    W25QXX_SECTOR_ERASE_4KB,
    W25QXX_BLOCK_ERASE_32KB,
    W25QXX_BLOCK_ERASE_64KB,
    W25QXX_CHIP_ERASE
} w25qxx_EraseInstruction_t;

typedef enum w25qxx_Status_e {
    W25QXX_STATUS_NOLINK,
    W25QXX_STATUS_LINK,
    W25QXX_STATUS_INIT,
    W25QXX_STATUS_WRITE,
    W25QXX_STATUS_READ,
    W25QXX_STATUS_ERASE,
    W25QXX_STATUS_WRITE_SR,
    W25QXX_STATUS_READ_SR,
    W25QXX_STATUS_RESET,
    W25QXX_STATUS_BUSY,
    W25QXX_STATUS_READY,
    W25QXX_STATUS_UNDEFINED
} w25qxx_Status_t;

typedef enum w25qxx_Error_e {
    W25QXX_ERROR_NONE,
    W25QXX_ERROR_STATUS,
    W25QXX_ERROR_INITIALIZATION,
    W25QXX_ERROR_ARGUMENT,
    W25QXX_ERROR_ADDRESS,
    W25QXX_ERROR_SPI,
    W25QXX_ERROR_TIMEOUT,
    W25QXX_ERROR_CHECKSUM,
    W25QXX_ERROR_INSTRUCTION
} w25qxx_Error_t;

typedef struct w25qxx_HandleTypeDef_s {
    struct {
        w25qxx_Transfer_Status_t (*Receive)(uint8_t *pDataRx, uint16_t size, uint32_t timeout);
        w25qxx_Transfer_Status_t (*Transmit)(const uint8_t *pDataTx, uint16_t size, uint32_t timeout);
        void (*CS_Set)(w25qxx_CS_State_t newState);
    } interface;

    w25qxx_Status_t status;
    w25qxx_Error_t error;
    uint32_t numberOfPages;
    uint16_t frameLength;
    uint16_t CRC16;
    uint8_t frameBuf[W25QXX_PAGE_SIZE];
    uint8_t ID[2];
    uint8_t statusRegister;
    uint8_t CMD;
    uint8_t addressBytes[3];
} w25qxx_HandleTypeDef;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Links user-defined interface functions to a device handle
 * @param w25qxx_Handle pointer to the device handle structure
 * @param fpReceive pointer to the user-defined SPI receive function
 * @param fpTransmit pointer to the user-defined SPI transmit function
 * @param fpCS_Set pointer to the user-defined chip select set function
 * @return `w25qxx_Handle->error`
 */
w25qxx_Error_t w25qxx_Link(w25qxx_HandleTypeDef *w25qxx_Handle,
                           w25qxx_Transfer_Status_t (*fpReceive)(uint8_t *, uint16_t, uint32_t),
                           w25qxx_Transfer_Status_t (*fpTransmit)(const uint8_t *, uint16_t, uint32_t),
                           void (*fpCS_Set)(w25qxx_CS_State_t));

/**
 * @brief Checks if the device is available and determines the number of pages
 * @param w25qxx_Handle pointer to the device handle structure
 * @return `w25qxx_Handle->error`
 */
w25qxx_Error_t w25qxx_Init(w25qxx_HandleTypeDef *w25qxx_Handle);

/**
 * @brief Writes data to w25qxx from external buffer
 * @param w25qxx_Handle pointer to the device handle structure
 * @param buf pointer to external buffer, that contains the data to send
 * @param dataLength number of bytes to write (<= 254 in case of trailingCRC)
 * @param address page address to write (multiple of 256 bytes)
 * @param trailingCRC insert or not insert CRC at the end of frame
 * @param waitForTask the way to ensure that operation is completed
 * @return `w25qxx_Handle->error`
 */
w25qxx_Error_t w25qxx_Write(w25qxx_HandleTypeDef *w25qxx_Handle, const uint8_t *buf, uint16_t dataLength,
                            uint32_t address, w25qxx_CRC_t trailingCRC, w25qxx_WaitForTask_t waitForTask);

/**
 * @brief Reads data from w25qxx to external buffer
 * @param w25qxx_Handle pointer to the device handle structure
 * @param buf pointer to external buffer, that will contain the received data
 * @param dataLength number of bytes to read (<= 254 in case of trailingCRC)
 * @param address page address to read (multiple of 256 bytes)
 * @param trailingCRC compare or not compare CRC at the end of frame
 * @param fastRead set true if SPIclk > 50MHz
 * @return `w25qxx_Handle->error`
 */
w25qxx_Error_t w25qxx_Read(w25qxx_HandleTypeDef *w25qxx_Handle, uint8_t *buf, uint16_t dataLength, uint32_t address,
                           w25qxx_CRC_t trailingCRC, w25qxx_FastRead_t fastRead);

/**
 * @brief Begins erase operation of sector, block or whole memory array
 * @param w25qxx_Handle pointer to the device handle structure
 * @param eraseInstruction pages groups to be erased
 * @param address start address of page, sector or block to be erased
 * @param waitForTask the way to ensure that operation is completed
 * @note Address has to be 0 in case of chip erase
 * @return `w25qxx_Handle->error`
 */
w25qxx_Error_t w25qxx_Erase(w25qxx_HandleTypeDef *w25qxx_Handle, w25qxx_EraseInstruction_t eraseInstruction,
                            uint32_t address, w25qxx_WaitForTask_t waitForTask);

/**
 * @brief Writes a status byte from handle to device itself
 * @param w25qxx_Handle pointer to the device handle structure
 * @param statusRegisterx device target status register(1-3)
 * @param statusRegisterBehaviour keep or not the status register content after device reset
 * @return `w25qxx_Handle->error`
 */
w25qxx_Error_t w25qxx_WriteStatus(w25qxx_HandleTypeDef *w25qxx_Handle, uint8_t statusRegisterx,
                                  w25qxx_SR_Behaviour_t statusRegisterBehaviour);

/**
 * @brief Reads a status byte from device to its handle
 * @param w25qxx_Handle pointer to the device handle structure
 * @param statusRegisterx device target status register(1-3)
 * @return `w25qxx_Handle->error`
 */
w25qxx_Error_t w25qxx_ReadStatus(w25qxx_HandleTypeDef *w25qxx_Handle, uint8_t statusRegisterx);

/**
 * @brief Resets any device errors within handle
 * @param w25qxx_Handle pointer to the device handle structure
 * @return `w25qxx_Handle->error`
 */
w25qxx_Error_t w25qxx_ResetError(w25qxx_HandleTypeDef *w25qxx_Handle);

/**
 * @brief Reads status register 1 and returns device status
 * @param w25qxx_Handle pointer to the device handle structure
 * @param timeout timeout duration to wait for ready flag [ms]
 * @return Device status `W25QXX_STATUS_READY`/`W25QXX_STATUS_BUSY` or `W25QXX_STATUS_UNDEFINED`
 * @note If no `timeout` provided, then instant device ready/busy status returned
 */
w25qxx_Status_t w25qxx_BusyCheck(w25qxx_HandleTypeDef *w25qxx_Handle, uint32_t timeout);

#ifdef __cplusplus
}
#endif