#include "w25qxx.h"

/* Private variables */
w25qxx_HandleTypeDef w25qxx_Handle;

/* Private function prototypes */
bool IS_Busy(SPI_TypeDef *SPIx);
ErrorStatus w25qxx_WaitWithTimeout(SPI_TypeDef *SPIx, uint32_t timeout);
void w25qxx_WriteEnable(SPI_TypeDef *SPIx);
void w25qxx_WriteDisable(SPI_TypeDef *SPIx);

ErrorStatus w25qxx_Init(SPI_TypeDef *SPIx)
{
	_delay_ms(100);

	/* Release from power-down */
	w25qxx_Handle.cmd = W25QXX_CMD_RELEASE_POWER_DOWN;
	CS0_Low;
	SPI_Transmit(SPIx, &w25qxx_Handle.cmd, 1);
	CS0_High;
	_delay_ms(1);
	/* Reset device*/
	w25qxx_Handle.cmd = W25QXX_CMD_ENABLE_RESET;
	CS0_Low;
	SPI_Transmit(SPIx, &w25qxx_Handle.cmd, 1);
	CS0_High;
	w25qxx_Handle.cmd = W25QXX_CMD_RESET_DEVICE;
	CS0_Low;
	SPI_Transmit(SPIx, &w25qxx_Handle.cmd, 1);
	CS0_High;
	_delay_ms(1);
	/* Get the ManufacturerID and DeviceID */
	w25qxx_ReadID(SPIx, w25qxx_Handle.ID);
	if (w25qxx_Handle.ID[0] != W25QXX_MANUFACTURER_ID)
		return ERROR;
	/* Determine number of pages */
	switch (w25qxx_Handle.ID[1])
	{
	case w25q80:
		w25qxx_Handle.numberOfPages = 8 * (KBtoByte(1) * KBtoByte(1) / W25QXX_PAGE_SIZE / 8);
		break;

	case w25q16:
		w25qxx_Handle.numberOfPages = 16 * (KBtoByte(1) * KBtoByte(1) / W25QXX_PAGE_SIZE / 8);
		break;

	case w25q32:
		w25qxx_Handle.numberOfPages = 32 * (KBtoByte(1) * KBtoByte(1) / W25QXX_PAGE_SIZE / 8);
		break;

	case w25q64:
		w25qxx_Handle.numberOfPages = 64 * (KBtoByte(1) * KBtoByte(1) / W25QXX_PAGE_SIZE / 8);
		break;

	case w25q128:
		w25qxx_Handle.numberOfPages = 128 * (KBtoByte(1) * KBtoByte(1) / W25QXX_PAGE_SIZE / 8);
		break;

	default:
		return ERROR; // Unsupported device
	}

	return SUCCESS;
}

ErrorStatus w25qxx_Write(SPI_TypeDef *SPIx, const uint8_t *buf, uint16_t bufSize, uint32_t address, uint8_t waitForTask)
{
	if ((bufSize == 0) || (bufSize > W25QXX_PAGE_SIZE))
		return ERROR; // 1-256 bytes data can be written
	if ((address % W25QXX_PAGE_SIZE) != 0)
		return ERROR; // Only first byte of the page can be pointed as the start byte
	if (address > ((W25QXX_PAGE_SIZE * w25qxx_Handle.numberOfPages) - bufSize))
		return ERROR;

	if (w25qxx_WaitWithTimeout(SPIx, 100) != SUCCESS)
		return ERROR;

	/* Page program */
	w25qxx_WriteEnable(SPIx);
	w25qxx_Handle.cmd = W25QXX_CMD_PAGE_PROGRAM;
	CS0_Low;
	SPI_Transmit(SPIx, &w25qxx_Handle.cmd, 1);
	/* A23-A0 - Start address of the desired page */
	w25qxx_Handle.addressBytes[0] = (uint8_t)(address >> 16);
	w25qxx_Handle.addressBytes[1] = (uint8_t)(address >> 8);
	w25qxx_Handle.addressBytes[2] = (uint8_t)(address);
	SPI_Transmit(SPIx, w25qxx_Handle.addressBytes, sizeof(w25qxx_Handle.addressBytes));
	/* Data write */
	SPI_Transmit(SPIx, buf, bufSize);
	CS0_High;

	if (waitForTask == delayWait)
		_delay_ms(W25QXX_PAGE_PROGRAM_TIME);
	else if (waitForTask == busyWait)
		while (IS_Busy(SPIx))
		{
		}

	return SUCCESS;
}

