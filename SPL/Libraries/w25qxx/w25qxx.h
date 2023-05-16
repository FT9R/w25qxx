#ifndef W25QXX_H
#define W25QXX_H

#include "w25qxx_Interface.h"
#include <stdbool.h>

/* Instruction Set */
#define W25QXX_CMD_WRITE_ENABLE 0x06
#define W25QXX_CMD_VOLATILE_SR_WRITE_ENABLE 0x50
#define W25QXX_CMD_WRITE_DISABLE 0x04
#define W25QXX_CMD_RELEASE_POWER_DOWN 0xAB
#define W25QXX_CMD_MANUFACTURER_DEVICE_ID 0x90
#define W25QXX_CMD_JEDEC_ID 0x9F
#define W25QXX_CMD_READ_UNIQUE_ID 0x4B
#define W25QXX_CMD_READ_DATA 0x03
#define W25QXX_CMD_FAST_READ 0x0B
#define W25QXX_CMD_PAGE_PROGRAM 0x02
#define W25QXX_CMD_SECTOR_ERASE_4KB 0x20
#define W25QXX_CMD_BLOCK_ERASE_32KB 0x52
#define W25QXX_CMD_BLOCK_ERASE_64KB 0xD8
#define W25QXX_CMD_CHIP_ERASE 0xC7
#define W25QXX_CMD_READ_STATUS_REGISTER1 0x05
#define W25QXX_CMD_WRITE_STATUS_REGISTER1 0x01
#define W25QXX_CMD_READ_STATUS_REGISTER2 0x35
#define W25QXX_CMD_WRITE_STATUS_REGISTER2 0x31
#define W25QXX_CMD_READ_STATUS_REGISTER3 0x15
#define W25QXX_CMD_WRITE_STATUS_REGISTER3 0x11
#define W25QXX_CMD_READ_SFDP_REGISTER 0x5A
#define W25QXX_CMD_ERASE_SECURITY_REGISTER 0x44
#define W25QXX_CMD_PROGRAM_SECURITY_REGISTER 0x42
#define W25QXX_CMD_READ_SECURITY_REGISTER 0x48
#define W25QXX_CMD_GLOBAL_BLOCK_LOCK 0x7E
#define W25QXX_CMD_GLOBAL_BLOCK_UNLOCK 0x98
#define W25QXX_CMD_READ_BLOCK_LOCK 0x3D
#define W25QXX_CMD_INDIVIDUAL_BLOCK_LOCK 0x36
#define W25QXX_CMD_INDIVIDUAL_BLOCK_UNLOCK 0x39
#define W25QXX_CMD_ERASE_PROGRAM_SUSPEND 0x75
#define W25QXX_CMD_ERASE_PROGRAM_RESUME 0x7A
#define W25QXX_CMD_POWER_DOWN 0xB9
#define W25QXX_CMD_ENABLE_RESET 0x66
#define W25QXX_CMD_RESET_DEVICE 0x99

/* Timings [ms] */
#define W25QXX_WRITE_STATUS_REGISTER_TIME 15
#define W25QXX_PAGE_PROGRAM_TIME 3
#define W25QXX_SECTOR_ERASE_TIME_4KB 400
#define W25QXX_BLOCK_ERASE_TIME_32KB 1600
#define W25QXX_BLOCK_ERASE_TIME_64KB 2000
#define W25QXX_CHIP_ERASE_TIME 25000

/* Device constants */
#define W25QXX_MANUFACTURER_ID 0xEF
#define W25QXX_PAGE_SIZE 256
typedef enum
{
	w25q80 = 0x13,
	w25q16,
	w25q32,
	w25q64,
	w25q128,
} w25qxx_Def;

/* Macro */
#define KBtoByte(KB) (KB * 1024)
#define CS_HIGH(w25qxx_Handle) SET_BIT(w25qxx_Handle->CS_Port->ODR, w25qxx_Handle->CS_Pin)
#define CS_LOW(w25qxx_Handle) CLEAR_BIT(w25qxx_Handle->CS_Port->ODR, w25qxx_Handle->CS_Pin)

typedef enum
{
	sectorErase_4KB,
	blockErase_32KB,
	blockErase_64KB,
	chipErase
} eraseInstruction_Def;

typedef enum
{
	noWait,
	delayWait,
	busyWait
} waitForTask_Def;

