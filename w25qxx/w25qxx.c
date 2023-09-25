#include "w25qxx.h"

/* Private function prototypes */
// static ErrorStatus w25qxx_WriteStatus(w25qxx_HandleTypeDef *w25qxx_Handle, uint8_t statusRegisterx);
static void w25qxx_ReadStatus(w25qxx_HandleTypeDef *w25qxx_Handle, uint8_t statusRegisterx);
static ErrorStatus w25qxx_ReadID(w25qxx_HandleTypeDef *w25qxx_Handle);
static ErrorStatus w25qxx_WaitWithTimeout(w25qxx_HandleTypeDef *w25qxx_Handle, uint32_t timeout);
static void w25qxx_WriteEnable(w25qxx_HandleTypeDef *w25qxx_Handle);
// static void w25qxx_WriteDisable(w25qxx_HandleTypeDef *w25qxx_Handle);
static uint16_t ModBus_CRC(const uint8_t *pBuffer, uint16_t bufSize);

ErrorStatus w25qxx_Init(w25qxx_HandleTypeDef *w25qxx_Handle, SPI_HandleTypeDef *hspix, GPIO_TypeDef *CS_Port,
                        uint16_t CS_Pin)
{
    w25qxx_Delay(100);

    /* SPI device specific info retrieve */
    w25qxx_Handle->hspix = hspix;
    w25qxx_Handle->CS_Port = CS_Port;
    w25qxx_Handle->CS_Pin = CS_Pin;
    w25qxx_Handle->status = ERROR;

    /* Check for SPI1-3 match */
    if ((w25qxx_Handle->hspix->Instance != SPI1) && (w25qxx_Handle->hspix->Instance != SPI2)
        && (w25qxx_Handle->hspix->Instance != SPI3))
        return w25qxx_Handle->status;

    /* Release from power-down */
    CS_LOW(w25qxx_Handle);
    w25qxx_Handle->CMD = W25QXX_CMD_RELEASE_POWER_DOWN;
    w25qxx_SPI_Transmit(w25qxx_Handle->hspix, &w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);
    CS_HIGH(w25qxx_Handle);
    w25qxx_Delay(1);

    /* Reset device*/
    CS_LOW(w25qxx_Handle);
    w25qxx_Handle->CMD = W25QXX_CMD_ENABLE_RESET;
    w25qxx_SPI_Transmit(w25qxx_Handle->hspix, &w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);
    CS_HIGH(w25qxx_Handle);
    w25qxx_Delay(1);
    CS_LOW(w25qxx_Handle);
    w25qxx_Handle->CMD = W25QXX_CMD_RESET_DEVICE;
    w25qxx_SPI_Transmit(w25qxx_Handle->hspix, &w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);
    CS_HIGH(w25qxx_Handle);
    w25qxx_Delay(1);

    /* Get the Manufacturer ID and Device ID */
    w25qxx_ReadID(w25qxx_Handle);
    if (w25qxx_Handle->ID[0] != W25QXX_MANUFACTURER_ID)
        return w25qxx_Handle->status;

    /* Determine number of pages */
    switch (w25qxx_Handle->ID[1])
    {
    case W25Q80:
        w25qxx_Handle->numberOfPages = 8 * (KB_TO_BYTE(1) * KB_TO_BYTE(1) / W25QXX_PAGE_SIZE / 8);
        break;

    case W25Q16:
        w25qxx_Handle->numberOfPages = 16 * (KB_TO_BYTE(1) * KB_TO_BYTE(1) / W25QXX_PAGE_SIZE / 8);
        break;

    case W25Q32:
        w25qxx_Handle->numberOfPages = 32 * (KB_TO_BYTE(1) * KB_TO_BYTE(1) / W25QXX_PAGE_SIZE / 8);
        break;

    case W25Q64:
        w25qxx_Handle->numberOfPages = 64 * (KB_TO_BYTE(1) * KB_TO_BYTE(1) / W25QXX_PAGE_SIZE / 8);
        break;

    case W25Q128:
        w25qxx_Handle->numberOfPages = 128 * (KB_TO_BYTE(1) * KB_TO_BYTE(1) / W25QXX_PAGE_SIZE / 8);
        break;

    default:
        return w25qxx_Handle->status; // Unsupported device
    }

    return w25qxx_Handle->status = SUCCESS;
}

