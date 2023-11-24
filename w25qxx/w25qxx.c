#include "w25qxx.h"

/* Private function prototypes */
static void w25qxx_ReadID(w25qxx_HandleTypeDef *w25qxx_Handle);
// static void w25qxx_WriteStatus(w25qxx_HandleTypeDef *w25qxx_Handle, uint8_t statusRegisterx);
static void w25qxx_ReadStatus(w25qxx_HandleTypeDef *w25qxx_Handle, uint8_t statusRegisterx);
static ErrorStatus w25qxx_WaitWithTimeout(w25qxx_HandleTypeDef *w25qxx_Handle, uint32_t timeout);
static void w25qxx_WriteEnable(w25qxx_HandleTypeDef *w25qxx_Handle);
// static void w25qxx_WriteDisable(w25qxx_HandleTypeDef *w25qxx_Handle);
static uint16_t ModBus_CRC(const uint8_t *pBuffer, uint16_t bufSize);

w25qxx_Status_t w25qxx_Init(w25qxx_HandleTypeDef *w25qxx_Handle, SPI_HandleTypeDef *hspix, GPIO_TypeDef *CS_Port,
                            uint16_t CS_Pin)
{
    /* Argument guards */
    if (w25qxx_Handle == NULL)
        return W25QXX_STATUS_ERROR_ARGUMENT;
    if (hspix == NULL)
        return w25qxx_Handle->status = W25QXX_STATUS_ERROR_ARGUMENT;
    if (CS_Port == NULL)
        return w25qxx_Handle->status = W25QXX_STATUS_ERROR_ARGUMENT;
    if (CS_Pin == 0x0000)
        return w25qxx_Handle->status = W25QXX_STATUS_ERROR_ARGUMENT;

    w25qxx_Delay(100);

    /* SPI device specific info retrieve */
    w25qxx_Handle->hspix = hspix;
    w25qxx_Handle->CS_Port = CS_Port;
    w25qxx_Handle->CS_Pin = CS_Pin;
    w25qxx_Handle->status = W25QXX_STATUS_RESET;

    /* Check for SPI1-3 match */
    if ((w25qxx_Handle->hspix->Instance != SPI1) && (w25qxx_Handle->hspix->Instance != SPI2) &&
        (w25qxx_Handle->hspix->Instance != SPI3))
        return w25qxx_Handle->status = W25QXX_STATUS_ERROR_INITIALIZATION;

    /* Release from power-down */
    w25qxx_Handle->CMD = W25QXX_CMD_RELEASE_POWER_DOWN;
    CS_LOW(w25qxx_Handle);
    w25qxx_SPI_Transmit(w25qxx_Handle->hspix, &w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);
    CS_HIGH(w25qxx_Handle);
    w25qxx_Delay(1);

    /* Reset device*/
    w25qxx_Handle->CMD = W25QXX_CMD_ENABLE_RESET;
    CS_LOW(w25qxx_Handle);
    w25qxx_SPI_Transmit(w25qxx_Handle->hspix, &w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);
    CS_HIGH(w25qxx_Handle);
    w25qxx_Handle->CMD = W25QXX_CMD_RESET_DEVICE;
    CS_LOW(w25qxx_Handle);
    w25qxx_SPI_Transmit(w25qxx_Handle->hspix, &w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);
    CS_HIGH(w25qxx_Handle);
    w25qxx_Delay(1);

    /* Get the Manufacturer ID and Device ID */
    w25qxx_ReadID(w25qxx_Handle);
    if (w25qxx_Handle->ID[0] != W25QXX_MANUFACTURER_ID)
        return w25qxx_Handle->status = W25QXX_STATUS_ERROR_INITIALIZATION;

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
        return w25qxx_Handle->status = W25QXX_STATUS_ERROR_INITIALIZATION;
    }

    w25qxx_Delay(10);

    return w25qxx_Handle->status = W25QXX_STATUS_READY;
}