ErrorStatus w25qxx_Read(SPI_TypeDef *SPIx, uint8_t *buf, uint16_t bufSize, uint32_t address)
{
	if ((bufSize == 0) || (bufSize > W25QXX_PAGE_SIZE))
		return ERROR; // 1-256 bytes data can be read
	if ((address % W25QXX_PAGE_SIZE) != 0)
		return ERROR; // Only first byte of the page can be pointed as the start byte
	if (address > ((W25QXX_PAGE_SIZE * w25qxx_Handle.numberOfPages) - bufSize))
		return ERROR;

	if (w25qxx_WaitWithTimeout(SPIx, 100) != SUCCESS)
		return ERROR;

	/* Page read */
	w25qxx_Handle.cmd = W25QXX_CMD_READ_DATA;
	CS0_Low;
	SPI_Transmit(SPIx, &w25qxx_Handle.cmd, 1);
	/* A23-A0 - Start address of the desired page */
	w25qxx_Handle.addressBytes[0] = (uint8_t)(address >> 16);
	w25qxx_Handle.addressBytes[1] = (uint8_t)(address >> 8);
	w25qxx_Handle.addressBytes[2] = (uint8_t)(address);
	SPI_Transmit(SPIx, w25qxx_Handle.addressBytes, sizeof(w25qxx_Handle.addressBytes));
	/* Data read */
	SPI_Receive(SPIx, buf, bufSize);
	CS0_High;

	return SUCCESS;
}

ErrorStatus w25qxx_ReadID(SPI_TypeDef *SPIx, uint8_t *ID_Buf)
{
	if (w25qxx_WaitWithTimeout(SPIx, 100) != SUCCESS)
		return ERROR;

	uint8_t dummyByte = NULL;
	w25qxx_Handle.cmd = W25QXX_CMD_MANUFACTURER_DEVICE_ID;
	CS0_Low;
	SPI_Transmit(SPIx, &w25qxx_Handle.cmd, 1);
	SPI_Transmit(SPIx, &dummyByte, 1);
	SPI_Transmit(SPIx, &dummyByte, 1);
	SPI_Transmit(SPIx, &dummyByte, 1);
	SPI_Receive(SPIx, ID_Buf, 2);
	CS0_High;

	return SUCCESS;
}

ErrorStatus w25qxx_WriteStatus(SPI_TypeDef *SPIx, uint8_t statusRegisterx, const uint8_t *status)
{
	if (w25qxx_WaitWithTimeout(SPIx, 100) != SUCCESS)
		return ERROR;

	w25qxx_Handle.cmd = W25QXX_CMD_VOLATILE_SR_WRITE_ENABLE;
	CS0_Low;
	SPI_Transmit(SPIx, &w25qxx_Handle.cmd, 1);
	CS0_High;

	switch (statusRegisterx)
	{
	case 1:
		w25qxx_Handle.cmd = W25QXX_CMD_WRITE_STATUS_REGISTER1;
		CS0_Low;
		SPI_Transmit(SPIx, &w25qxx_Handle.cmd, 1);
		SPI_Transmit(SPIx, status, 1);
		CS0_High;

		break;

	case 2:
		w25qxx_Handle.cmd = W25QXX_CMD_WRITE_STATUS_REGISTER2;
		CS0_Low;
		SPI_Transmit(SPIx, &w25qxx_Handle.cmd, 1);
		SPI_Transmit(SPIx, status, 1);
		CS0_High;

		break;

	case 3:
		w25qxx_Handle.cmd = W25QXX_CMD_WRITE_STATUS_REGISTER3;
		CS0_Low;
		SPI_Transmit(SPIx, &w25qxx_Handle.cmd, 1);
		SPI_Transmit(SPIx, status, 1);
		CS0_High;

		break;
	}

	return SUCCESS;
}