ErrorStatus w25qxx_Write(w25qxx_HandleTypeDef *w25qxx_Handle, const uint8_t *buf, uint16_t dataLength, uint32_t address,
                         bool trailingCRC, waitForTask_t waitForTask)
{
    w25qxx_Handle->status = ERROR;
    uint16_t CRC16 = 0x0000;

    /* Argument guards */
    if ((dataLength == 0) || (buf == NULL))
        return w25qxx_Handle->status;
    if (trailingCRC)
        dataLength += sizeof(uint16_t);
    if (dataLength > W25QXX_PAGE_SIZE)
        return w25qxx_Handle->status;
    if ((address % W25QXX_PAGE_SIZE) != 0)
        return w25qxx_Handle->status; // Only first byte of the page can be pointed as the start byte
    if (address > (W25QXX_PAGE_SIZE * (w25qxx_Handle->numberOfPages - 1)))
        return w25qxx_Handle->status; // The boundaries of write operation beyond memory

    if (w25qxx_WaitWithTimeout(w25qxx_Handle, W25QXX_RESPONSE_TIMEOUT) != SUCCESS)
        return w25qxx_Handle->status;

    /* Middle buffer operations */
    uint8_t *midBuf = malloc(sizeof(*midBuf) * dataLength);
    if (midBuf == NULL)
        return w25qxx_Handle->status; // Insufficient heap memory available
    if (trailingCRC)
    {
        /* Data + checksum */
        memcpy(midBuf, buf, dataLength - sizeof(uint16_t));
        CRC16 = ModBus_CRC(buf, dataLength - sizeof(uint16_t));
        midBuf[dataLength - 2] = ((CRC16 >> 8) & 0xFF);
        midBuf[dataLength - 1] = (CRC16 & 0xFF);
    }
    else
        /* Pure data */
        memcpy(midBuf, buf, dataLength);

    /* Command */
    w25qxx_WriteEnable(w25qxx_Handle);
    CS_LOW(w25qxx_Handle);
    w25qxx_Handle->CMD = W25QXX_CMD_PAGE_PROGRAM;
    w25qxx_SPI_Transmit(w25qxx_Handle->hspix, &w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);

    /* A23-A0 - Start address of the desired page */
    ADDRESS_BYTES_SWAP(w25qxx_Handle, address);
    w25qxx_SPI_Transmit(w25qxx_Handle->hspix, w25qxx_Handle->addressBytes, sizeof(w25qxx_Handle->addressBytes),
                        W25QXX_TX_TIMEOUT);

    /* Data write */
    w25qxx_SPI_Transmit(w25qxx_Handle->hspix, midBuf, dataLength, W25QXX_TX_TIMEOUT);
    free(midBuf);
    CS_HIGH(w25qxx_Handle);

    /* Wait options */
    if (waitForTask == W25QXX_WAIT_DELAY)
        w25qxx_Delay(W25QXX_PAGE_PROGRAM_TIME);
    else if (waitForTask == W25QXX_WAIT_BUSY)
    {
        if (w25qxx_WaitWithTimeout(w25qxx_Handle, W25QXX_PAGE_PROGRAM_TIME) != SUCCESS)
            return w25qxx_Handle->status;
    }

    return w25qxx_Handle->status = SUCCESS;
}