w25qxx_Status_t w25qxx_Write(w25qxx_HandleTypeDef *w25qxx_Handle, const uint8_t *buf, uint16_t dataLength,
                             uint32_t address, bool trailingCRC, w25qxx_WaitForTask_t waitForTask)
{
    w25qxx_Handle->status = W25QXX_STATUS_BUSY_WRITE;
    uint16_t frameLength = dataLength;
    uint16_t CRC16 = 0x0000;

    /* Argument guards */
    if ((dataLength == 0) || (buf == NULL))
        return w25qxx_Handle->status = W25QXX_STATUS_ERROR_ARGUMENT;
    if (trailingCRC)
        frameLength += sizeof(CRC16);
    if (frameLength > W25QXX_PAGE_SIZE)
        return w25qxx_Handle->status = W25QXX_STATUS_ERROR_ARGUMENT;
    if ((address % W25QXX_PAGE_SIZE) != 0)
        return w25qxx_Handle->status = W25QXX_STATUS_ERROR_ARGUMENT;
    if (address > (W25QXX_PAGE_SIZE * (w25qxx_Handle->numberOfPages - 1)))
        return w25qxx_Handle->status = W25QXX_STATUS_ERROR_ARGUMENT;

    /* Checksum calculate */
    if (trailingCRC)
        CRC16 = ModBus_CRC(buf, dataLength);

    if (w25qxx_WaitWithTimeout(w25qxx_Handle, W25QXX_RESPONSE_TIMEOUT) != SUCCESS)
        return w25qxx_Handle->status = W25QXX_STATUS_ERROR_TIMEOUT;

    /* Command */
    w25qxx_WriteEnable(w25qxx_Handle);
    w25qxx_Handle->CMD = W25QXX_CMD_PAGE_PROGRAM;
    CS_LOW(w25qxx_Handle);
    w25qxx_SPI_Transmit(w25qxx_Handle->hspix, &w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);

    /* A23-A0 - Start address of the desired page */
    ADDRESS_BYTES_SWAP(w25qxx_Handle, address);
    w25qxx_SPI_Transmit(w25qxx_Handle->hspix, w25qxx_Handle->addressBytes, sizeof(w25qxx_Handle->addressBytes),
                        W25QXX_TX_TIMEOUT);

    /* Data */
    w25qxx_SPI_Transmit(w25qxx_Handle->hspix, (uint8_t *) buf, dataLength, W25QXX_TX_TIMEOUT);

    /* Checksum */
    if (trailingCRC)
        w25qxx_SPI_Transmit(w25qxx_Handle->hspix, (uint8_t *) &CRC16, sizeof(CRC16), W25QXX_TX_TIMEOUT);
    CS_HIGH(w25qxx_Handle);

    /* Wait options */
    if (waitForTask == W25QXX_WAIT_DELAY)
        w25qxx_Delay(W25QXX_PAGE_PROGRAM_TIME);
    else if (waitForTask == W25QXX_WAIT_BUSY)
    {
        if (w25qxx_WaitWithTimeout(w25qxx_Handle, W25QXX_PAGE_PROGRAM_TIME) != SUCCESS)
            return w25qxx_Handle->status = W25QXX_STATUS_ERROR_TIMEOUT;
    }

    return w25qxx_Handle->status = W25QXX_STATUS_READY;
}

