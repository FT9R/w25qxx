#ifndef W25QXX_H
#define W25QXX_H

#include "SPI.h"
#include "Delay.h"
#include <stdbool.h>

/* Timings [ms] */
#define WriteStatusRegisterTime 15
#define PageProgramTime 3
#define SectorEraseTime_4KB 400
#define BlockEraseTime_32KB 1600
#define BlockEraseTime_64KB 2000
#define ChipEraseTime 25000

#define PageSize 256
#define AddressBytesMask 0xFFFFFF
#define KBtoByte(KB) (KB * 1024)

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

typedef enum
{
	w25q80 = 0x13,
	w25q16 = 0x14,
	w25q32 = 0x15,
	w25q64 = 0x16,
	w25q128 = 0x17,
} w25qxx_Def;

/**
 * @brief  Check if the device is available and determine the number of pages
 * @param  SPIx: where x can be 1, 2 or 3 in SPI mode
 * @retval operation status (SUCCESS or ERROR)
 */
ErrorStatus w25qxx_Init(SPI_TypeDef *SPIx);

/**
* @brief  Write data to w25qxx from external buffer via SPIx
* @param  SPIx: where x can be 1, 2 or 3 in SPI mode
* @param  buf: pointer to external buffer, that contains the data to send
* @param	bufSize: size of external buffer, that contains the data to send
* @param	address: page address to write (multiple of 256 bytes)
* @param	waitForTask: the way to ensure that operation is completed
--------- @arg noWait: external routines has to provide necessary timings
--------- @arg delayWait: use delay to halt the main cycle while operation is completed
--------- @arg busyWait: check BUSY bit in status register 1 and halt main cycle while it is set to 1
* @retval operation status (SUCCESS or ERROR)
*/
ErrorStatus w25qxx_Write(SPI_TypeDef *SPIx, const uint8_t *buf, uint16_t bufSize, uint32_t address, uint8_t waitForTask);

/**
 * @brief  Read data from w25qxx to external buffer via SPIx
 * @param  SPIx: where x can be 1, 2 or 3 in SPI mode
 * @param  buf: pointer to external buffer, that contains the data to receive
 * @param	bufSize: size of external buffer, that contains the data to receive
 * @param	address: page address to read (multiple of 256 bytes)
 * @retval operation status (SUCCESS or ERROR)
 */
ErrorStatus w25qxx_Read(SPI_TypeDef *SPIx, uint8_t *buf, uint16_t bufSize, uint32_t address);

/**
 * @brief  Read the JEDEC assigned manufacturer ID and the specific device ID
 * @param  SPIx: where x can be 1, 2 or 3 in SPI mode
 * @param  ID_Buf[2]: pointer to external buffer, that contains the data to receive
 * @retval operation status (SUCCESS or ERROR)
 */
ErrorStatus w25qxx_ReadID(SPI_TypeDef *SPIx, uint8_t *ID_Buf);

/**
* @brief  Write a byte to the status register
* @param  SPIx: where x can be 1, 2 or 3 in SPI mode
* @param  statusRegisterx: number of status register
--------- @arg 1
--------- @arg 2
--------- @arg 3
* @param  status: pointer to the status byte to be written to the status register
* @retval operation status (SUCCESS or ERROR)
*/
ErrorStatus w25qxx_WriteStatus(SPI_TypeDef *SPIx, uint8_t statusRegisterx, const uint8_t *status);

/**
* @brief  Read a byte from the status register
* @param  SPIx: where x can be 1, 2 or 3 in SPI mode
* @param  statusRegisterx: number of status register
--------- @arg 1
--------- @arg 2
--------- @arg 3
* @param  status: pointer to a byte that will contain the data from the status register
* @retval none
*/
void w25qxx_ReadStatus(SPI_TypeDef *SPIx, uint8_t statusRegisterx, uint8_t *status);

/**
 * @brief  Check if IC is busy or not through the status register 1
 * @param  SPIx: where x can be 1, 2 or 3 in SPI mode
 * @retval true in case if IC is busy
 */
bool IS_Busy(SPI_TypeDef *SPIx);

/**
 * @brief  Halt main cycle while IC is busy with determined timeout
 * @param  SPIx: where x can be 1, 2 or 3 in SPI mode
 * @param  timeout: timeout[ms]
 * @retval operation status (SUCCESS or ERROR)
 * @note		retval = SUCCESS if IC is ready to any command
 * @note		retval = ERROR if timeout is expired
 */
ErrorStatus w25qxx_WaitWithTimeout(SPI_TypeDef *SPIx, uint32_t timeout);

/**
 * @brief  Enable write access prior to every write or erase operation
 * @param  SPIx: where x can be 1, 2 or 3 in SPI mode
 * @retval none
 */
void w25qxx_WriteEnable(SPI_TypeDef *SPIx);

/**
 * @brief  Disable write access manually
 * @param  SPIx: where x can be 1, 2 or 3 in SPI mode
 * @retval none
 */
void w25qxx_WriteDisable(SPI_TypeDef *SPIx);

/**
* @brief  Begin erase operation of sector, block or whole memory array
* @param  SPIx: where x can be 1, 2 or 3 in SPI mode
* @param  eraseInstruction: the way to erase
--------- @arg sectorErase_4KB
--------- @arg blockErase_32KB
--------- @arg blockErase_64KB
--------- @arg chipErase
* @param  address: start address of sector or block to be erased
* @param	waitForTask: the way to ensure that operation is completed
--------- @arg noWait: external routines has to provide necessary timings
--------- @arg delayWait: use delay to halt the main cycle while operation is completed
--------- @arg busyWait: check BUSY bit in status register 1 and halt main cycle while it is set to 1
* @retval operation status (SUCCESS or ERROR)
*/
ErrorStatus w25qxx_Erase(SPI_TypeDef *SPIx, eraseInstruction_Def eraseInstruction, uint32_t address, uint8_t waitForTask);

#endif