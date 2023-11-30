#ifndef W25QXX_H
#define W25QXX_H

#include "w25qxx_Interface.h"

/* Instruction Set */
#define W25QXX_CMD_WRITE_ENABLE              0x06
#define W25QXX_CMD_VOLATILE_SR_WRITE_ENABLE  0x50
#define W25QXX_CMD_WRITE_DISABLE             0x04
#define W25QXX_CMD_RELEASE_POWER_DOWN        0xAB
#define W25QXX_CMD_MANUFACTURER_DEVICE_ID    0x90
#define W25QXX_CMD_JEDEC_ID                  0x9F
#define W25QXX_CMD_READ_UNIQUE_ID            0x4B
#define W25QXX_CMD_READ_DATA                 0x03
#define W25QXX_CMD_FAST_READ                 0x0B
#define W25QXX_CMD_PAGE_PROGRAM              0x02
#define W25QXX_CMD_SECTOR_ERASE_4KB          0x20
#define W25QXX_CMD_BLOCK_ERASE_32KB          0x52
#define W25QXX_CMD_BLOCK_ERASE_64KB          0xD8
#define W25QXX_CMD_CHIP_ERASE                0xC7
#define W25QXX_CMD_READ_STATUS_REGISTER1     0x05
#define W25QXX_CMD_WRITE_STATUS_REGISTER1    0x01
#define W25QXX_CMD_READ_STATUS_REGISTER2     0x35
#define W25QXX_CMD_WRITE_STATUS_REGISTER2    0x31
#define W25QXX_CMD_READ_STATUS_REGISTER3     0x15
#define W25QXX_CMD_WRITE_STATUS_REGISTER3    0x11
#define W25QXX_CMD_READ_SFDP_REGISTER        0x5A
#define W25QXX_CMD_ERASE_SECURITY_REGISTER   0x44
#define W25QXX_CMD_PROGRAM_SECURITY_REGISTER 0x42
#define W25QXX_CMD_READ_SECURITY_REGISTER    0x48
#define W25QXX_CMD_GLOBAL_BLOCK_LOCK         0x7E
#define W25QXX_CMD_GLOBAL_BLOCK_UNLOCK       0x98
#define W25QXX_CMD_READ_BLOCK_LOCK           0x3D
#define W25QXX_CMD_INDIVIDUAL_BLOCK_LOCK     0x36
#define W25QXX_CMD_INDIVIDUAL_BLOCK_UNLOCK   0x39
#define W25QXX_CMD_ERASE_PROGRAM_SUSPEND     0x75
#define W25QXX_CMD_ERASE_PROGRAM_RESUME      0x7A
#define W25QXX_CMD_POWER_DOWN                0xB9
#define W25QXX_CMD_ENABLE_RESET              0x66
#define W25QXX_CMD_RESET_DEVICE              0x99

/* Timings [ms] */
#define W25QXX_PAGE_PROGRAM_TIME          3
#define W25QXX_WRITE_STATUS_REGISTER_TIME 15
#define W25QXX_SECTOR_ERASE_TIME_4KB      400
#define W25QXX_BLOCK_ERASE_TIME_32KB      1600
#define W25QXX_BLOCK_ERASE_TIME_64KB      2000
#define W25QXX_CHIP_ERASE_TIME            25000

/* Timeouts [ms] */
#define W25QXX_TX_TIMEOUT       100
#define W25QXX_RX_TIMEOUT       100
#define W25QXX_RESPONSE_TIMEOUT 100

/* Device constants */
#define W25QXX_MANUFACTURER_ID 0xEF
#define W25QXX_PAGE_SIZE       256
#define W25QXX_SECTOR_SIZE_4KB ((W25QXX_PAGE_SIZE) * 16)
#define W25QXX_BLOCK_SIZE_32KB ((W25QXX_PAGE_SIZE) * 128)
#define W25QXX_BLOCK_SIZE_64KB ((W25QXX_PAGE_SIZE) * 256)

enum w25qxx_Device_e { W25Q80 = 0x13, W25Q16, W25Q32, W25Q64, W25Q128 };

/* Macro */
#define KB_TO_BYTE(KB) ((KB) * 1024)
#define ADDRESS_BYTES_SWAP(DEVICE_HANDLE, ADDRESS)                  \
    (DEVICE_HANDLE)->addressBytes[0] = (uint8_t) ((ADDRESS) >> 16); \
    (DEVICE_HANDLE)->addressBytes[1] = (uint8_t) ((ADDRESS) >> 8);  \
    (DEVICE_HANDLE)->addressBytes[2] = (uint8_t) ((ADDRESS) >> 0)

/* Data types */
typedef enum w25qxx_EraseInstruction_e {
    W25QXX_SECTOR_ERASE_4KB,
    W25QXX_BLOCK_ERASE_32KB,
    W25QXX_BLOCK_ERASE_64KB,
    W25QXX_CHIP_ERASE
} w25qxx_EraseInstruction_t;

typedef enum w25qxx_WaitForTask_e { W25QXX_WAIT_NO, W25QXX_WAIT_DELAY, W25QXX_WAIT_BUSY } w25qxx_WaitForTask_t;

typedef enum w25qxx_Status_e {
    W25QXX_STATUS_NOLINK,
    W25QXX_STATUS_RESET,
    W25QXX_STATUS_READY,
    W25QXX_STATUS_BUSY_INIT,
    W25QXX_STATUS_BUSY_WRITE,
    W25QXX_STATUS_BUSY_READ,
    W25QXX_STATUS_BUSY_ERASE,
} w25qxx_Status_t;