w25qxx_Status_t w25qxx_Read(w25qxx_HandleTypeDef *w25qxx_Handle, uint8_t *buf, uint16_t dataLength, uint32_t address,
                            bool trailingCRC, bool fastRead)
{
    w25qxx_Handle->status = W25QXX_STATUS_BUSY_READ;
    uint16_t frameLength = dataLength;
    uint16_t CRC16 = 0x0000;

    /* Argument guards */
    if ((dataLength == 0) || (buf == NULL))
        return w25qxx_Handle->status = W25QXX_STATUS_ERROR_ARGUMENT;
    if (trailingCRC)
        frameLength += sizeof(CRC16);
    if (frameLength > W25QXX_PAGE_SIZE)
        return w25qxx_Handle->status = W25QXX_STATUS_ERROR_ARGUMENT;
    if ((address % W25QXX_PAGE_SIZE) != 0)
        return w25qxx_Handle->status = W25QXX_STATUS_ERROR_ARGUMENT;
    if (address > (W25QXX_PAGE_SIZE * (w25qxx_Handle->numberOfPages - 1)))
        return w25qxx_Handle->status = W25QXX_STATUS_ERROR_ARGUMENT;

    if (w25qxx_WaitWithTimeout(w25qxx_Handle, W25QXX_RESPONSE_TIMEOUT) != SUCCESS)
        return w25qxx_Handle->status = W25QXX_STATUS_ERROR_TIMEOUT;

    /* Frame buffer operations */
    uint8_t *frameBuf = malloc(sizeof(*frameBuf) * frameLength);
    if (frameBuf == NULL)
        return w25qxx_Handle->status = W25QXX_STATUS_ERROR_MEM_MANAGE;

    /* Command */
    w25qxx_Handle->CMD = fastRead ? W25QXX_CMD_FAST_READ : W25QXX_CMD_READ_DATA;
    CS_LOW(w25qxx_Handle);
    w25qxx_SPI_Transmit(w25qxx_Handle->hspix, &w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);

    /* A23-A0 - Start address of the desired page */
    ADDRESS_BYTES_SWAP(w25qxx_Handle, address);
    w25qxx_SPI_Transmit(w25qxx_Handle->hspix, w25qxx_Handle->addressBytes, sizeof(w25qxx_Handle->addressBytes),
                        W25QXX_TX_TIMEOUT);

    /* Data receive */
    if (fastRead)
        w25qxx_SPI_Transmit(w25qxx_Handle->hspix, &w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);
    w25qxx_SPI_Receive(w25qxx_Handle->hspix, frameBuf, frameLength, W25QXX_RX_TIMEOUT);
    CS_HIGH(w25qxx_Handle);

    /* Checksum compare */
    if (trailingCRC)
    {
        CRC16 = ModBus_CRC(frameBuf, dataLength);
        if (memcmp(&frameBuf[dataLength], &CRC16, sizeof(CRC16)) != 0)
        {
            free(frameBuf);
            return w25qxx_Handle->status = W25QXX_STATUS_ERROR_CHECKSUM;
        }
    }

    /* Copy received data without checksum to the destination buffer */
    memcpy(buf, frameBuf, dataLength);
    free(frameBuf);

    return w25qxx_Handle->status = W25QXX_STATUS_READY;
}

w25qxx_Status_t w25qxx_Erase(w25qxx_HandleTypeDef *w25qxx_Handle, w25qxx_EraseInstruction_t eraseInstruction,
                             uint32_t address, w25qxx_WaitForTask_t waitForTask)
{
    w25qxx_Handle->status = W25QXX_STATUS_BUSY_ERASE;

    if (w25qxx_WaitWithTimeout(w25qxx_Handle, W25QXX_RESPONSE_TIMEOUT) != SUCCESS)
        return w25qxx_Handle->status = W25QXX_STATUS_ERROR_TIMEOUT;

    switch (eraseInstruction)
    {
    case W25QXX_SECTOR_ERASE_4KB:
        if ((address % W25QXX_SECTOR_SIZE_4KB) != 0)
            return w25qxx_Handle->status = W25QXX_STATUS_ERROR_ARGUMENT;
        if (address > ((W25QXX_PAGE_SIZE * w25qxx_Handle->numberOfPages) - W25QXX_SECTOR_SIZE_4KB))
            return w25qxx_Handle->status = W25QXX_STATUS_ERROR_ARGUMENT;

        /* Command */
        w25qxx_WriteEnable(w25qxx_Handle);
        w25qxx_Handle->CMD = W25QXX_CMD_SECTOR_ERASE_4KB;
        CS_LOW(w25qxx_Handle);
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
                return w25qxx_Handle->status = W25QXX_STATUS_ERROR_TIMEOUT;
        }
        break;

    case W25QXX_BLOCK_ERASE_32KB:
        if ((address % W25QXX_BLOCK_SIZE_32KB) != 0)
            return w25qxx_Handle->status = W25QXX_STATUS_ERROR_ARGUMENT;
        if (address > ((W25QXX_PAGE_SIZE * w25qxx_Handle->numberOfPages) - W25QXX_BLOCK_SIZE_32KB))
            return w25qxx_Handle->status = W25QXX_STATUS_ERROR_ARGUMENT;

        /* Command */
        w25qxx_WriteEnable(w25qxx_Handle);
        w25qxx_Handle->CMD = W25QXX_CMD_BLOCK_ERASE_32KB;
        CS_LOW(w25qxx_Handle);
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
                return w25qxx_Handle->status = W25QXX_STATUS_ERROR_TIMEOUT;
        }
        break;

    case W25QXX_BLOCK_ERASE_64KB:
        if ((address % W25QXX_BLOCK_SIZE_64KB) != 0)
            return w25qxx_Handle->status = W25QXX_STATUS_ERROR_ARGUMENT;
        if (address > ((W25QXX_PAGE_SIZE * w25qxx_Handle->numberOfPages) - W25QXX_BLOCK_SIZE_64KB))
            return w25qxx_Handle->status = W25QXX_STATUS_ERROR_ARGUMENT;

        /* Command */
        w25qxx_WriteEnable(w25qxx_Handle);
        w25qxx_Handle->CMD = W25QXX_CMD_BLOCK_ERASE_64KB;
        CS_LOW(w25qxx_Handle);
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
                return w25qxx_Handle->status = W25QXX_STATUS_ERROR_TIMEOUT;
        }
        break;

    case W25QXX_CHIP_ERASE:
        if (address != 0)
            return w25qxx_Handle->status = W25QXX_STATUS_ERROR_ARGUMENT;

        /* Command */
        w25qxx_WriteEnable(w25qxx_Handle);
        w25qxx_Handle->CMD = W25QXX_CMD_CHIP_ERASE;
        CS_LOW(w25qxx_Handle);
        w25qxx_SPI_Transmit(w25qxx_Handle->hspix, &w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);
        CS_HIGH(w25qxx_Handle);

        /* Wait options */
        if (waitForTask == W25QXX_WAIT_DELAY)
            w25qxx_Delay(W25QXX_CHIP_ERASE_TIME);
        else if (waitForTask == W25QXX_WAIT_BUSY)
        {
            if (w25qxx_WaitWithTimeout(w25qxx_Handle, W25QXX_CHIP_ERASE_TIME) != SUCCESS)
                return w25qxx_Handle->status = W25QXX_STATUS_ERROR_TIMEOUT;
        }
        break;

    default:
        return w25qxx_Handle->status = W25QXX_STATUS_ERROR_INSTRUCTION;
    }

    return w25qxx_Handle->status = W25QXX_STATUS_READY;
}