ErrorStatus w25qxx_Read(w25qxx_HandleTypeDef *w25qxx_Handle, uint8_t *buf, uint16_t dataLength, uint32_t address,
                        bool trailingCRC, bool fastRead)
{
    w25qxx_Handle->status = ERROR;
    uint16_t CRC16 = 0x0000;

    /* Argument guards */
    if ((dataLength == 0) || (buf == NULL))
        return w25qxx_Handle->status;
    if (trailingCRC)
        dataLength += sizeof(uint16_t);
    if (dataLength > W25QXX_PAGE_SIZE)
        return w25qxx_Handle->status;
    if ((address % W25QXX_PAGE_SIZE) != 0)
        return w25qxx_Handle->status; // Only first byte of the page can be pointed as the start byte
    if (address > (W25QXX_PAGE_SIZE * (w25qxx_Handle->numberOfPages - 1)))
        return w25qxx_Handle->status; // The boundaries of read operation beyond memory

    if (w25qxx_WaitWithTimeout(w25qxx_Handle, W25QXX_RESPONSE_TIMEOUT) != SUCCESS)
        return w25qxx_Handle->status;

    /* Middle buffer operations */
    uint8_t *midBuf = malloc(sizeof(*midBuf) * dataLength);
    if (midBuf == NULL)
        return w25qxx_Handle->status; // Insufficient heap memory available

    /* Command */
    CS_LOW(w25qxx_Handle);
    if (fastRead)
        w25qxx_Handle->CMD = W25QXX_CMD_FAST_READ;
    else
        w25qxx_Handle->CMD = W25QXX_CMD_READ_DATA;
    w25qxx_SPI_Transmit(w25qxx_Handle->hspix, &w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);

    /* A23-A0 - Start address of the desired page */
    ADDRESS_BYTES_SWAP(w25qxx_Handle, address);
    w25qxx_SPI_Transmit(w25qxx_Handle->hspix, w25qxx_Handle->addressBytes, sizeof(w25qxx_Handle->addressBytes),
                        W25QXX_TX_TIMEOUT);

    /* Data read */
    if (fastRead)
        w25qxx_SPI_Transmit(w25qxx_Handle->hspix, &w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);
    w25qxx_SPI_Receive(w25qxx_Handle->hspix, midBuf, dataLength, W25QXX_RX_TIMEOUT);
    CS_HIGH(w25qxx_Handle);

    /* Checksum compare */
    if (trailingCRC)
    {
        CRC16 = ModBus_CRC(midBuf, dataLength - sizeof(uint16_t));
        if ((midBuf[dataLength - 2] != ((CRC16 >> 8) & 0xFF)) || (midBuf[dataLength - 1] != (CRC16 & 0xFF)))
        {
            free(midBuf);
            return w25qxx_Handle->status; // CRC error
        }
    }

    /* Copy middle buffer content to the destination buffer */
    if (trailingCRC)
        memcpy(buf, midBuf, dataLength - sizeof(uint16_t));
    else
        memcpy(buf, midBuf, dataLength);
    free(midBuf);

    return w25qxx_Handle->status = SUCCESS;
}

