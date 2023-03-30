#include "w25qxx.h"

const uint8_t WriteEnable = 0x06;
const uint8_t VolatileSR_WriteEnable = 0x50;
const uint8_t WriteDisable = 0x04;
const uint8_t PowerDown = 0xB9;
const uint8_t ReleasePowerDown = 0xAB;
const uint8_t ReadManufacturerDeviceID = 0x90;
const uint8_t JEDEC_ID = 0x9F;
const uint8_t ReadUniqueID = 0x4B;
const uint8_t ReadData = 0x03;
const uint8_t FastRead = 0x0B;
const uint8_t PageProgram = 0x02;
const uint8_t SectorErase_4KB = 0x20;
const uint8_t BlockErase_32KB = 0x52;
const uint8_t BlockErase_64KB = 0xD8;
const uint8_t ChipErase = 0xC7;
const uint8_t ReadStatusRegister1 = 0x05;
const uint8_t WriteStatusRegister1 = 0x01;
const uint8_t ReadStatusRegister2 = 0x35;
const uint8_t WriteStatusRegister2 = 0x31;
const uint8_t ReadStatusRegister3 = 0x15;
const uint8_t WriteStatusRegister3 = 0x11;
const uint8_t EnableReset = 0x66;
const uint8_t ResetDevice = 0x99;
/* Maintenance */
const uint8_t ManufacturerID = 0xEF;
const uint8_t DummyByte = NULL;
uint32_t numberOfPages;
uint32_t addressBytes;
uint8_t threeAddrBytes[3];
uint8_t w25qxx_ID[2];

ErrorStatus w25qxx_Init(SPI_TypeDef *SPIx)
{
	_delay_ms(100);

	/* Release from power-down */
	CS0_Low;
	SPI_Transmit(SPIx, &ReleasePowerDown, 1);
	CS0_High;
	_delay_us(3);
	/* Reset device*/
	CS0_Low;
	SPI_Transmit(SPIx, &EnableReset, 1);
	CS0_High;
	_delay_us(3);
	CS0_Low;
	SPI_Transmit(SPIx, &ResetDevice, 1);
	CS0_High;
	_delay_us(30);
	/* Get the ManufacturerID and DeviceID */
	w25qxx_ReadID(SPIx, w25qxx_ID);
	if (w25qxx_ID[0] != ManufacturerID)
		return ERROR;
	/* Determine number of pages */
	switch (w25qxx_ID[1])
	{
	case w25q80:
		numberOfPages = 8 * (KBtoByte(1) * KBtoByte(1) / PageSize / 8);
		break;

	case w25q16:
		numberOfPages = 16 * (KBtoByte(1) * KBtoByte(1) / PageSize / 8);
		break;

	case w25q32:
		numberOfPages = 32 * (KBtoByte(1) * KBtoByte(1) / PageSize / 8);
		break;

	case w25q64:
		numberOfPages = 64 * (KBtoByte(1) * KBtoByte(1) / PageSize / 8);
		break;

	case w25q128:
		numberOfPages = 128 * (KBtoByte(1) * KBtoByte(1) / PageSize / 8);
		break;

	default:
		return ERROR; // unsupported device
		break;
	}

	return SUCCESS;
}

ErrorStatus w25qxx_Write(SPI_TypeDef *SPIx, const uint8_t *buf, uint16_t bufSize, uint32_t address, uint8_t waitForTask)
{
	if ((bufSize == 0) || (bufSize > PageSize))
		return ERROR; // 1-256 bytes data can be written
	if ((address % PageSize) != 0)
		return ERROR; // only first byte of the page can be pointed as the start byte
	if (address > ((PageSize * numberOfPages) - 1))
		return ERROR;

	if (w25qxx_WaitWithTimeout(SPIx, 100) != SUCCESS)
		return ERROR;

	w25qxx_WriteEnable(SPIx);
	/* Page program */
	CS0_Low;
	SPI_Transmit(SPIx, &PageProgram, 1);
	/* A23-A0 - Start address of the desired page */
	MODIFY_REG(addressBytes, AddressBytesMask, (address & AddressBytesMask));
	threeAddrBytes[0] = (uint8_t)(addressBytes >> 16);
	threeAddrBytes[1] = (uint8_t)(addressBytes >> 8);
	threeAddrBytes[2] = (uint8_t)(addressBytes);
	SPI_Transmit(SPIx, threeAddrBytes, sizeof(threeAddrBytes));
	/* Data write */
	SPI_Transmit(SPIx, buf, bufSize);
	CS0_High;
	_delay_us(3);
	if (waitForTask == delayWait)
		_delay_ms(PageProgramTime);
	else if (waitForTask == busyWait)
		while (IS_Busy(SPIx))
			;

	return SUCCESS;
}