bool w25qxx_Busy(w25qxx_HandleTypeDef *w25qxx_Handle)
{
    w25qxx_ReadStatus(w25qxx_Handle, 1);

    return READ_BIT(w25qxx_Handle->statusRegister, 1u << 0);
}

/**
 * @section Private functions
 */
static void w25qxx_ReadID(w25qxx_HandleTypeDef *w25qxx_Handle)
{
    /* Command */
    w25qxx_Handle->CMD = W25QXX_CMD_MANUFACTURER_DEVICE_ID;
    CS_LOW(w25qxx_Handle);
    w25qxx_SPI_Transmit(w25qxx_Handle->hspix, &w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);

    /* 24-bit address (A23-A0) of 000000h */
    ADDRESS_BYTES_SWAP(w25qxx_Handle, (uint32_t) 0);
    w25qxx_SPI_Transmit(w25qxx_Handle->hspix, w25qxx_Handle->addressBytes, sizeof(w25qxx_Handle->addressBytes),
                        W25QXX_TX_TIMEOUT);

    /* Get Manufacturer ID and Device ID */
    w25qxx_SPI_Receive(w25qxx_Handle->hspix, w25qxx_Handle->ID, sizeof(w25qxx_Handle->ID), W25QXX_RX_TIMEOUT);
    CS_HIGH(w25qxx_Handle);
}

// static void w25qxx_WriteStatus(w25qxx_HandleTypeDef *w25qxx_Handle, uint8_t statusRegisterx)
// {
//     w25qxx_Handle->CMD = W25QXX_CMD_VOLATILE_SR_WRITE_ENABLE;
//     CS_LOW(w25qxx_Handle);
//     w25qxx_SPI_Transmit(w25qxx_Handle->hspix, &w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);
//     CS_HIGH(w25qxx_Handle);

// switch (statusRegisterx)
// {
// case 1:
//     w25qxx_Handle->CMD = W25QXX_CMD_WRITE_STATUS_REGISTER1;
//     CS_LOW(w25qxx_Handle);
//     w25qxx_SPI_Transmit(w25qxx_Handle->hspix, &w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);
//     w25qxx_SPI_Transmit(w25qxx_Handle->hspix, &w25qxx_Handle->statusRegister, sizeof(w25qxx_Handle->statusRegister),
//                         W25QXX_TX_TIMEOUT);
//     CS_HIGH(w25qxx_Handle);
//     break;