ErrorStatus w25qxx_Erase(w25qxx_HandleTypeDef *w25qxx_Handle, eraseInstruction_t eraseInstruction, uint32_t address,
                         waitForTask_t waitForTask)
{
    w25qxx_Handle->status = ERROR;

    if (w25qxx_WaitWithTimeout(w25qxx_Handle, W25QXX_RESPONSE_TIMEOUT) != SUCCESS)
        return w25qxx_Handle->status;

    switch (eraseInstruction)
    {
    case W25QXX_SECTOR_ERASE_4KB:
        if ((address % KB_TO_BYTE(4)) != 0)
            return w25qxx_Handle->status; // Incorrect start address for sector
        if (address > ((W25QXX_PAGE_SIZE * w25qxx_Handle->numberOfPages) - KB_TO_BYTE(4)))
            return w25qxx_Handle->status; // The boundaries of erase operation beyond memory

        /* Command */
        w25qxx_WriteEnable(w25qxx_Handle);
        CS_LOW(w25qxx_Handle);
        w25qxx_Handle->CMD = W25QXX_CMD_SECTOR_ERASE_4KB;
        w25qxx_SPI_Transmit(w25qxx_Handle->hspix, &w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);

        /* A23-A0 - Start address of the desired page */
        ADDRESS_BYTES_SWAP(w25qxx_Handle, address);
        w25qxx_SPI_Transmit(w25qxx_Handle->hspix, w25qxx_Handle->addressBytes, sizeof(w25qxx_Handle->addressBytes),
                            W25QXX_TX_TIMEOUT);
        CS_HIGH(w25qxx_Handle);

        /* Wait options */
        if (waitForTask == W25QXX_WAIT_DELAY)
            w25qxx_Delay(W25QXX_SECTOR_ERASE_TIME_4KB);
        else if (waitForTask == W25QXX_WAIT_BUSY)
        {
            if (w25qxx_WaitWithTimeout(w25qxx_Handle, W25QXX_SECTOR_ERASE_TIME_4KB) != SUCCESS)
                return w25qxx_Handle->status;
        }
        break;

    case W25QXX_BLOCK_ERASE_32KB:
        if ((address % KB_TO_BYTE(32)) != 0)
            return w25qxx_Handle->status; // Incorrect start address for block
        if (address > ((W25QXX_PAGE_SIZE * w25qxx_Handle->numberOfPages) - KB_TO_BYTE(32)))
            return w25qxx_Handle->status; // The boundaries of erase operation beyond memory

        /* Command */
        w25qxx_WriteEnable(w25qxx_Handle);
        CS_LOW(w25qxx_Handle);
        w25qxx_Handle->CMD = W25QXX_CMD_BLOCK_ERASE_32KB;
        w25qxx_SPI_Transmit(w25qxx_Handle->hspix, &w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);

        /* A23-A0 - Start address of the desired page */
        ADDRESS_BYTES_SWAP(w25qxx_Handle, address);
        w25qxx_SPI_Transmit(w25qxx_Handle->hspix, w25qxx_Handle->addressBytes, sizeof(w25qxx_Handle->addressBytes),
                            W25QXX_TX_TIMEOUT);
        CS_HIGH(w25qxx_Handle);

        /* Wait options */
        if (waitForTask == W25QXX_WAIT_DELAY)
            w25qxx_Delay(W25QXX_BLOCK_ERASE_TIME_32KB);
        else if (waitForTask == W25QXX_WAIT_BUSY)
        {
            if (w25qxx_WaitWithTimeout(w25qxx_Handle, W25QXX_BLOCK_ERASE_TIME_32KB) != SUCCESS)
                return w25qxx_Handle->status;
        }
        break;

    case W25QXX_BLOCK_ERASE_64KB:
        if ((address % KB_TO_BYTE(64)) != 0)
            return w25qxx_Handle->status; // Incorrect start address for block
        if (address > ((W25QXX_PAGE_SIZE * w25qxx_Handle->numberOfPages) - KB_TO_BYTE(64)))
            return w25qxx_Handle->status; // The boundaries of erase operation beyond memory

        /* Command */
        w25qxx_WriteEnable(w25qxx_Handle);
        CS_LOW(w25qxx_Handle);
        w25qxx_Handle->CMD = W25QXX_CMD_BLOCK_ERASE_64KB;
        w25qxx_SPI_Transmit(w25qxx_Handle->hspix, &w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);

        /* A23-A0 - Start address of the desired page */
        ADDRESS_BYTES_SWAP(w25qxx_Handle, address);
        w25qxx_SPI_Transmit(w25qxx_Handle->hspix, w25qxx_Handle->addressBytes, sizeof(w25qxx_Handle->addressBytes),
                            W25QXX_TX_TIMEOUT);
        CS_HIGH(w25qxx_Handle);

        /* Wait options */
        if (waitForTask == W25QXX_WAIT_DELAY)
            w25qxx_Delay(W25QXX_BLOCK_ERASE_TIME_64KB);
        else if (waitForTask == W25QXX_WAIT_BUSY)
        {
            if (w25qxx_WaitWithTimeout(w25qxx_Handle, W25QXX_BLOCK_ERASE_TIME_64KB) != SUCCESS)
                return w25qxx_Handle->status;
        }
        break;

    case W25QXX_CHIP_ERASE:
        if (address != NULL)
            return w25qxx_Handle->status;

        /* Command */
        w25qxx_WriteEnable(w25qxx_Handle);
        CS_LOW(w25qxx_Handle);
        w25qxx_Handle->CMD = W25QXX_CMD_CHIP_ERASE;
        w25qxx_SPI_Transmit(w25qxx_Handle->hspix, &w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);
        CS_HIGH(w25qxx_Handle);

        /* Wait options */
        if (waitForTask == W25QXX_WAIT_DELAY)
            w25qxx_Delay(W25QXX_CHIP_ERASE_TIME);
        else if (waitForTask == W25QXX_WAIT_BUSY)
        {
            if (w25qxx_WaitWithTimeout(w25qxx_Handle, W25QXX_CHIP_ERASE_TIME) != SUCCESS)
                return w25qxx_Handle->status;
        }
        break;

    default:
        return w25qxx_Handle->status; // Invalid erase instruction
    }

    return w25qxx_Handle->status = SUCCESS;
}

