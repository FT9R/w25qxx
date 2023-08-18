#include "w25qxx.h"

/* Public variables */
w25qxx_HandleTypeDef w25qxx_Handle;

/* Private function prototypes */
ErrorStatus w25qxx_ReadID(w25qxx_HandleTypeDef *w25qxx_Handle);
ErrorStatus w25qxx_WaitWithTimeout(w25qxx_HandleTypeDef *w25qxx_Handle, uint32_t timeout);
void w25qxx_WriteEnable(w25qxx_HandleTypeDef *w25qxx_Handle);
void w25qxx_WriteDisable(w25qxx_HandleTypeDef *w25qxx_Handle);

ErrorStatus w25qxx_Init(w25qxx_HandleTypeDef *w25qxx_Handle, SPI_TypeDef *SPIx, GPIO_TypeDef *CS_Port, uint16_t CS_Pin)
{
    w25qxx_Delay(100);

    /* SPI device specific info retrieve */
    w25qxx_Handle->SPIx = SPIx;
    w25qxx_Handle->CS_Port = CS_Port;
    w25qxx_Handle->CS_Pin = CS_Pin;
    w25qxx_Handle->status = ERROR;

    /* Check for SPI1-3 match */
    if ((w25qxx_Handle->SPIx != SPI1) && (w25qxx_Handle->SPIx != SPI2) && (w25qxx_Handle->SPIx != SPI3))
        return w25qxx_Handle->status;

    /* Release from power-down */
    w25qxx_Handle->cmd = W25QXX_CMD_RELEASE_POWER_DOWN;
    CS_LOW(w25qxx_Handle);
    w25qxx_SPI_Transmit(w25qxx_Handle->SPIx, &w25qxx_Handle->cmd, 1);
    CS_HIGH(w25qxx_Handle);
    w25qxx_Delay(1);

    /* Reset device*/
    w25qxx_Handle->cmd = W25QXX_CMD_ENABLE_RESET;
    CS_LOW(w25qxx_Handle);
    w25qxx_SPI_Transmit(w25qxx_Handle->SPIx, &w25qxx_Handle->cmd, 1);
    CS_HIGH(w25qxx_Handle);
    w25qxx_Handle->cmd = W25QXX_CMD_RESET_DEVICE;
    CS_LOW(w25qxx_Handle);
    w25qxx_SPI_Transmit(w25qxx_Handle->SPIx, &w25qxx_Handle->cmd, 1);
    CS_HIGH(w25qxx_Handle);
    w25qxx_Delay(1);

    /* Get the ManufacturerID and DeviceID */
    w25qxx_ReadID(w25qxx_Handle);
    if (w25qxx_Handle->ID[0] != W25QXX_MANUFACTURER_ID)
        return w25qxx_Handle->status;

    /* Determine number of pages */
    switch (w25qxx_Handle->ID[1])
    {
    case w25q80:
        w25qxx_Handle->numberOfPages = 8 * (KB_TO_BYTE(1) * KB_TO_BYTE(1) / W25QXX_PAGE_SIZE / 8);
        break;

    case w25q16:
        w25qxx_Handle->numberOfPages = 16 * (KB_TO_BYTE(1) * KB_TO_BYTE(1) / W25QXX_PAGE_SIZE / 8);
        break;

    case w25q32:
        w25qxx_Handle->numberOfPages = 32 * (KB_TO_BYTE(1) * KB_TO_BYTE(1) / W25QXX_PAGE_SIZE / 8);
        break;

    case w25q64:
        w25qxx_Handle->numberOfPages = 64 * (KB_TO_BYTE(1) * KB_TO_BYTE(1) / W25QXX_PAGE_SIZE / 8);
        break;

    case w25q128:
        w25qxx_Handle->numberOfPages = 128 * (KB_TO_BYTE(1) * KB_TO_BYTE(1) / W25QXX_PAGE_SIZE / 8);
        break;

    default:
        return w25qxx_Handle->status; // Unsupported device
    }

    return w25qxx_Handle->status = SUCCESS;
}