// case 2:
//     w25qxx_Handle->CMD = W25QXX_CMD_WRITE_STATUS_REGISTER2;
//     CS_LOW(w25qxx_Handle);
//     w25qxx_SPI_Transmit(w25qxx_Handle->hspix, &w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);
//     w25qxx_SPI_Transmit(w25qxx_Handle->hspix, &w25qxx_Handle->statusRegister, sizeof(w25qxx_Handle->statusRegister),
//                         W25QXX_TX_TIMEOUT);
//     CS_HIGH(w25qxx_Handle);
//     break;

// case 3:
//     w25qxx_Handle->CMD = W25QXX_CMD_WRITE_STATUS_REGISTER3;
//     CS_LOW(w25qxx_Handle);
//     w25qxx_SPI_Transmit(w25qxx_Handle->hspix, &w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);
//     w25qxx_SPI_Transmit(w25qxx_Handle->hspix, &w25qxx_Handle->statusRegister, sizeof(w25qxx_Handle->statusRegister),
//                         W25QXX_TX_TIMEOUT);
//     CS_HIGH(w25qxx_Handle);
//     break;

// default:
//     break;
// }
// }

static void w25qxx_ReadStatus(w25qxx_HandleTypeDef *w25qxx_Handle, uint8_t statusRegisterx)
{
    switch (statusRegisterx)
    {
    case 1:
        w25qxx_Handle->CMD = W25QXX_CMD_READ_STATUS_REGISTER1;
        CS_LOW(w25qxx_Handle);
        w25qxx_SPI_Transmit(w25qxx_Handle->hspix, &w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);
        w25qxx_SPI_Receive(w25qxx_Handle->hspix, &w25qxx_Handle->statusRegister, sizeof(w25qxx_Handle->statusRegister),
                           W25QXX_RX_TIMEOUT);
        CS_HIGH(w25qxx_Handle);
        break;

    case 2:
        w25qxx_Handle->CMD = W25QXX_CMD_READ_STATUS_REGISTER2;
        CS_LOW(w25qxx_Handle);
        w25qxx_SPI_Transmit(w25qxx_Handle->hspix, &w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);
        w25qxx_SPI_Receive(w25qxx_Handle->hspix, &w25qxx_Handle->statusRegister, sizeof(w25qxx_Handle->statusRegister),
                           W25QXX_RX_TIMEOUT);
        CS_HIGH(w25qxx_Handle);
        break;

    case 3:
        w25qxx_Handle->CMD = W25QXX_CMD_READ_STATUS_REGISTER3;
        CS_LOW(w25qxx_Handle);
        w25qxx_SPI_Transmit(w25qxx_Handle->hspix, &w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);
        w25qxx_SPI_Receive(w25qxx_Handle->hspix, &w25qxx_Handle->statusRegister, sizeof(w25qxx_Handle->statusRegister),
                           W25QXX_RX_TIMEOUT);
        CS_HIGH(w25qxx_Handle);
        break;
    }
}

static ErrorStatus w25qxx_WaitWithTimeout(w25qxx_HandleTypeDef *w25qxx_Handle, uint32_t timeout)
{
    uint32_t tickStart = uwTick;

    /* Command */
    w25qxx_Handle->CMD = W25QXX_CMD_READ_STATUS_REGISTER1;
    CS_LOW(w25qxx_Handle);
    w25qxx_SPI_Transmit(w25qxx_Handle->hspix, &w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);

    while ((uwTick - tickStart) < timeout)
    {
        /* Get busy bit state */
        w25qxx_SPI_Receive(w25qxx_Handle->hspix, &w25qxx_Handle->statusRegister, sizeof(w25qxx_Handle->statusRegister),
                           W25QXX_RX_TIMEOUT);
        if (!READ_BIT(w25qxx_Handle->statusRegister, 1u << 0))
        {
            CS_HIGH(w25qxx_Handle);
            return SUCCESS;
        }
    }
    CS_HIGH(w25qxx_Handle);

    return ERROR;
}

static void w25qxx_WriteEnable(w25qxx_HandleTypeDef *w25qxx_Handle)
{
    w25qxx_Handle->CMD = W25QXX_CMD_WRITE_ENABLE;
    CS_LOW(w25qxx_Handle);
    w25qxx_SPI_Transmit(w25qxx_Handle->hspix, &w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);
    CS_HIGH(w25qxx_Handle);
}

// static void w25qxx_WriteDisable(w25qxx_HandleTypeDef *w25qxx_Handle)
// {
//     w25qxx_Handle->CMD = W25QXX_CMD_WRITE_DISABLE;
//     CS_LOW(w25qxx_Handle);
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