bool w25qxx_Busy(w25qxx_HandleTypeDef *w25qxx_Handle)
{
    w25qxx_ReadStatus(w25qxx_Handle, 1);

    return READ_BIT(w25qxx_Handle->statusRegister, 1 << 1);
}

/**
 * @section Private functions
 */
// static ErrorStatus w25qxx_WriteStatus(w25qxx_HandleTypeDef *w25qxx_Handle, uint8_t statusRegisterx)
// {
//     w25qxx_Handle->status = ERROR;

// if (w25qxx_WaitWithTimeout(w25qxx_Handle, W25QXX_RESPONSE_TIMEOUT) != SUCCESS)
//     return w25qxx_Handle->status;

// CS_LOW(w25qxx_Handle);
// w25qxx_Handle->CMD = W25QXX_CMD_VOLATILE_SR_WRITE_ENABLE;
// w25qxx_SPI_Transmit(w25qxx_Handle->hspix, &w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);
// CS_HIGH(w25qxx_Handle);

// switch (statusRegisterx)
// {
// case 1:
//     CS_LOW(w25qxx_Handle);
//     w25qxx_Handle->CMD = W25QXX_CMD_WRITE_STATUS_REGISTER1;
//     w25qxx_SPI_Transmit(w25qxx_Handle->hspix, &w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);
//     w25qxx_SPI_Transmit(w25qxx_Handle->hspix, &w25qxx_Handle->statusRegister, sizeof(w25qxx_Handle->statusRegister),
//                         W25QXX_TX_TIMEOUT);
//     CS_HIGH(w25qxx_Handle);
//     break;

// case 2:
//     CS_LOW(w25qxx_Handle);
//     w25qxx_Handle->CMD = W25QXX_CMD_WRITE_STATUS_REGISTER2;
//     w25qxx_SPI_Transmit(w25qxx_Handle->hspix, &w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);
//     w25qxx_SPI_Transmit(w25qxx_Handle->hspix, &w25qxx_Handle->statusRegister, sizeof(w25qxx_Handle->statusRegister),
//                         W25QXX_TX_TIMEOUT);
//     CS_HIGH(w25qxx_Handle);
//     break;

// case 3:
//     CS_LOW(w25qxx_Handle);
//     w25qxx_Handle->CMD = W25QXX_CMD_WRITE_STATUS_REGISTER3;
//     w25qxx_SPI_Transmit(w25qxx_Handle->hspix, &w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);
//     w25qxx_SPI_Transmit(w25qxx_Handle->hspix, &w25qxx_Handle->statusRegister, sizeof(w25qxx_Handle->statusRegister),
//                         W25QXX_TX_TIMEOUT);
//     CS_HIGH(w25qxx_Handle);
//     break;

// default:
//     return w25qxx_Handle->status;
// }

// return w25qxx_Handle->status = SUCCESS;
// }