ErrorStatus w25qxx_Read(SPI_TypeDef *SPIx, uint8_t *buf, uint16_t bufSize, uint32_t address)
{
	if ((bufSize == 0) || (bufSize > PageSize))
		return ERROR; // 1-256 bytes data can be read
	if ((address % PageSize) != 0)
		return ERROR; // only first byte of the page can be pointed as the start byte
	if (address > ((PageSize * numberOfPages) - 1))
		return ERROR;

	if (w25qxx_WaitWithTimeout(SPIx, 100) != SUCCESS)
		return ERROR;

	/* Page read */
	CS0_Low;
	SPI_Transmit(SPIx, &ReadData, 1);
	/* A23-A0 - Start address of the desired page */
	MODIFY_REG(addressBytes, AddressBytesMask, (address & AddressBytesMask));
	threeAddrBytes[0] = (uint8_t)(addressBytes >> 16);
	threeAddrBytes[1] = (uint8_t)(addressBytes >> 8);
	threeAddrBytes[2] = (uint8_t)(addressBytes);
	SPI_Transmit(SPIx, threeAddrBytes, sizeof(threeAddrBytes));
	/* Data read */
	SPI_Receive(SPIx, buf, bufSize);
	CS0_High;
	_delay_us(3);

	return SUCCESS;
}

ErrorStatus w25qxx_ReadID(SPI_TypeDef *SPIx, uint8_t *ID_Buf)
{
	if (w25qxx_WaitWithTimeout(SPIx, 100) != SUCCESS)
		return ERROR;

	CS0_Low;
	SPI_Transmit(SPIx, &ReadManufacturerDeviceID, 1);
	SPI_Transmit(SPIx, &DummyByte, 1);
	SPI_Transmit(SPIx, &DummyByte, 1);
	SPI_Transmit(SPIx, &DummyByte, 1);
	SPI_Receive(SPIx, ID_Buf, 2);
	CS0_High;
	_delay_us(3);

	return SUCCESS;
}

ErrorStatus w25qxx_WriteStatus(SPI_TypeDef *SPIx, uint8_t statusRegisterx, const uint8_t *status)
{
	if (w25qxx_WaitWithTimeout(SPIx, 100) != SUCCESS)
		return ERROR;

	switch (statusRegisterx)
	{
	case 1:
		CS0_Low;
		SPI_Transmit(SPIx, &VolatileSR_WriteEnable, 1);
		CS0_High;
		_delay_us(3);
		/* Write status */
		CS0_Low;
		SPI_Transmit(SPIx, &WriteStatusRegister1, 1);
		SPI_Transmit(SPIx, status, 1);
		CS0_High;
		_delay_us(3);
		break;

	case 2:
		CS0_Low;
		SPI_Transmit(SPIx, &VolatileSR_WriteEnable, 1);
		CS0_High;
		_delay_us(3);
		/* Write status */
		CS0_Low;
		SPI_Transmit(SPIx, &WriteStatusRegister2, 1);
		SPI_Transmit(SPIx, status, 1);
		CS0_High;
		_delay_us(3);
		break;

	case 3:
		CS0_Low;
		SPI_Transmit(SPIx, &VolatileSR_WriteEnable, 1);
		CS0_High;
		_delay_us(3);
		/* Write status */
		CS0_Low;
		SPI_Transmit(SPIx, &WriteStatusRegister3, 1);
		SPI_Transmit(SPIx, status, 1);
		CS0_High;
		_delay_us(3);
		break;
	}

	return SUCCESS;
}

void w25qxx_ReadStatus(SPI_TypeDef *SPIx, uint8_t statusRegisterx, uint8_t *status)
{
	switch (statusRegisterx)
	{
	case 1:
		CS0_Low;
		SPI_Transmit(SPIx, &ReadStatusRegister1, 1);
		SPI_Receive(SPIx, status, 1);
		CS0_High;
		_delay_us(3);
		break;

	case 2:
		CS0_Low;
		SPI_Transmit(SPIx, &ReadStatusRegister2, 1);
		SPI_Receive(SPIx, status, 1);
		CS0_High;
		_delay_us(3);
		break;

	case 3:
		CS0_Low;
		SPI_Transmit(SPIx, &ReadStatusRegister3, 1);
		SPI_Receive(SPIx, status, 1);
		CS0_High;
		_delay_us(3);
		break;
	}
}

bool IS_Busy(SPI_TypeDef *SPIx)
{
	uint8_t status;
	w25qxx_ReadStatus(SPIx, 1, &status);

	return READ_BIT(status, 1 << 0);
}

ErrorStatus w25qxx_WaitWithTimeout(SPI_TypeDef *SPIx, uint32_t timeout)
{
	while (1)
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
			_delay_us(3);
			return ERROR;
		}
	}
}