typedef struct
{
	uint8_t ID[2];
	uint32_t numberOfPages;
	uint8_t cmd;
	uint8_t addressBytes[3];
	SPI_TypeDef *SPIx;
	GPIO_TypeDef *CS_Port;
	uint16_t CS_Pin;
	ErrorStatus status;
} w25qxx_HandleTypeDef;

/**
* @brief	Check if the device is available and determine the number of pages
* @param	w25qxx_Handle: pointer to the device handle structure
* @param	SPIx: where x can be 1, 2 or 3 in SPI mode
* @param	CS_Port: GPIOx
* @param	CS_Pin: GPIO_Pin_x
* @retval	operation status (SUCCESS or ERROR)
*/
ErrorStatus w25qxx_Init(w25qxx_HandleTypeDef *w25qxx_Handle, SPI_TypeDef *SPIx, GPIO_TypeDef *CS_Port, uint16_t CS_Pin);

/**
* @brief	Write data to w25qxx from external buffer
* @param	w25qxx_Handle: pointer to the device handle structure
* @param 	buf: pointer to external buffer, that contains the data to send
* @param	bufSize: size of external buffer, that contains the data to send
* @param	address: page address to write (multiple of 256 bytes)
* @param	waitForTask: the way to ensure that operation is completed
---------	@arg noWait: external routines have to provide necessary timings
---------	@arg delayWait: use delay to halt the main cycle while operation is completed
---------	@arg busyWait: check BUSY bit in status register 1 and halt main cycle while it is set to 1
* @retval	operation status (SUCCESS or ERROR)
*/
ErrorStatus w25qxx_Write(w25qxx_HandleTypeDef *w25qxx_Handle, const uint8_t *buf, uint16_t bufSize, uint32_t address, uint8_t waitForTask);

/**
* @brief	Read data from w25qxx to external buffer
* @param	w25qxx_Handle: pointer to the device handle structure
* @param	buf: pointer to external buffer, that contains the data to receive
* @param	bufSize: size of external buffer, that contains the data to receive
* @param	address: page address to read (multiple of 256 bytes)
* @retval	operation status (SUCCESS or ERROR)
*/
ErrorStatus w25qxx_Read(w25qxx_HandleTypeDef *w25qxx_Handle, uint8_t *buf, uint16_t bufSize, uint32_t address);

/**
* @brief	Write a byte to the status register
* @param	w25qxx_Handle: pointer to the device handle structure
* @param	statusRegisterx: number of status register
---------	@arg 1
---------	@arg 2
---------	@arg 3
* @param	status: pointer to the status byte to be written to the status register
* @retval	operation status (SUCCESS or ERROR)
*/
ErrorStatus w25qxx_WriteStatus(w25qxx_HandleTypeDef *w25qxx_Handle, uint8_t statusRegisterx, const uint8_t *status);

/**
* @brief	Read a byte from the status register
* @param	w25qxx_Handle: pointer to the device handle structure
* @param	statusRegisterx: number of status register
---------	@arg 1
---------	@arg 2
---------	@arg 3
* @param 	status: pointer to a byte that will contain the data from the status register
* @retval	none
*/
void w25qxx_ReadStatus(w25qxx_HandleTypeDef *w25qxx_Handle, uint8_t statusRegisterx, uint8_t *status);

/**
* @brief	Begin erase operation of sector, block or whole memory array
* @param	w25qxx_Handle: pointer to the device handle structure
* @param	eraseInstruction: the way to erase
---------	@arg sectorErase_4KB
---------	@arg blockErase_32KB
---------	@arg blockErase_64KB
---------	@arg chipErase
* @param	address: start address of sector or block to be erased
---------	@arg NULL in case if chipErase has been chosen
* @param	waitForTask: the way to ensure that operation is completed
---------	@arg noWait: external routines have to provide necessary timings
---------	@arg delayWait: use delay to halt the main cycle while operation is completed
---------	@arg busyWait: check BUSY bit in status register 1 and halt main cycle while it is set to 1
* @retval	operation status (SUCCESS or ERROR)
*/
ErrorStatus w25qxx_Erase(w25qxx_HandleTypeDef *w25qxx_Handle, eraseInstruction_Def eraseInstruction, uint32_t address, uint8_t waitForTask);

#endif