ErrorStatus w25qxx_Write(w25qxx_HandleTypeDef *w25qxx_Handle, const uint8_t *buf, uint16_t bufSize, uint32_t address,
                         waitForTask_t waitForTask)
{
    if ((bufSize == 0) || (bufSize > W25QXX_PAGE_SIZE))
        return ERROR; // 1-256 bytes data can be written
    if ((address % W25QXX_PAGE_SIZE) != 0)
        return ERROR; // Only first byte of the page can be pointed as the start byte
    if (address > ((W25QXX_PAGE_SIZE * w25qxx_Handle->numberOfPages) - bufSize))
        return ERROR;

    if (w25qxx_WaitWithTimeout(w25qxx_Handle, 100) != SUCCESS)
        return ERROR;

    /* Page program */
    w25qxx_WriteEnable(w25qxx_Handle);
    w25qxx_Handle->cmd = W25QXX_CMD_PAGE_PROGRAM;
    CS_LOW(w25qxx_Handle);
    w25qxx_SPI_Transmit(w25qxx_Handle->SPIx, &w25qxx_Handle->cmd, 1);

    /* A23-A0 - Start address of the desired page */
    w25qxx_Handle->addressBytes[0] = (uint8_t) (address >> 16);
    w25qxx_Handle->addressBytes[1] = (uint8_t) (address >> 8);
    w25qxx_Handle->addressBytes[2] = (uint8_t) (address);
    w25qxx_SPI_Transmit(w25qxx_Handle->SPIx, w25qxx_Handle->addressBytes, sizeof(w25qxx_Handle->addressBytes));

    /* Data write */
    w25qxx_SPI_Transmit(w25qxx_Handle->SPIx, buf, bufSize);
    CS_HIGH(w25qxx_Handle);

    if (waitForTask == WAIT_DELAY)
        w25qxx_Delay(W25QXX_PAGE_PROGRAM_TIME);
    else if (waitForTask == WAIT_BUSY)
        while (w25qxx_Busy(w25qxx_Handle)) {}

    return SUCCESS;
}

ErrorStatus w25qxx_Read(w25qxx_HandleTypeDef *w25qxx_Handle, uint8_t *buf, uint16_t bufSize, uint32_t address)
{
    if ((bufSize == 0) || (bufSize > W25QXX_PAGE_SIZE))
        return ERROR; // 1-256 bytes data can be read
    if ((address % W25QXX_PAGE_SIZE) != 0)
        return ERROR; // Only first byte of the page can be pointed as the start byte
    if (address > ((W25QXX_PAGE_SIZE * w25qxx_Handle->numberOfPages) - bufSize))
        return ERROR;

    if (w25qxx_WaitWithTimeout(w25qxx_Handle, 100) != SUCCESS)
        return ERROR;

    /* Page read */
    w25qxx_Handle->cmd = W25QXX_CMD_READ_DATA;
    CS_LOW(w25qxx_Handle);
    w25qxx_SPI_Transmit(w25qxx_Handle->SPIx, &w25qxx_Handle->cmd, 1);

    /* A23-A0 - Start address of the desired page */
    w25qxx_Handle->addressBytes[0] = (uint8_t) (address >> 16);
    w25qxx_Handle->addressBytes[1] = (uint8_t) (address >> 8);
    w25qxx_Handle->addressBytes[2] = (uint8_t) (address);
    w25qxx_SPI_Transmit(w25qxx_Handle->SPIx, w25qxx_Handle->addressBytes, sizeof(w25qxx_Handle->addressBytes));

    /* Data read */
    w25qxx_SPI_Receive(w25qxx_Handle->SPIx, buf, bufSize);
    CS_HIGH(w25qxx_Handle);

    return SUCCESS;
}