typedef enum w25qxx_Error_e {
    W25QXX_ERROR_NONE,
    W25QXX_ERROR_REFERENCE_HANDLE,
    W25QXX_ERROR_STATUS_MATCH,
    W25QXX_ERROR_INITIALIZATION,
    W25QXX_ERROR_ARGUMENT,
    W25QXX_ERROR_ADDRESS,
    W25QXX_ERROR_SPI,
    W25QXX_ERROR_TIMEOUT,
    W25QXX_ERROR_MEM_MANAGE,
    W25QXX_ERROR_CHECKSUM,
    W25QXX_ERROR_INSTRUCTION
} w25qxx_Error_t;

typedef struct w25qxx_HandleTypeDef_s
{
    uint8_t ID[2];
    uint32_t numberOfPages;
    uint8_t statusRegister;
    uint8_t CMD;
    uint8_t addressBytes[3];
    uint16_t frameLength;
    uint16_t CRC16;
    w25qxx_Status_t status;
    w25qxx_Error_t error;
    struct
    {
        ErrorStatus (*Receive)(uint8_t *pDataRx, uint16_t size, uint32_t timeout);
        ErrorStatus (*Transmit)(uint8_t *pDataTx, uint16_t size, uint32_t timeout);
        void (*CS_Set)(w25qxx_CS_State_t newState);
        void (*Delay)(uint32_t ms);
    } interface;
} w25qxx_HandleTypeDef;

/**
 * @brief Links user-defined interface functions to a device handle
 * @param w25qxx_Handle: pointer to the device handle structure
 * @param fpReceive: pointer to the user-defined SPI receive function
 * @param fpTransmit: pointer to the user-defined SPI transmit function
 * @param fpCS_Set: pointer to the user-defined chip select set function
 * @param fpDelay: pointer to the user-defined delay[ms] function
 * @return Device error state
 */
w25qxx_Error_t w25qxx_Link(w25qxx_HandleTypeDef *w25qxx_Handle, ErrorStatus (*fpReceive)(uint8_t *, uint16_t, uint32_t),
                           ErrorStatus (*fpTransmit)(uint8_t *, uint16_t, uint32_t),
                           void (*fpCS_Set)(w25qxx_CS_State_t), void (*fpDelay)(uint32_t));

/**
 * @brief Checks if the device is available and determines the number of pages
 * @param w25qxx_Handle: pointer to the device handle structure
 * @return Device error state
 */
w25qxx_Error_t w25qxx_Init(w25qxx_HandleTypeDef *w25qxx_Handle);

/**
 * @brief Writes data to w25qxx from external buffer
 * @param w25qxx_Handle: pointer to the device handle structure
 * @param buf: pointer to external buffer, that contains the data to write
 * @param dataLength: number of bytes to write
 * @param address: page address to write (multiple of 256 bytes)
 * @param trailingCRC: insert or not insert CRC at the end of frame
 * @param waitForTask: the way to ensure that operation is completed
 * @return Device error state
 */
w25qxx_Error_t w25qxx_Write(w25qxx_HandleTypeDef *w25qxx_Handle, const uint8_t *buf, uint16_t dataLength,
                            uint32_t address, bool trailingCRC, w25qxx_WaitForTask_t waitForTask);

/**
 * @brief Reads data from w25qxx to external buffer
 * @param w25qxx_Handle: pointer to the device handle structure
 * @param buf: pointer to external buffer, that will contain the received data
 * @param dataLength: number of bytes to read
 * @param address: page address to read (multiple of 256 bytes)
 * @param trailingCRC: compare or not compare CRC at the end of frame
 * @param fastRead: set true if SPIclk > 50MHz
 * @return Device error state
 */
w25qxx_Error_t w25qxx_Read(w25qxx_HandleTypeDef *w25qxx_Handle, uint8_t *buf, uint16_t dataLength, uint32_t address,
                           bool trailingCRC, bool fastRead);

/**
 * @brief Begins erase operation of sector, block or whole memory array
 * @param w25qxx_Handle: pointer to the device handle structure
 * @param eraseInstruction: pages groups to be erased
 * @param address: start address of page, block or sector to be erased
 * @param waitForTask: the way to ensure that operation is completed
 * @return Device error state
 * @note Address has to be 0 in case of chip erase
 */
w25qxx_Error_t w25qxx_Erase(w25qxx_HandleTypeDef *w25qxx_Handle, w25qxx_EraseInstruction_t eraseInstruction,
                            uint32_t address, w25qxx_WaitForTask_t waitForTask);

/**
 * @brief Reads status register 1 and updates device status based on busy bit once
 * @param w25qxx_Handle: pointer to the device handle structure
 * @return Device error state
 */
w25qxx_Error_t w25qxx_BusyCheck(w25qxx_HandleTypeDef *w25qxx_Handle);

/**
 * @brief Reads status register 1 and updates device status based on busy bit continuously with timeout
 * @param w25qxx_Handle: pointer to the device handle structure
 * @param timeout: timeout for this operation
 * @return Device error state
 */
w25qxx_Error_t w25qxx_WaitWithTimeout(w25qxx_HandleTypeDef *w25qxx_Handle, uint32_t timeout);

/**
 * @brief Resets any device errors
 * @param w25qxx_Handle: pointer to the device handle structure
 * @return Device error state
 */
w25qxx_Error_t w25qxx_ResetError(w25qxx_HandleTypeDef *w25qxx_Handle);

#endif