void w25qxx_ReadStatus(SPI_TypeDef *SPIx, uint8_t statusRegisterx, uint8_t *status)
{
	switch (statusRegisterx)
	{
	case 1:
		w25qxx_Handle.cmd = W25QXX_CMD_READ_STATUS_REGISTER1;
		CS0_Low;
		SPI_Transmit(SPIx, &w25qxx_Handle.cmd, 1);
		SPI_Receive(SPIx, status, 1);
		CS0_High;

		break;

	case 2:
		w25qxx_Handle.cmd = W25QXX_CMD_READ_STATUS_REGISTER2;
		CS0_Low;
		SPI_Transmit(SPIx, &w25qxx_Handle.cmd, 1);
		SPI_Receive(SPIx, status, 1);
		CS0_High;

		break;

	case 3:
		w25qxx_Handle.cmd = W25QXX_CMD_READ_STATUS_REGISTER2;
		CS0_Low;
		SPI_Transmit(SPIx, &w25qxx_Handle.cmd, 1);
		SPI_Receive(SPIx, status, 1);
		CS0_High;

		break;
	}
}

bool IS_Busy(SPI_TypeDef *SPIx)
{
	uint8_t status;
	w25qxx_ReadStatus(SPIx, 1, &status);

	return READ_BIT(status, 0x01);
}

ErrorStatus w25qxx_WaitWithTimeout(SPI_TypeDef *SPIx, uint32_t timeout)
{
	while (true)
	{
		if (IS_Busy(SPIx))
		{
			timeout--;
			_delay_ms(1);
		}
		else
			return SUCCESS;

		if (timeout <= 0)
		{
			CS0_High;

			return ERROR;
		}
	}
}

void w25qxx_WriteEnable(SPI_TypeDef *SPIx)
{
	w25qxx_Handle.cmd = W25QXX_CMD_WRITE_ENABLE;
	CS0_Low;
	SPI_Transmit(SPIx, &w25qxx_Handle.cmd, 1);
	CS0_High;
}

void w25qxx_WriteDisable(SPI_TypeDef *SPIx)
{
	w25qxx_Handle.cmd = W25QXX_CMD_WRITE_DISABLE;
	CS0_Low;
	SPI_Transmit(SPIx, &w25qxx_Handle.cmd, 1);
	CS0_High;
}