ErrorStatus w25qxx_WriteStatus(w25qxx_HandleTypeDef *w25qxx_Handle, uint8_t statusRegisterx, const uint8_t *status)
{
    if (w25qxx_WaitWithTimeout(w25qxx_Handle, 100) != SUCCESS)
        return ERROR;

    w25qxx_Handle->cmd = W25QXX_CMD_VOLATILE_SR_WRITE_ENABLE;
    CS_LOW(w25qxx_Handle);
    w25qxx_SPI_Transmit(w25qxx_Handle->SPIx, &w25qxx_Handle->cmd, 1);
    CS_HIGH(w25qxx_Handle);

    switch (statusRegisterx)
    {
    case 1:
        w25qxx_Handle->cmd = W25QXX_CMD_WRITE_STATUS_REGISTER1;
        CS_LOW(w25qxx_Handle);
        w25qxx_SPI_Transmit(w25qxx_Handle->SPIx, &w25qxx_Handle->cmd, 1);
        w25qxx_SPI_Transmit(w25qxx_Handle->SPIx, status, 1);
        CS_HIGH(w25qxx_Handle);
        break;

    case 2:
        w25qxx_Handle->cmd = W25QXX_CMD_WRITE_STATUS_REGISTER2;
        CS_LOW(w25qxx_Handle);
        w25qxx_SPI_Transmit(w25qxx_Handle->SPIx, &w25qxx_Handle->cmd, 1);
        w25qxx_SPI_Transmit(w25qxx_Handle->SPIx, status, 1);
        CS_HIGH(w25qxx_Handle);
        break;

    case 3:
        w25qxx_Handle->cmd = W25QXX_CMD_WRITE_STATUS_REGISTER3;
        CS_LOW(w25qxx_Handle);
        w25qxx_SPI_Transmit(w25qxx_Handle->SPIx, &w25qxx_Handle->cmd, 1);
        w25qxx_SPI_Transmit(w25qxx_Handle->SPIx, status, 1);
        CS_HIGH(w25qxx_Handle);
        break;
    }

    return SUCCESS;
}

void w25qxx_ReadStatus(w25qxx_HandleTypeDef *w25qxx_Handle, uint8_t statusRegisterx, uint8_t *status)
{
    switch (statusRegisterx)
    {
    case 1:
        w25qxx_Handle->cmd = W25QXX_CMD_READ_STATUS_REGISTER1;
        CS_LOW(w25qxx_Handle);
        w25qxx_SPI_Transmit(w25qxx_Handle->SPIx, &w25qxx_Handle->cmd, 1);
        w25qxx_SPI_Receive(w25qxx_Handle->SPIx, status, 1);
        CS_HIGH(w25qxx_Handle);
        break;

    case 2:
        w25qxx_Handle->cmd = W25QXX_CMD_READ_STATUS_REGISTER2;
        CS_LOW(w25qxx_Handle);
        w25qxx_SPI_Transmit(w25qxx_Handle->SPIx, &w25qxx_Handle->cmd, 1);
        w25qxx_SPI_Receive(w25qxx_Handle->SPIx, status, 1);
        CS_HIGH(w25qxx_Handle);
        break;

    case 3:
        w25qxx_Handle->cmd = W25QXX_CMD_READ_STATUS_REGISTER2;
        CS_LOW(w25qxx_Handle);
        w25qxx_SPI_Transmit(w25qxx_Handle->SPIx, &w25qxx_Handle->cmd, 1);
        w25qxx_SPI_Receive(w25qxx_Handle->SPIx, status, 1);
        CS_HIGH(w25qxx_Handle);
        break;
    }
}