void w25qxx_WriteEnable(SPI_TypeDef *SPIx)
{
	CS0_Low;
	SPI_Transmit(SPIx, &WriteEnable, 1);
	CS0_High;
	_delay_us(3);
}

void w25qxx_WriteDisable(SPI_TypeDef *SPIx)
{
	CS0_Low;
	SPI_Transmit(SPIx, &WriteDisable, 1);
	CS0_High;
	_delay_us(3);
}

ErrorStatus w25qxx_Erase(SPI_TypeDef *SPIx, eraseInstruction_Def eraseInstruction, uint32_t address, uint8_t waitForTask)
{
	if (w25qxx_WaitWithTimeout(SPIx, 100) != SUCCESS)
		return ERROR;

	switch (eraseInstruction)
	{
	case sectorErase_4KB:
		if ((address % KBtoByte(4)) != 0)
			return ERROR; // incorrect start address for sector
		if (address > ((PageSize * numberOfPages) - KBtoByte(4)))
			return ERROR; // the boundaries of erase operation beyond memory
		w25qxx_WriteEnable(SPIx);
		/* Sector Erase */
		CS0_Low;
		SPI_Transmit(SPIx, &SectorErase_4KB, 1);
		/* A23-A0 - Start address of the desired sector */
		MODIFY_REG(addressBytes, AddressBytesMask, (address & AddressBytesMask));
		threeAddrBytes[0] = (uint8_t)(addressBytes >> 16);
		threeAddrBytes[1] = (uint8_t)(addressBytes >> 8);
		threeAddrBytes[2] = (uint8_t)(addressBytes);
		SPI_Transmit(SPIx, threeAddrBytes, sizeof(threeAddrBytes));
		CS0_High;
		_delay_us(3);
		if (waitForTask == delayWait)
			_delay_ms(SectorEraseTime_4KB);
		else if (waitForTask == busyWait)
			while (IS_Busy(SPIx))
				;
		break;

	case blockErase_32KB:
		if ((address % KBtoByte(32)) != 0)
			return ERROR; // incorrect start address for block
		if (address > ((PageSize * numberOfPages) - KBtoByte(32)))
			return ERROR; // the boundaries of erase operation beyond memory
		w25qxx_WriteEnable(SPIx);
		/* Block Erase */
		CS0_Low;
		SPI_Transmit(SPIx, &BlockErase_32KB, 1);
		/* A23-A0 - Start address of the desired block */
		MODIFY_REG(addressBytes, AddressBytesMask, (address & AddressBytesMask));
		threeAddrBytes[0] = (uint8_t)(addressBytes >> 16);
		threeAddrBytes[1] = (uint8_t)(addressBytes >> 8);
		threeAddrBytes[2] = (uint8_t)(addressBytes);
		SPI_Transmit(SPIx, threeAddrBytes, sizeof(threeAddrBytes));
		CS0_High;
		_delay_us(3);
		if (waitForTask == delayWait)
			_delay_ms(BlockEraseTime_32KB);
		else if (waitForTask == busyWait)
			while (IS_Busy(SPIx))
				;
		break;

	case blockErase_64KB:
		if ((address % KBtoByte(64)) != 0)
			return ERROR; // incorrect start address for block
		if (address > ((PageSize * numberOfPages) - KBtoByte(64)))
			return ERROR; // the boundaries of erase operation beyond memory
		w25qxx_WriteEnable(SPIx);
		/* Block Erase */
		CS0_Low;
		SPI_Transmit(SPIx, &BlockErase_64KB, 1);
		/* A23-A0 - Start address of the desired block */
		MODIFY_REG(addressBytes, AddressBytesMask, (address & AddressBytesMask));
		threeAddrBytes[0] = (uint8_t)(addressBytes >> 16);
		threeAddrBytes[1] = (uint8_t)(addressBytes >> 8);
		threeAddrBytes[2] = (uint8_t)(addressBytes);
		SPI_Transmit(SPIx, threeAddrBytes, sizeof(threeAddrBytes));
		CS0_High;
		_delay_us(3);
		if (waitForTask == delayWait)
			_delay_ms(BlockEraseTime_64KB);
		else if (waitForTask == busyWait)
			while (IS_Busy(SPIx))
				;
		break;

	case chipErase:
		w25qxx_WriteEnable(SPIx);
		/* Chip Erase */
		CS0_Low;
		SPI_Transmit(SPIx, &ChipErase, 1);
		CS0_High;
		_delay_us(3);
		if (waitForTask == delayWait)
			_delay_ms(ChipEraseTime);
		else if (waitForTask == busyWait)
			while (IS_Busy(SPIx))
				;
		break;

	default:
		return ERROR;
		break;
	}

	return SUCCESS;
}