static void w25qxx_ReadStatus(w25qxx_HandleTypeDef *w25qxx_Handle, uint8_t statusRegisterx)
{
    switch (statusRegisterx)
    {
    case 1:
        CS_LOW(w25qxx_Handle);
        w25qxx_Handle->CMD = W25QXX_CMD_READ_STATUS_REGISTER1;
        w25qxx_SPI_Transmit(w25qxx_Handle->hspix, &w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);
        w25qxx_SPI_Receive(w25qxx_Handle->hspix, &w25qxx_Handle->statusRegister, sizeof(w25qxx_Handle->statusRegister),
                           W25QXX_RX_TIMEOUT);
        CS_HIGH(w25qxx_Handle);
        break;

    case 2:
        CS_LOW(w25qxx_Handle);
        w25qxx_Handle->CMD = W25QXX_CMD_READ_STATUS_REGISTER2;
        w25qxx_SPI_Transmit(w25qxx_Handle->hspix, &w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);
        w25qxx_SPI_Receive(w25qxx_Handle->hspix, &w25qxx_Handle->statusRegister, sizeof(w25qxx_Handle->statusRegister),
                           W25QXX_RX_TIMEOUT);
        CS_HIGH(w25qxx_Handle);
        break;

    case 3:
        CS_LOW(w25qxx_Handle);
        w25qxx_Handle->CMD = W25QXX_CMD_READ_STATUS_REGISTER2;
        w25qxx_SPI_Transmit(w25qxx_Handle->hspix, &w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);
        w25qxx_SPI_Receive(w25qxx_Handle->hspix, &w25qxx_Handle->statusRegister, sizeof(w25qxx_Handle->statusRegister),
                           W25QXX_RX_TIMEOUT);
        CS_HIGH(w25qxx_Handle);
        break;
    }
}

static ErrorStatus w25qxx_ReadID(w25qxx_HandleTypeDef *w25qxx_Handle)
{
    w25qxx_Handle->status = ERROR;

    if (w25qxx_WaitWithTimeout(w25qxx_Handle, W25QXX_RESPONSE_TIMEOUT) != SUCCESS)
        return w25qxx_Handle->status;

    /* Command */
    CS_LOW(w25qxx_Handle);
    w25qxx_Handle->CMD = W25QXX_CMD_MANUFACTURER_DEVICE_ID;
    w25qxx_SPI_Transmit(w25qxx_Handle->hspix, &w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);

    /* 24-bit address (A23-A0) of 000000h */
    ADDRESS_BYTES_SWAP(w25qxx_Handle, (uint32_t) 0);
    w25qxx_SPI_Transmit(w25qxx_Handle->hspix, w25qxx_Handle->addressBytes, sizeof(w25qxx_Handle->addressBytes),
                        W25QXX_TX_TIMEOUT);

    /* Get Manufacturer ID and Device ID */
    w25qxx_SPI_Receive(w25qxx_Handle->hspix, w25qxx_Handle->ID, sizeof(w25qxx_Handle->ID), W25QXX_RX_TIMEOUT);
    CS_HIGH(w25qxx_Handle);

    return w25qxx_Handle->status = SUCCESS;
}

static ErrorStatus w25qxx_WaitWithTimeout(w25qxx_HandleTypeDef *w25qxx_Handle, uint32_t timeout)
{
    w25qxx_Handle->status = ERROR;
    uint32_t tickStart = uwTick;

    while ((uwTick - tickStart) < timeout)
    {
        if (!w25qxx_Busy(w25qxx_Handle))
            return w25qxx_Handle->status = SUCCESS;
    }
    return w25qxx_Handle->status;
}

static void w25qxx_WriteEnable(w25qxx_HandleTypeDef *w25qxx_Handle)
{
    CS_LOW(w25qxx_Handle);
    w25qxx_Handle->CMD = W25QXX_CMD_WRITE_ENABLE;
    w25qxx_SPI_Transmit(w25qxx_Handle->hspix, &w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);
    CS_HIGH(w25qxx_Handle);
}

// static void w25qxx_WriteDisable(w25qxx_HandleTypeDef *w25qxx_Handle)
// {
//     CS_LOW(w25qxx_Handle);
//     w25qxx_Handle->CMD = W25QXX_CMD_WRITE_DISABLE;
//     w25qxx_SPI_Transmit(w25qxx_Handle->hspix, &w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);
//     CS_HIGH(w25qxx_Handle);
// }

static uint16_t ModBus_CRC(const uint8_t *pBuffer, uint16_t bufSize)
{
    uint16_t CRC16 = 0xffff;
    uint16_t i, j;
    for (i = 0; i < bufSize; i++)
    {
        CRC16 ^= pBuffer[i];
        for (j = 0; j < 8; j++)
        {
            if (CRC16 & 1)
                CRC16 = (CRC16 >> 1) ^ 0xA001;
            else
                CRC16 = (CRC16 >> 1);
        }
    }
    return CRC16;
}