ErrorStatus w25qxx_Erase(w25qxx_HandleTypeDef *w25qxx_Handle, eraseInstruction_t eraseInstruction, uint32_t address,
                         waitForTask_t waitForTask)
{
    if (w25qxx_WaitWithTimeout(w25qxx_Handle, 100) != SUCCESS)
        return ERROR;

    switch (eraseInstruction)
    {
    case SECTOR_ERASE_4KB:
        if ((address % KB_TO_BYTE(4)) != 0)
            return ERROR; // Incorrect start address for sector
        if (address > ((W25QXX_PAGE_SIZE * w25qxx_Handle->numberOfPages) - KB_TO_BYTE(4)))
            return ERROR; // The boundaries of erase operation beyond memory

        /* Sector Erase */
        w25qxx_WriteEnable(w25qxx_Handle);
        w25qxx_Handle->cmd = W25QXX_CMD_SECTOR_ERASE_4KB;
        CS_LOW(w25qxx_Handle);
        w25qxx_SPI_Transmit(w25qxx_Handle->SPIx, &w25qxx_Handle->cmd, 1);

        /* A23-A0 - Start address of the desired sector */
        w25qxx_Handle->addressBytes[0] = (uint8_t) (address >> 16);
        w25qxx_Handle->addressBytes[1] = (uint8_t) (address >> 8);
        w25qxx_Handle->addressBytes[2] = (uint8_t) (address);
        w25qxx_SPI_Transmit(w25qxx_Handle->SPIx, w25qxx_Handle->addressBytes, sizeof(w25qxx_Handle->addressBytes));
        CS_HIGH(w25qxx_Handle);

        if (waitForTask == WAIT_DELAY)
        {
            for (uint32_t i = 0; i < (W25QXX_SECTOR_ERASE_TIME_4KB / 100); i++)
            {
                w25qxx_Delay(100);
            }
        }
        else if (waitForTask == WAIT_BUSY)
        {
            while (w25qxx_Busy(w25qxx_Handle)) {}
        }
        break;

    case BLOCK_ERASE_32KB:
        if ((address % KB_TO_BYTE(32)) != 0)
            return ERROR; // Incorrect start address for block
        if (address > ((W25QXX_PAGE_SIZE * w25qxx_Handle->numberOfPages) - KB_TO_BYTE(32)))
            return ERROR; // The boundaries of erase operation beyond memory

        /* Block Erase */
        w25qxx_WriteEnable(w25qxx_Handle);
        w25qxx_Handle->cmd = W25QXX_CMD_BLOCK_ERASE_32KB;
        CS_LOW(w25qxx_Handle);
        w25qxx_SPI_Transmit(w25qxx_Handle->SPIx, &w25qxx_Handle->cmd, 1);

        /* A23-A0 - Start address of the desired block */
        w25qxx_Handle->addressBytes[0] = (uint8_t) (address >> 16);
        w25qxx_Handle->addressBytes[1] = (uint8_t) (address >> 8);
        w25qxx_Handle->addressBytes[2] = (uint8_t) (address);
        w25qxx_SPI_Transmit(w25qxx_Handle->SPIx, w25qxx_Handle->addressBytes, sizeof(w25qxx_Handle->addressBytes));
        CS_HIGH(w25qxx_Handle);

        if (waitForTask == WAIT_DELAY)
        {
            for (uint32_t i = 0; i < (W25QXX_BLOCK_ERASE_TIME_32KB / 100); i++)
            {
                w25qxx_Delay(100);
            }
        }
        else if (waitForTask == WAIT_BUSY)
        {
            while (w25qxx_Busy(w25qxx_Handle)) {}
        }
        break;

    case BLOCK_ERASE_64KB:
        if ((address % KB_TO_BYTE(64)) != 0)
            return ERROR; // Incorrect start address for block
        if (address > ((W25QXX_PAGE_SIZE * w25qxx_Handle->numberOfPages) - KB_TO_BYTE(64)))
            return ERROR; // The boundaries of erase operation beyond memory

        /* Block Erase */
        w25qxx_WriteEnable(w25qxx_Handle);
        w25qxx_Handle->cmd = W25QXX_CMD_BLOCK_ERASE_64KB;
        CS_LOW(w25qxx_Handle);
        w25qxx_SPI_Transmit(w25qxx_Handle->SPIx, &w25qxx_Handle->cmd, 1);

        /* A23-A0 - Start address of the desired block */
        w25qxx_Handle->addressBytes[0] = (uint8_t) (address >> 16);
        w25qxx_Handle->addressBytes[1] = (uint8_t) (address >> 8);
        w25qxx_Handle->addressBytes[2] = (uint8_t) (address);
        w25qxx_SPI_Transmit(w25qxx_Handle->SPIx, w25qxx_Handle->addressBytes, sizeof(w25qxx_Handle->addressBytes));
        CS_HIGH(w25qxx_Handle);

        if (waitForTask == WAIT_DELAY)
        {
            for (uint32_t i = 0; i < (W25QXX_BLOCK_ERASE_TIME_64KB / 100); i++)
            {
                w25qxx_Delay(100);
            }
        }
        else if (waitForTask == WAIT_BUSY)
        {
            while (w25qxx_Busy(w25qxx_Handle)) {}
        }
        break;

    case CHIP_ERASE:
        if (address != NULL)
            return ERROR;

        /* Chip Erase */
        w25qxx_WriteEnable(w25qxx_Handle);
        w25qxx_Handle->cmd = W25QXX_CMD_CHIP_ERASE;
        CS_LOW(w25qxx_Handle);
        w25qxx_SPI_Transmit(w25qxx_Handle->SPIx, &w25qxx_Handle->cmd, 1);
        CS_HIGH(w25qxx_Handle);

        if (waitForTask == WAIT_DELAY)
        {
            for (uint32_t i = 0; i < (W25QXX_CHIP_ERASE_TIME / 100); i++)
            {
                w25qxx_Delay(100);
            }
        }
        else if (waitForTask == WAIT_BUSY)
        {
            while (w25qxx_Busy(w25qxx_Handle)) {}
        }
        break;

    default:
        return ERROR; //	Invalid erase instruction
    }

    return SUCCESS;
}