ErrorStatus w25qxx_Erase(SPI_TypeDef *SPIx, eraseInstruction_Def eraseInstruction, uint32_t address, uint8_t waitForTask)
{
	if (w25qxx_WaitWithTimeout(SPIx, 100) != SUCCESS)
		return ERROR;

	switch (eraseInstruction)
	{
	case sectorErase_4KB:
		if ((address % KBtoByte(4)) != 0)
			return ERROR; // Incorrect start address for sector
		if (address > ((W25QXX_PAGE_SIZE * w25qxx_Handle.numberOfPages) - KBtoByte(4)))
			return ERROR; // The boundaries of erase operation beyond memory

		/* Sector Erase */
		w25qxx_WriteEnable(SPIx);
		w25qxx_Handle.cmd = W25QXX_CMD_SECTOR_ERASE_4KB;
		CS0_Low;
		SPI_Transmit(SPIx, &w25qxx_Handle.cmd, 1);
		/* A23-A0 - Start address of the desired sector */
		w25qxx_Handle.addressBytes[0] = (uint8_t)(address >> 16);
		w25qxx_Handle.addressBytes[1] = (uint8_t)(address >> 8);
		w25qxx_Handle.addressBytes[2] = (uint8_t)(address);
		SPI_Transmit(SPIx, w25qxx_Handle.addressBytes, sizeof(w25qxx_Handle.addressBytes));
		CS0_High;

		if (waitForTask == delayWait)
		{
			for (uint32_t i = 0; i < (W25QXX_SECTOR_ERASE_TIME_4KB / 100); i++)
			{
				_delay_ms(100);
			}
		}
		else if (waitForTask == busyWait)
		{
			while (IS_Busy(SPIx))
			{
			}
		}
		break;

	case blockErase_32KB:
		if ((address % KBtoByte(32)) != 0)
			return ERROR; // Incorrect start address for block
		if (address > ((W25QXX_PAGE_SIZE * w25qxx_Handle.numberOfPages) - KBtoByte(32)))
			return ERROR; // The boundaries of erase operation beyond memory

		/* Block Erase */
		w25qxx_WriteEnable(SPIx);
		w25qxx_Handle.cmd = W25QXX_CMD_BLOCK_ERASE_32KB;
		CS0_Low;
		SPI_Transmit(SPIx, &w25qxx_Handle.cmd, 1);
		/* A23-A0 - Start address of the desired block */
		w25qxx_Handle.addressBytes[0] = (uint8_t)(address >> 16);
		w25qxx_Handle.addressBytes[1] = (uint8_t)(address >> 8);
		w25qxx_Handle.addressBytes[2] = (uint8_t)(address);
		SPI_Transmit(SPIx, w25qxx_Handle.addressBytes, sizeof(w25qxx_Handle.addressBytes));
		CS0_High;

		if (waitForTask == delayWait)
		{
			for (uint32_t i = 0; i < (W25QXX_BLOCK_ERASE_TIME_32KB / 100); i++)
			{
				_delay_ms(100);
			}
		}
		else if (waitForTask == busyWait)
		{
			while (IS_Busy(SPIx))
			{
			}
		}
		break;

	case blockErase_64KB:
		if ((address % KBtoByte(64)) != 0)
			return ERROR; // Incorrect start address for block
		if (address > ((W25QXX_PAGE_SIZE * w25qxx_Handle.numberOfPages) - KBtoByte(64)))
			return ERROR; // The boundaries of erase operation beyond memory

		/* Block Erase */
		w25qxx_WriteEnable(SPIx);
		w25qxx_Handle.cmd = W25QXX_CMD_BLOCK_ERASE_64KB;
		CS0_Low;
		SPI_Transmit(SPIx, &w25qxx_Handle.cmd, 1);
		/* A23-A0 - Start address of the desired block */
		w25qxx_Handle.addressBytes[0] = (uint8_t)(address >> 16);
		w25qxx_Handle.addressBytes[1] = (uint8_t)(address >> 8);
		w25qxx_Handle.addressBytes[2] = (uint8_t)(address);
		SPI_Transmit(SPIx, w25qxx_Handle.addressBytes, sizeof(w25qxx_Handle.addressBytes));
		CS0_High;

		if (waitForTask == delayWait)
		{
			for (uint32_t i = 0; i < (W25QXX_BLOCK_ERASE_TIME_64KB / 100); i++)
			{
				_delay_ms(100);
			}
		}
		else if (waitForTask == busyWait)
		{
			while (IS_Busy(SPIx))
			{
			}
		}
		break;

	case chipErase:
		/* Chip Erase */
		w25qxx_WriteEnable(SPIx);
		w25qxx_Handle.cmd = W25QXX_CMD_CHIP_ERASE;
		CS0_Low;
		SPI_Transmit(SPIx, &w25qxx_Handle.cmd, 1);
		CS0_High;

		if (waitForTask == delayWait)
		{
			for (uint32_t i = 0; i < (W25QXX_CHIP_ERASE_TIME / 100); i++)
			{
				_delay_ms(100);
			}
		}
		else if (waitForTask == busyWait)
		{
			while (IS_Busy(SPIx))
			{
			}
		}
		break;

	default:
		return ERROR; //	Invalid erase instruction
	}

	return SUCCESS;
}