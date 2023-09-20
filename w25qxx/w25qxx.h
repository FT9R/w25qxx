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
#define W25QXX_WRITE_STATUS_REGISTER_TIME 15
#define W25QXX_PAGE_PROGRAM_TIME          3
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
typedef enum { w25q80 = 0x13, w25q16, w25q32, w25q64, w25q128 } w25qxx_Def;

/* Macro */
#define KB_TO_BYTE(KB)         (KB * 1024)
#define CS_HIGH(DEVICE_HANDLE) SET_BIT((DEVICE_HANDLE)->CS_Port->BSRR, (DEVICE_HANDLE)->CS_Pin)
#define CS_LOW(DEVICE_HANDLE)  SET_BIT((DEVICE_HANDLE)->CS_Port->BSRR, (DEVICE_HANDLE)->CS_Pin << 16)
#define ADDRESS_BYTES_SWAP(DEVICE_HANDLE, ADDRESS)              \
    DEVICE_HANDLE->addressBytes[0] = (uint8_t) (ADDRESS >> 16); \
    DEVICE_HANDLE->addressBytes[1] = (uint8_t) (ADDRESS >> 8);  \
    DEVICE_HANDLE->addressBytes[2] = (uint8_t) (ADDRESS >> 0)

typedef enum {
    W25QXX_SECTOR_ERASE_4KB,
    W25QXX_BLOCK_ERASE_32KB,
    W25QXX_BLOCK_ERASE_64KB,
    W25QXX_CHIP_ERASE
} eraseInstruction_t;

typedef enum { W25QXX_WAIT_NO, W25QXX_WAIT_DELAY, W25QXX_WAIT_BUSY } waitForTask_t;

typedef struct
{
    uint8_t ID[2];
    uint8_t statusRegister;
    uint8_t addressBytes[3];
    uint8_t CMD;
    SPI_HandleTypeDef *hspix;
    GPIO_TypeDef *CS_Port;
    uint16_t CS_Pin;
    uint32_t numberOfPages;
    ErrorStatus status;
} w25qxx_HandleTypeDef;

/**
 * @brief Checks if the device is available and determines the number of pages
 *
 * @param w25qxx_Handle: pointer to the device handle structure
 * @param SPIx: SPI1, SPI2 or SPI3
 * @param CS_Port: GPIOx
 * @param CS_Pin: GPIO_Pin_x
 * @return ErrorStatus
 */
ErrorStatus w25qxx_Init(w25qxx_HandleTypeDef *w25qxx_Handle, SPI_HandleTypeDef *hspix, GPIO_TypeDef *CS_Port,
                        uint16_t CS_Pin);

/**
 * @brief Writes data to w25qxx from external buffer
 *
 * @param w25qxx_Handle: pointer to the device handle structure
 * @param buf: pointer to external buffer, that contains the data to write
 * @param dataLength: number of bytes to write
 * @param address: page address to write (multiple of 256 bytes)
 * @param waitForTask: the way to ensure that operation is completed
 * @return ErrorStatus
 */
ErrorStatus w25qxx_Write(w25qxx_HandleTypeDef *w25qxx_Handle, const uint8_t *buf, uint16_t dataLength, uint32_t address,
                         bool trailingCRC, waitForTask_t waitForTask);

/**
 * @brief Reads data from w25qxx to external buffer
 *
 * @param w25qxx_Handle: pointer to the device handle structure
 * @param buf: pointer to external buffer, that will contain the received data
 * @param dataLength: number of bytes to read
 * @param address: page address to read (multiple of 256 bytes)
 * @return ErrorStatus
 */
ErrorStatus w25qxx_Read(w25qxx_HandleTypeDef *w25qxx_Handle, uint8_t *buf, uint16_t dataLength, uint32_t address,
                        bool trailingCRC);

/**
 * @brief Begins erase operation of sector, block or whole memory array
 *
 * @param w25qxx_Handle: pointer to the device handle structure
 * @param eraseInstruction: pages groups to be erased
 * @param address: start address of sector or block to be erased
 * @param waitForTask: the way to ensure that operation is completed
 * @return ErrorStatus
 */
ErrorStatus w25qxx_Erase(w25qxx_HandleTypeDef *w25qxx_Handle, eraseInstruction_t eraseInstruction, uint32_t address,
                         waitForTask_t waitForTask);

/**
 * @brief Checks the BUSY bit in status register 1
 *
 * @param w25qxx_Handle: pointer to the device handle structure
 * @return true: device is busy
 * @return false: device is free
 */
bool w25qxx_Busy(w25qxx_HandleTypeDef *w25qxx_Handle);

#endif