bool w25qxx_Busy(w25qxx_HandleTypeDef *w25qxx_Handle)
{
    uint8_t status;
    w25qxx_ReadStatus(w25qxx_Handle, 1, &status);

    return READ_BIT(status, 0x01);
}

/**
 * @section Private functions
 */
ErrorStatus w25qxx_ReadID(w25qxx_HandleTypeDef *w25qxx_Handle)
{
    if (w25qxx_WaitWithTimeout(w25qxx_Handle, 100) != SUCCESS)
        return ERROR;

    uint8_t dummyByte = NULL;
    w25qxx_Handle->cmd = W25QXX_CMD_MANUFACTURER_DEVICE_ID;
    CS_LOW(w25qxx_Handle);
    w25qxx_SPI_Transmit(w25qxx_Handle->SPIx, &w25qxx_Handle->cmd, 1);
    w25qxx_SPI_Transmit(w25qxx_Handle->SPIx, &dummyByte, 1);
    w25qxx_SPI_Transmit(w25qxx_Handle->SPIx, &dummyByte, 1);
    w25qxx_SPI_Transmit(w25qxx_Handle->SPIx, &dummyByte, 1);
    w25qxx_SPI_Receive(w25qxx_Handle->SPIx, w25qxx_Handle->ID, 2);
    CS_HIGH(w25qxx_Handle);

    return SUCCESS;
}

ErrorStatus w25qxx_WaitWithTimeout(w25qxx_HandleTypeDef *w25qxx_Handle, uint32_t timeout)
{
    while (true)
    {
        if (w25qxx_Busy(w25qxx_Handle))
        {
            timeout--;
            w25qxx_Delay(1);
        }
        else
            return SUCCESS;

        if (timeout <= 0)
            return ERROR;
    }
}

void w25qxx_WriteEnable(w25qxx_HandleTypeDef *w25qxx_Handle)
{
    w25qxx_Handle->cmd = W25QXX_CMD_WRITE_ENABLE;
    CS_LOW(w25qxx_Handle);
    w25qxx_SPI_Transmit(w25qxx_Handle->SPIx, &w25qxx_Handle->cmd, 1);
    CS_HIGH(w25qxx_Handle);
}

void w25qxx_WriteDisable(w25qxx_HandleTypeDef *w25qxx_Handle)
{
    w25qxx_Handle->cmd = W25QXX_CMD_WRITE_DISABLE;
    CS_LOW(w25qxx_Handle);
    w25qxx_SPI_Transmit(w25qxx_Handle->SPIx, &w25qxx_Handle->cmd, 1);
    CS_HIGH(w25qxx_Handle);
}
