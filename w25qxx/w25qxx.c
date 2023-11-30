#include "w25qxx.h"

/* Private function prototypes */
static w25qxx_Error_t w25qxx_PowerDown(w25qxx_HandleTypeDef *w25qxx_Handle);
static w25qxx_Error_t w25qxx_ReleasePowerDown(w25qxx_HandleTypeDef *w25qxx_Handle);
static w25qxx_Error_t w25qxx_ResetDevice(w25qxx_HandleTypeDef *w25qxx_Handle);
static w25qxx_Error_t w25qxx_ReadID(w25qxx_HandleTypeDef *w25qxx_Handle);
static w25qxx_Error_t w25qxx_WriteStatus(w25qxx_HandleTypeDef *w25qxx_Handle, uint8_t statusRegisterx);
static w25qxx_Error_t w25qxx_ReadStatus(w25qxx_HandleTypeDef *w25qxx_Handle, uint8_t statusRegisterx);
static w25qxx_Error_t w25qxx_WriteEnable(w25qxx_HandleTypeDef *w25qxx_Handle);
static w25qxx_Error_t w25qxx_WriteDisable(w25qxx_HandleTypeDef *w25qxx_Handle);
static uint16_t ModBus_CRC(const uint8_t *pBuffer, uint16_t bufSize);

w25qxx_Error_t w25qxx_Link(w25qxx_HandleTypeDef *w25qxx_Handle, ErrorStatus (*fpReceive)(uint8_t *, uint16_t, uint32_t),
                           ErrorStatus (*fpTransmit)(uint8_t *, uint16_t, uint32_t),
                           void (*fpCS_Set)(w25qxx_CS_State_t), void (*fpDelay)(uint32_t))
{
    /* Argument guards */
    if (w25qxx_Handle == NULL)
        goto w25qxx_ErrorHandler;
    if (fpTransmit == NULL)
    {
        w25qxx_Handle->error = W25QXX_ERROR_ARGUMENT;
        goto w25qxx_ErrorHandler;
    }
    if (fpReceive == NULL)
    {
        w25qxx_Handle->error = W25QXX_ERROR_ARGUMENT;
        goto w25qxx_ErrorHandler;
    }
    if (fpCS_Set == NULL)
    {
        w25qxx_Handle->error = W25QXX_ERROR_ARGUMENT;
        goto w25qxx_ErrorHandler;
    }
    if (fpDelay == NULL)
    {
        w25qxx_Handle->error = W25QXX_ERROR_ARGUMENT;
        goto w25qxx_ErrorHandler;
    }

    /* Set up handle fields to its default state */
    memset(w25qxx_Handle, 0, sizeof(*w25qxx_Handle));

    /* Link functions within handle with the user defined ones */
    w25qxx_Handle->interface.Receive = fpReceive;
    w25qxx_Handle->interface.Transmit = fpTransmit;
    w25qxx_Handle->interface.CS_Set = fpCS_Set;
    w25qxx_Handle->interface.Delay = fpDelay;

    /* Operation succeed */
    w25qxx_Handle->status = W25QXX_STATUS_RESET;

w25qxx_ErrorHandler:
    if (w25qxx_Handle == NULL)
        return W25QXX_ERROR_REFERENCE_HANDLE;

    return w25qxx_Handle->error;
}

w25qxx_Error_t w25qxx_Init(w25qxx_HandleTypeDef *w25qxx_Handle)
{
    /* Argument guards */
    if (w25qxx_Handle == NULL)
        goto w25qxx_ErrorHandler;

    /* Existing errors check */
    if (w25qxx_Handle->error != W25QXX_ERROR_NONE)
        goto w25qxx_ErrorHandler;

    /* Actual status check */
    if (w25qxx_Handle->status != W25QXX_STATUS_RESET)
    {
        w25qxx_Handle->error = W25QXX_ERROR_STATUS_MATCH;
        goto w25qxx_ErrorHandler;
    }

    /* Start operation */
    w25qxx_Handle->status = W25QXX_STATUS_BUSY_INIT;
    w25qxx_Handle->interface.CS_Set(W25QXX_CS_HIGH);
    w25qxx_Handle->interface.Delay(100);

    if (w25qxx_ReleasePowerDown(w25qxx_Handle) != W25QXX_ERROR_NONE)
    {
        w25qxx_Handle->error = W25QXX_ERROR_INITIALIZATION;
        goto w25qxx_ErrorHandler;
    }

    if (w25qxx_ResetDevice(w25qxx_Handle) != W25QXX_ERROR_NONE)
    {
        w25qxx_Handle->error = W25QXX_ERROR_INITIALIZATION;
        goto w25qxx_ErrorHandler;
    }

    /* Get the Manufacturer ID and Device ID */
    if (w25qxx_ReadID(w25qxx_Handle) != W25QXX_ERROR_NONE)
    {
        w25qxx_Handle->error = W25QXX_ERROR_INITIALIZATION;
        goto w25qxx_ErrorHandler;
    }
    if (w25qxx_Handle->ID[0] != W25QXX_MANUFACTURER_ID)
    {
        w25qxx_Handle->error = W25QXX_ERROR_INITIALIZATION;
        goto w25qxx_ErrorHandler;
    }

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

    /* Unsupported device */
    default:
        w25qxx_Handle->error = W25QXX_ERROR_INITIALIZATION;
        goto w25qxx_ErrorHandler;
    }

    /* Operation succeed */
    w25qxx_Handle->interface.Delay(10);
    w25qxx_Handle->status = W25QXX_STATUS_READY;

w25qxx_ErrorHandler:
    if (w25qxx_Handle == NULL)
        return W25QXX_ERROR_REFERENCE_HANDLE;

    return w25qxx_Handle->error;
}

w25qxx_Error_t w25qxx_ResetError(w25qxx_HandleTypeDef *w25qxx_Handle)
{
    /* Argument guards */
    if (w25qxx_Handle == NULL)
        goto w25qxx_ErrorHandler;

    /* Existing errors check */
    if (w25qxx_Handle->error == W25QXX_ERROR_NONE)
        goto w25qxx_ErrorHandler;

    /* Reset error */
    w25qxx_Handle->error = W25QXX_ERROR_NONE;

    /* Try to get response from device */
    w25qxx_WaitWithTimeout(w25qxx_Handle, W25QXX_RESPONSE_TIMEOUT);

w25qxx_ErrorHandler:
    if (w25qxx_Handle == NULL)
        return W25QXX_ERROR_REFERENCE_HANDLE;

    return w25qxx_Handle->error;
}

w25qxx_Error_t w25qxx_Write(w25qxx_HandleTypeDef *w25qxx_Handle, const uint8_t *buf, uint16_t dataLength,
                            uint32_t address, bool trailingCRC, w25qxx_WaitForTask_t waitForTask)
{
    /* Argument guards */
    if (w25qxx_Handle == NULL)
        goto w25qxx_ErrorHandler;
    w25qxx_Handle->frameLength = dataLength;
    if (buf == NULL)
    {
        w25qxx_Handle->error = W25QXX_ERROR_ARGUMENT;
        goto w25qxx_ErrorHandler;
    }
    if (dataLength == 0)
    {
        w25qxx_Handle->error = W25QXX_ERROR_ARGUMENT;
        goto w25qxx_ErrorHandler;
    }
    if (trailingCRC)
        w25qxx_Handle->frameLength += sizeof(w25qxx_Handle->CRC16);
    if (w25qxx_Handle->frameLength > W25QXX_PAGE_SIZE)
    {
        w25qxx_Handle->error = W25QXX_ERROR_ADDRESS;
        goto w25qxx_ErrorHandler;
    }
    if ((address % W25QXX_PAGE_SIZE) != 0)
    {
        w25qxx_Handle->error = W25QXX_ERROR_ADDRESS;
        goto w25qxx_ErrorHandler;
    }
    if (address > (W25QXX_PAGE_SIZE * (w25qxx_Handle->numberOfPages - 1)))
    {
        w25qxx_Handle->error = W25QXX_ERROR_ADDRESS;
        goto w25qxx_ErrorHandler;
    }

    /* Existing errors check */
    if (w25qxx_Handle->error != W25QXX_ERROR_NONE)
        goto w25qxx_ErrorHandler;

    /* Actual status check */
    if (w25qxx_Handle->status != W25QXX_STATUS_READY)
    {
        w25qxx_Handle->error = W25QXX_ERROR_STATUS_MATCH;
        goto w25qxx_ErrorHandler;
    }

    /* Checksum calculate */
    if (trailingCRC)
        w25qxx_Handle->CRC16 = ModBus_CRC(buf, dataLength);

    if (w25qxx_WaitWithTimeout(w25qxx_Handle, W25QXX_RESPONSE_TIMEOUT) != W25QXX_ERROR_NONE)
    {
        w25qxx_Handle->error = W25QXX_ERROR_TIMEOUT;
        goto w25qxx_ErrorHandler;
    }

    /* Actual status update */
    w25qxx_Handle->status = W25QXX_STATUS_BUSY_WRITE;

    /* Command */
    if (w25qxx_WriteEnable(w25qxx_Handle) != W25QXX_ERROR_NONE)
    {
        w25qxx_Handle->error = W25QXX_ERROR_SPI;
        goto w25qxx_ErrorHandler;
    }
    w25qxx_Handle->CMD = W25QXX_CMD_PAGE_PROGRAM;
    w25qxx_Handle->interface.CS_Set(W25QXX_CS_LOW);
    if (w25qxx_Handle->interface.Transmit(&w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT) !=
        SUCCESS)
    {
        w25qxx_Handle->error = W25QXX_ERROR_SPI;
        goto w25qxx_ErrorHandler;
    }

    /* A23-A0 - Start address of the desired page */
    ADDRESS_BYTES_SWAP(w25qxx_Handle, address);
    if (w25qxx_Handle->interface.Transmit(w25qxx_Handle->addressBytes, sizeof(w25qxx_Handle->addressBytes),
                                          W25QXX_TX_TIMEOUT) != SUCCESS)
    {
        w25qxx_Handle->error = W25QXX_ERROR_SPI;
        goto w25qxx_ErrorHandler;
    }

    /* Data */
    if (w25qxx_Handle->interface.Transmit((uint8_t *) buf, dataLength, W25QXX_TX_TIMEOUT) != SUCCESS)
    {
        w25qxx_Handle->error = W25QXX_ERROR_SPI;
        goto w25qxx_ErrorHandler;
    }

    /* Checksum */
    if (trailingCRC)
    {
        if (w25qxx_Handle->interface.Transmit((uint8_t *) &w25qxx_Handle->CRC16, sizeof(w25qxx_Handle->CRC16),
                                              W25QXX_TX_TIMEOUT) != SUCCESS)
        {
            w25qxx_Handle->error = W25QXX_ERROR_SPI;
            goto w25qxx_ErrorHandler;
        }
    }
    w25qxx_Handle->interface.CS_Set(W25QXX_CS_HIGH);

    /* Wait options */
    switch (waitForTask)
    {
    case W25QXX_WAIT_NO:
        break;

    case W25QXX_WAIT_DELAY:
        w25qxx_Handle->interface.Delay(W25QXX_PAGE_PROGRAM_TIME);
        w25qxx_Handle->status = W25QXX_STATUS_READY;
        break;

    case W25QXX_WAIT_BUSY:
        if (w25qxx_WaitWithTimeout(w25qxx_Handle, W25QXX_PAGE_PROGRAM_TIME) != W25QXX_ERROR_NONE)
        {
            w25qxx_Handle->error = W25QXX_ERROR_TIMEOUT;
            goto w25qxx_ErrorHandler;
        }
        break;

    default:
        {
            w25qxx_Handle->error = W25QXX_ERROR_ARGUMENT;
            goto w25qxx_ErrorHandler;
        }
        break;
    }

w25qxx_ErrorHandler:
    if (w25qxx_Handle == NULL)
        return W25QXX_ERROR_REFERENCE_HANDLE;

    w25qxx_Handle->interface.CS_Set(W25QXX_CS_HIGH);
    return w25qxx_Handle->error;
}

w25qxx_Error_t w25qxx_Read(w25qxx_HandleTypeDef *w25qxx_Handle, uint8_t *buf, uint16_t dataLength, uint32_t address,
                           bool trailingCRC, bool fastRead)
{
    uint8_t *frameBuf = NULL;

    /* Argument guards */
    if (w25qxx_Handle == NULL)
        goto w25qxx_ErrorHandler;
    w25qxx_Handle->frameLength = dataLength;
    if (buf == NULL)
    {
        w25qxx_Handle->error = W25QXX_ERROR_ARGUMENT;
        goto w25qxx_ErrorHandler;
    }
    if (dataLength == 0)
    {
        w25qxx_Handle->error = W25QXX_ERROR_ARGUMENT;
        goto w25qxx_ErrorHandler;
    }
    if (trailingCRC)
        w25qxx_Handle->frameLength += sizeof(w25qxx_Handle->CRC16);
    if (w25qxx_Handle->frameLength > W25QXX_PAGE_SIZE)
    {
        w25qxx_Handle->error = W25QXX_ERROR_ADDRESS;
        goto w25qxx_ErrorHandler;
    }
    if ((address % W25QXX_PAGE_SIZE) != 0)
    {
        w25qxx_Handle->error = W25QXX_ERROR_ADDRESS;
        goto w25qxx_ErrorHandler;
    }
    if (address > (W25QXX_PAGE_SIZE * (w25qxx_Handle->numberOfPages - 1)))
    {
        w25qxx_Handle->error = W25QXX_ERROR_ADDRESS;
        goto w25qxx_ErrorHandler;
    }

    /* Existing errors check */
    if (w25qxx_Handle->error != W25QXX_ERROR_NONE)
        goto w25qxx_ErrorHandler;

    /* Actual status check */
    if (w25qxx_Handle->status != W25QXX_STATUS_READY)
    {
        w25qxx_Handle->error = W25QXX_ERROR_STATUS_MATCH;
        goto w25qxx_ErrorHandler;
    }

    if (w25qxx_WaitWithTimeout(w25qxx_Handle, W25QXX_RESPONSE_TIMEOUT) != W25QXX_ERROR_NONE)
    {
        w25qxx_Handle->error = W25QXX_ERROR_TIMEOUT;
        goto w25qxx_ErrorHandler;
    }

    /* Actual status update */
    w25qxx_Handle->status = W25QXX_STATUS_BUSY_READ;

    /* Frame buffer operations */
    frameBuf = malloc(sizeof(*frameBuf) * w25qxx_Handle->frameLength);
    if (frameBuf == NULL)
    {
        w25qxx_Handle->error = W25QXX_ERROR_MEM_MANAGE;
        goto w25qxx_ErrorHandler;
    }

    /* Command */
    w25qxx_Handle->CMD = fastRead ? W25QXX_CMD_FAST_READ : W25QXX_CMD_READ_DATA;
    w25qxx_Handle->interface.CS_Set(W25QXX_CS_LOW);
    if (w25qxx_Handle->interface.Transmit(&w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT) !=
        SUCCESS)
    {
        w25qxx_Handle->error = W25QXX_ERROR_SPI;
        goto w25qxx_ErrorHandler;
    }

    /* A23-A0 - Start address of the desired page */
    ADDRESS_BYTES_SWAP(w25qxx_Handle, address);
    if (w25qxx_Handle->interface.Transmit(w25qxx_Handle->addressBytes, sizeof(w25qxx_Handle->addressBytes),
                                          W25QXX_TX_TIMEOUT) != SUCCESS)
    {
        w25qxx_Handle->error = W25QXX_ERROR_SPI;
        goto w25qxx_ErrorHandler;
    }

    /* 8 dummy clocks */
    if (fastRead)
    {
        if (w25qxx_Handle->interface.Transmit(&w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT) !=
            SUCCESS)
        {
            w25qxx_Handle->error = W25QXX_ERROR_SPI;
            goto w25qxx_ErrorHandler;
        }
    }

    /* Data receive */
    if (w25qxx_Handle->interface.Receive(frameBuf, w25qxx_Handle->frameLength, W25QXX_RX_TIMEOUT) != SUCCESS)
    {
        w25qxx_Handle->error = W25QXX_ERROR_SPI;
        goto w25qxx_ErrorHandler;
    }
    w25qxx_Handle->interface.CS_Set(W25QXX_CS_HIGH);

    /* Checksum compare */
    if (trailingCRC)
    {
        w25qxx_Handle->CRC16 = ModBus_CRC(frameBuf, dataLength);
        if (memcmp(&frameBuf[dataLength], &w25qxx_Handle->CRC16, sizeof(w25qxx_Handle->CRC16)) != 0)
        {
            w25qxx_Handle->error = W25QXX_ERROR_CHECKSUM;
            goto w25qxx_ErrorHandler;
        }
    }

    /* Copy received data without checksum to the destination buffer */
    memcpy(buf, frameBuf, dataLength);

    /* Operation succeed */
    w25qxx_Handle->status = W25QXX_STATUS_READY;

w25qxx_ErrorHandler:
    if (frameBuf != NULL)
        free(frameBuf);
    if (w25qxx_Handle == NULL)
        return W25QXX_ERROR_REFERENCE_HANDLE;

    w25qxx_Handle->interface.CS_Set(W25QXX_CS_HIGH);
    return w25qxx_Handle->error;
}

w25qxx_Error_t w25qxx_Erase(w25qxx_HandleTypeDef *w25qxx_Handle, w25qxx_EraseInstruction_t eraseInstruction,
                            uint32_t address, w25qxx_WaitForTask_t waitForTask)
{
    /* Argument guards */
    if (w25qxx_Handle == NULL)
        goto w25qxx_ErrorHandler;

    /* Existing errors check */
    if (w25qxx_Handle->error != W25QXX_ERROR_NONE)
        goto w25qxx_ErrorHandler;

    /* Actual status check */
    if (w25qxx_Handle->status != W25QXX_STATUS_READY)
    {
        w25qxx_Handle->error = W25QXX_ERROR_STATUS_MATCH;
        goto w25qxx_ErrorHandler;
    }

    if (w25qxx_WaitWithTimeout(w25qxx_Handle, W25QXX_RESPONSE_TIMEOUT) != W25QXX_ERROR_NONE)
    {
        w25qxx_Handle->error = W25QXX_ERROR_TIMEOUT;
        goto w25qxx_ErrorHandler;
    }

    /* Actual status update */
    w25qxx_Handle->status = W25QXX_STATUS_BUSY_ERASE;

    switch (eraseInstruction)
    {
    case W25QXX_SECTOR_ERASE_4KB:
        if ((address % W25QXX_SECTOR_SIZE_4KB) != 0)
        {
            w25qxx_Handle->error = W25QXX_ERROR_ADDRESS;
            goto w25qxx_ErrorHandler;
        }
        if (address > ((W25QXX_PAGE_SIZE * w25qxx_Handle->numberOfPages) - W25QXX_SECTOR_SIZE_4KB))
        {
            w25qxx_Handle->error = W25QXX_ERROR_ADDRESS;
            goto w25qxx_ErrorHandler;
        }

        /* Command */
        if (w25qxx_WriteEnable(w25qxx_Handle) != W25QXX_ERROR_NONE)
        {
            w25qxx_Handle->error = W25QXX_ERROR_SPI;
            goto w25qxx_ErrorHandler;
        }
        w25qxx_Handle->CMD = W25QXX_CMD_SECTOR_ERASE_4KB;
        w25qxx_Handle->interface.CS_Set(W25QXX_CS_LOW);
        if (w25qxx_Handle->interface.Transmit(&w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT) !=
            SUCCESS)
        {
            w25qxx_Handle->error = W25QXX_ERROR_SPI;
            goto w25qxx_ErrorHandler;
        }

        /* A23-A0 - Start address of the desired page */
        ADDRESS_BYTES_SWAP(w25qxx_Handle, address);
        if (w25qxx_Handle->interface.Transmit(w25qxx_Handle->addressBytes, sizeof(w25qxx_Handle->addressBytes),
                                              W25QXX_TX_TIMEOUT) != SUCCESS)
        {
            w25qxx_Handle->error = W25QXX_ERROR_SPI;
            goto w25qxx_ErrorHandler;
        }
        w25qxx_Handle->interface.CS_Set(W25QXX_CS_HIGH);

        /* Wait options */
        switch (waitForTask)
        {
        case W25QXX_WAIT_NO:
            break;

        case W25QXX_WAIT_DELAY:
            w25qxx_Handle->interface.Delay(W25QXX_SECTOR_ERASE_TIME_4KB);
            w25qxx_Handle->status = W25QXX_STATUS_READY;
            break;

        case W25QXX_WAIT_BUSY:
            if (w25qxx_WaitWithTimeout(w25qxx_Handle, W25QXX_SECTOR_ERASE_TIME_4KB) != W25QXX_ERROR_NONE)
            {
                w25qxx_Handle->error = W25QXX_ERROR_TIMEOUT;
                goto w25qxx_ErrorHandler;
            }
            break;

        default:
            {
                w25qxx_Handle->error = W25QXX_ERROR_ARGUMENT;
                goto w25qxx_ErrorHandler;
            }
            break;
        }
        break;

    case W25QXX_BLOCK_ERASE_32KB:
        if ((address % W25QXX_BLOCK_SIZE_32KB) != 0)
        {
            w25qxx_Handle->error = W25QXX_ERROR_ADDRESS;
            goto w25qxx_ErrorHandler;
        }
        if (address > ((W25QXX_PAGE_SIZE * w25qxx_Handle->numberOfPages) - W25QXX_BLOCK_SIZE_32KB))
        {
            w25qxx_Handle->error = W25QXX_ERROR_ADDRESS;
            goto w25qxx_ErrorHandler;
        }

        /* Command */
        if (w25qxx_WriteEnable(w25qxx_Handle) != W25QXX_ERROR_NONE)
        {
            w25qxx_Handle->error = W25QXX_ERROR_SPI;
            goto w25qxx_ErrorHandler;
        }
        w25qxx_Handle->CMD = W25QXX_CMD_BLOCK_ERASE_32KB;
        w25qxx_Handle->interface.CS_Set(W25QXX_CS_LOW);
        if (w25qxx_Handle->interface.Transmit(&w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT) !=
            SUCCESS)
        {
            w25qxx_Handle->error = W25QXX_ERROR_SPI;
            goto w25qxx_ErrorHandler;
        }

        /* A23-A0 - Start address of the desired page */
        ADDRESS_BYTES_SWAP(w25qxx_Handle, address);
        if (w25qxx_Handle->interface.Transmit(w25qxx_Handle->addressBytes, sizeof(w25qxx_Handle->addressBytes),
                                              W25QXX_TX_TIMEOUT) != SUCCESS)
        {
            w25qxx_Handle->error = W25QXX_ERROR_SPI;
            goto w25qxx_ErrorHandler;
        }
        w25qxx_Handle->interface.CS_Set(W25QXX_CS_HIGH);

        /* Wait options */
        switch (waitForTask)
        {
        case W25QXX_WAIT_NO:
            break;

        case W25QXX_WAIT_DELAY:
            w25qxx_Handle->interface.Delay(W25QXX_BLOCK_ERASE_TIME_32KB);
            w25qxx_Handle->status = W25QXX_STATUS_READY;
            break;

        case W25QXX_WAIT_BUSY:
            if (w25qxx_WaitWithTimeout(w25qxx_Handle, W25QXX_BLOCK_ERASE_TIME_32KB) != W25QXX_ERROR_NONE)
            {
                w25qxx_Handle->error = W25QXX_ERROR_TIMEOUT;
                goto w25qxx_ErrorHandler;
            }
            break;

        default:
            {
                w25qxx_Handle->error = W25QXX_ERROR_ARGUMENT;
                goto w25qxx_ErrorHandler;
            }
            break;
        }
        break;

    case W25QXX_BLOCK_ERASE_64KB:
        if ((address % W25QXX_BLOCK_SIZE_64KB) != 0)
        {
            w25qxx_Handle->error = W25QXX_ERROR_ADDRESS;
            goto w25qxx_ErrorHandler;
        }
        if (address > ((W25QXX_PAGE_SIZE * w25qxx_Handle->numberOfPages) - W25QXX_BLOCK_SIZE_64KB))
        {
            w25qxx_Handle->error = W25QXX_ERROR_ADDRESS;
            goto w25qxx_ErrorHandler;
        }

        /* Command */
        if (w25qxx_WriteEnable(w25qxx_Handle) != W25QXX_ERROR_NONE)
        {
            w25qxx_Handle->error = W25QXX_ERROR_SPI;
            goto w25qxx_ErrorHandler;
        }
        w25qxx_Handle->CMD = W25QXX_CMD_BLOCK_ERASE_64KB;
        w25qxx_Handle->interface.CS_Set(W25QXX_CS_LOW);
        if (w25qxx_Handle->interface.Transmit(&w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT) !=
            SUCCESS)
        {
            w25qxx_Handle->error = W25QXX_ERROR_SPI;
            goto w25qxx_ErrorHandler;
        }

        /* A23-A0 - Start address of the desired page */
        ADDRESS_BYTES_SWAP(w25qxx_Handle, address);
        if (w25qxx_Handle->interface.Transmit(w25qxx_Handle->addressBytes, sizeof(w25qxx_Handle->addressBytes),
                                              W25QXX_TX_TIMEOUT) != SUCCESS)
        {
            w25qxx_Handle->error = W25QXX_ERROR_SPI;
            goto w25qxx_ErrorHandler;
        }
        w25qxx_Handle->interface.CS_Set(W25QXX_CS_HIGH);

        /* Wait options */
        switch (waitForTask)
        {
        case W25QXX_WAIT_NO:
            break;

        case W25QXX_WAIT_DELAY:
            w25qxx_Handle->interface.Delay(W25QXX_BLOCK_ERASE_TIME_64KB);
            w25qxx_Handle->status = W25QXX_STATUS_READY;
            break;

        case W25QXX_WAIT_BUSY:
            if (w25qxx_WaitWithTimeout(w25qxx_Handle, W25QXX_BLOCK_ERASE_TIME_64KB) != W25QXX_ERROR_NONE)
            {
                w25qxx_Handle->error = W25QXX_ERROR_TIMEOUT;
                goto w25qxx_ErrorHandler;
            }
            break;

        default:
            {
                w25qxx_Handle->error = W25QXX_ERROR_ARGUMENT;
                goto w25qxx_ErrorHandler;
            }
            break;
        }
        break;

    case W25QXX_CHIP_ERASE:
        if (address != 0)
        {
            w25qxx_Handle->error = W25QXX_ERROR_ADDRESS;
            goto w25qxx_ErrorHandler;
        }

        /* Command */
        if (w25qxx_WriteEnable(w25qxx_Handle) != W25QXX_ERROR_NONE)
        {
            w25qxx_Handle->error = W25QXX_ERROR_SPI;
            goto w25qxx_ErrorHandler;
        }
        w25qxx_Handle->CMD = W25QXX_CMD_CHIP_ERASE;
        w25qxx_Handle->interface.CS_Set(W25QXX_CS_LOW);
        if (w25qxx_Handle->interface.Transmit(&w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT) !=
            SUCCESS)
        {
            w25qxx_Handle->error = W25QXX_ERROR_SPI;
            goto w25qxx_ErrorHandler;
        }
        w25qxx_Handle->interface.CS_Set(W25QXX_CS_HIGH);

        /* Wait options */
        switch (waitForTask)
        {
        case W25QXX_WAIT_NO:
            break;

        case W25QXX_WAIT_DELAY:
            w25qxx_Handle->interface.Delay(W25QXX_CHIP_ERASE_TIME);
            w25qxx_Handle->status = W25QXX_STATUS_READY;
            break;

        case W25QXX_WAIT_BUSY:
            if (w25qxx_WaitWithTimeout(w25qxx_Handle, W25QXX_CHIP_ERASE_TIME) != W25QXX_ERROR_NONE)
            {
                w25qxx_Handle->error = W25QXX_ERROR_TIMEOUT;
                goto w25qxx_ErrorHandler;
            }
            break;

        default:
            {
                w25qxx_Handle->error = W25QXX_ERROR_ARGUMENT;
                goto w25qxx_ErrorHandler;
            }
            break;
        }
        break;

    default:
        {
            w25qxx_Handle->error = W25QXX_ERROR_INSTRUCTION;
            goto w25qxx_ErrorHandler;
        }
        break;
    }

w25qxx_ErrorHandler:
    if (w25qxx_Handle == NULL)
        return W25QXX_ERROR_REFERENCE_HANDLE;

    w25qxx_Handle->interface.CS_Set(W25QXX_CS_HIGH);
    return w25qxx_Handle->error;
}

w25qxx_Error_t w25qxx_BusyCheck(w25qxx_HandleTypeDef *w25qxx_Handle)
{
    /* Argument guards */
    if (w25qxx_Handle == NULL)
        goto w25qxx_ErrorHandler;

    /* Existing errors check */
    if (w25qxx_Handle->error != W25QXX_ERROR_NONE)
        goto w25qxx_ErrorHandler;

    /* Get busy bit state */
    if (w25qxx_ReadStatus(w25qxx_Handle, 1u) != W25QXX_ERROR_NONE)
    {
        w25qxx_Handle->error = W25QXX_ERROR_SPI;
        goto w25qxx_ErrorHandler;
    }

    /* Update device status according to the busy bit state */
    if (!READ_BIT(w25qxx_Handle->statusRegister, 1u << 0))
        w25qxx_Handle->status = W25QXX_STATUS_READY;

w25qxx_ErrorHandler:
    if (w25qxx_Handle == NULL)
        return W25QXX_ERROR_REFERENCE_HANDLE;

    return w25qxx_Handle->error;
}

w25qxx_Error_t w25qxx_WaitWithTimeout(w25qxx_HandleTypeDef *w25qxx_Handle, uint32_t timeout)
{
    uint32_t tickStart = uwTick;

    /* Argument guards */
    if (w25qxx_Handle == NULL)
        goto w25qxx_ErrorHandler;
    if (timeout == 0)
    {
        w25qxx_Handle->error = W25QXX_ERROR_ARGUMENT;
        goto w25qxx_ErrorHandler;
    }

    /* Existing errors check */
    if (w25qxx_Handle->error != W25QXX_ERROR_NONE)
        goto w25qxx_ErrorHandler;

    /* Command */
    w25qxx_Handle->CMD = W25QXX_CMD_READ_STATUS_REGISTER1;
    w25qxx_Handle->interface.CS_Set(W25QXX_CS_LOW);
    if (w25qxx_Handle->interface.Transmit(&w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT) !=
        SUCCESS)
    {
        w25qxx_Handle->error = W25QXX_ERROR_SPI;
        goto w25qxx_ErrorHandler;
    }

    /* Start polling */
    while (true)
    {
        /* Get status register 1 data */
        if (w25qxx_Handle->interface.Receive(&w25qxx_Handle->statusRegister, sizeof(w25qxx_Handle->statusRegister),
                                             W25QXX_RX_TIMEOUT) != SUCCESS)
        {
            w25qxx_Handle->error = W25QXX_ERROR_SPI;
            goto w25qxx_ErrorHandler;
        }

        /* Update device status according to the busy bit state */
        if (!READ_BIT(w25qxx_Handle->statusRegister, 1u << 0))
        {
            w25qxx_Handle->status = W25QXX_STATUS_READY;
            break;
        }

        /* Timeout check */
        if ((uwTick - tickStart) > timeout)
        {
            w25qxx_Handle->error = W25QXX_ERROR_TIMEOUT;
            goto w25qxx_ErrorHandler;
        }
    }

w25qxx_ErrorHandler:
    if (w25qxx_Handle == NULL)
        return W25QXX_ERROR_REFERENCE_HANDLE;

    w25qxx_Handle->interface.CS_Set(W25QXX_CS_HIGH);
    return w25qxx_Handle->error;
}

/**
 * @section Private functions
 */
static w25qxx_Error_t w25qxx_PowerDown(w25qxx_HandleTypeDef *w25qxx_Handle)
{
    /* Argument guards */
    if (w25qxx_Handle == NULL)
        goto w25qxx_ErrorHandler;

    /* Existing errors check */
    if (w25qxx_Handle->error != W25QXX_ERROR_NONE)
        goto w25qxx_ErrorHandler;

    /* Command */
    w25qxx_Handle->CMD = W25QXX_CMD_POWER_DOWN;
    w25qxx_Handle->interface.CS_Set(W25QXX_CS_LOW);
    if (w25qxx_Handle->interface.Transmit(&w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT) !=
        SUCCESS)
    {
        w25qxx_Handle->error = W25QXX_ERROR_SPI;
        goto w25qxx_ErrorHandler;
    }

w25qxx_ErrorHandler:
    if (w25qxx_Handle == NULL)
        return W25QXX_ERROR_REFERENCE_HANDLE;

    w25qxx_Handle->interface.CS_Set(W25QXX_CS_HIGH);
    w25qxx_Handle->interface.Delay(1);
    return w25qxx_Handle->error;
}

static w25qxx_Error_t w25qxx_ReleasePowerDown(w25qxx_HandleTypeDef *w25qxx_Handle)
{
    /* Argument guards */
    if (w25qxx_Handle == NULL)
        goto w25qxx_ErrorHandler;

    /* Existing errors check */
    if (w25qxx_Handle->error != W25QXX_ERROR_NONE)
        goto w25qxx_ErrorHandler;

    /* Command */
    w25qxx_Handle->CMD = W25QXX_CMD_RELEASE_POWER_DOWN;
    w25qxx_Handle->interface.CS_Set(W25QXX_CS_LOW);
    if (w25qxx_Handle->interface.Transmit(&w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT) !=
        SUCCESS)
    {
        w25qxx_Handle->error = W25QXX_ERROR_SPI;
        goto w25qxx_ErrorHandler;
    }

w25qxx_ErrorHandler:
    if (w25qxx_Handle == NULL)
        return W25QXX_ERROR_REFERENCE_HANDLE;

    w25qxx_Handle->interface.CS_Set(W25QXX_CS_HIGH);
    w25qxx_Handle->interface.Delay(1);
    return w25qxx_Handle->error;
}

static w25qxx_Error_t w25qxx_ResetDevice(w25qxx_HandleTypeDef *w25qxx_Handle)
{
    /* Argument guards */
    if (w25qxx_Handle == NULL)
        goto w25qxx_ErrorHandler;

    /* Existing errors check */
    if (w25qxx_Handle->error != W25QXX_ERROR_NONE)
        goto w25qxx_ErrorHandler;

    /* Command */
    w25qxx_Handle->CMD = W25QXX_CMD_ENABLE_RESET;
    w25qxx_Handle->interface.CS_Set(W25QXX_CS_LOW);
    if (w25qxx_Handle->interface.Transmit(&w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT) !=
        SUCCESS)
    {
        w25qxx_Handle->error = W25QXX_ERROR_SPI;
        goto w25qxx_ErrorHandler;
    }
    w25qxx_Handle->interface.CS_Set(W25QXX_CS_HIGH);
    w25qxx_Handle->CMD = W25QXX_CMD_RESET_DEVICE;
    w25qxx_Handle->interface.CS_Set(W25QXX_CS_LOW);
    if (w25qxx_Handle->interface.Transmit(&w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT) !=
        SUCCESS)
    {
        w25qxx_Handle->error = W25QXX_ERROR_SPI;
        goto w25qxx_ErrorHandler;
    }

w25qxx_ErrorHandler:
    if (w25qxx_Handle == NULL)
        return W25QXX_ERROR_REFERENCE_HANDLE;

    w25qxx_Handle->interface.CS_Set(W25QXX_CS_HIGH);
    w25qxx_Handle->interface.Delay(1);
    return w25qxx_Handle->error;
}

static w25qxx_Error_t w25qxx_ReadID(w25qxx_HandleTypeDef *w25qxx_Handle)
{
    /* Argument guards */
    if (w25qxx_Handle == NULL)
        goto w25qxx_ErrorHandler;

    /* Existing errors check */
    if (w25qxx_Handle->error != W25QXX_ERROR_NONE)
        goto w25qxx_ErrorHandler;

    /* Command */
    w25qxx_Handle->CMD = W25QXX_CMD_MANUFACTURER_DEVICE_ID;
    w25qxx_Handle->interface.CS_Set(W25QXX_CS_LOW);
    if (w25qxx_Handle->interface.Transmit(&w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT) !=
        SUCCESS)
    {
        w25qxx_Handle->error = W25QXX_ERROR_SPI;
        goto w25qxx_ErrorHandler;
    }

    /* 24-bit address (A23-A0) of 000000h */
    ADDRESS_BYTES_SWAP(w25qxx_Handle, (uint32_t) 0);
    if (w25qxx_Handle->interface.Transmit(w25qxx_Handle->addressBytes, sizeof(w25qxx_Handle->addressBytes),
                                          W25QXX_TX_TIMEOUT) != SUCCESS)
    {
        w25qxx_Handle->error = W25QXX_ERROR_SPI;
        goto w25qxx_ErrorHandler;
    }

    /* Get Manufacturer ID and Device ID */
    if (w25qxx_Handle->interface.Receive(w25qxx_Handle->ID, sizeof(w25qxx_Handle->ID), W25QXX_RX_TIMEOUT) != SUCCESS)
    {
        w25qxx_Handle->error = W25QXX_ERROR_SPI;
        goto w25qxx_ErrorHandler;
    }

w25qxx_ErrorHandler:
    if (w25qxx_Handle == NULL)
        return W25QXX_ERROR_REFERENCE_HANDLE;

    w25qxx_Handle->interface.CS_Set(W25QXX_CS_HIGH);
    return w25qxx_Handle->error;
}

static w25qxx_Error_t w25qxx_WriteStatus(w25qxx_HandleTypeDef *w25qxx_Handle, uint8_t statusRegisterx)
{
    /* Argument guards */
    if (w25qxx_Handle == NULL)
        goto w25qxx_ErrorHandler;
    if ((statusRegisterx < 1) || (statusRegisterx > 3))
    {
        w25qxx_Handle->error = W25QXX_ERROR_ARGUMENT;
        goto w25qxx_ErrorHandler;
    }

    /* Existing errors check */
    if (w25qxx_Handle->error != W25QXX_ERROR_NONE)
        goto w25qxx_ErrorHandler;

    /* Write Enable for Volatile Status Register */
    w25qxx_Handle->CMD = W25QXX_CMD_VOLATILE_SR_WRITE_ENABLE;
    w25qxx_Handle->interface.CS_Set(W25QXX_CS_LOW);
    if (w25qxx_Handle->interface.Transmit(&w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT) !=
        SUCCESS)
    {
        w25qxx_Handle->error = W25QXX_ERROR_SPI;
        goto w25qxx_ErrorHandler;
    }
    w25qxx_Handle->interface.CS_Set(W25QXX_CS_HIGH);

    /* Command */
    switch (statusRegisterx)
    {
    case 1:
        w25qxx_Handle->CMD = W25QXX_CMD_WRITE_STATUS_REGISTER1;
        break;

    case 2:
        w25qxx_Handle->CMD = W25QXX_CMD_WRITE_STATUS_REGISTER2;
        break;

    case 3:
        w25qxx_Handle->CMD = W25QXX_CMD_WRITE_STATUS_REGISTER3;
        break;
    }
    w25qxx_Handle->interface.CS_Set(W25QXX_CS_LOW);
    if (w25qxx_Handle->interface.Transmit(&w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT) !=
        SUCCESS)
    {
        w25qxx_Handle->error = W25QXX_ERROR_SPI;
        goto w25qxx_ErrorHandler;
    }

    /* Status write */
    if (w25qxx_Handle->interface.Transmit(&w25qxx_Handle->statusRegister, sizeof(w25qxx_Handle->statusRegister),
                                          W25QXX_TX_TIMEOUT) != SUCCESS)
    {
        w25qxx_Handle->error = W25QXX_ERROR_SPI;
        goto w25qxx_ErrorHandler;
    }

w25qxx_ErrorHandler:
    if (w25qxx_Handle == NULL)
        return W25QXX_ERROR_REFERENCE_HANDLE;

    w25qxx_Handle->interface.CS_Set(W25QXX_CS_HIGH);
    return w25qxx_Handle->error;
}

static w25qxx_Error_t w25qxx_ReadStatus(w25qxx_HandleTypeDef *w25qxx_Handle, uint8_t statusRegisterx)
{
    /* Argument guards */
    if (w25qxx_Handle == NULL)
        goto w25qxx_ErrorHandler;
    if ((statusRegisterx < 1) || (statusRegisterx > 3))
    {
        w25qxx_Handle->error = W25QXX_ERROR_ARGUMENT;
        goto w25qxx_ErrorHandler;
    }

    /* Existing errors check */
    if (w25qxx_Handle->error != W25QXX_ERROR_NONE)
        goto w25qxx_ErrorHandler;

    /* Command */
    switch (statusRegisterx)
    {
    case 1:
        w25qxx_Handle->CMD = W25QXX_CMD_READ_STATUS_REGISTER1;
        break;

    case 2:
        w25qxx_Handle->CMD = W25QXX_CMD_READ_STATUS_REGISTER2;
        break;

    case 3:
        w25qxx_Handle->CMD = W25QXX_CMD_READ_STATUS_REGISTER3;
        break;
    }
    w25qxx_Handle->interface.CS_Set(W25QXX_CS_LOW);
    if (w25qxx_Handle->interface.Transmit(&w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT) !=
        SUCCESS)
    {
        w25qxx_Handle->error = W25QXX_ERROR_SPI;
        goto w25qxx_ErrorHandler;
    }

    /* Status read */
    if (w25qxx_Handle->interface.Receive(&w25qxx_Handle->statusRegister, sizeof(w25qxx_Handle->statusRegister),
                                         W25QXX_RX_TIMEOUT) != SUCCESS)
    {
        w25qxx_Handle->error = W25QXX_ERROR_SPI;
        goto w25qxx_ErrorHandler;
    }

w25qxx_ErrorHandler:
    if (w25qxx_Handle == NULL)
        return W25QXX_ERROR_REFERENCE_HANDLE;

    w25qxx_Handle->interface.CS_Set(W25QXX_CS_HIGH);
    return w25qxx_Handle->error;
}

static w25qxx_Error_t w25qxx_WriteEnable(w25qxx_HandleTypeDef *w25qxx_Handle)
{
    /* Argument guards */
    if (w25qxx_Handle == NULL)
        goto w25qxx_ErrorHandler;

    /* Existing errors check */
    if (w25qxx_Handle->error != W25QXX_ERROR_NONE)
        goto w25qxx_ErrorHandler;

    /* Command */
    w25qxx_Handle->CMD = W25QXX_CMD_WRITE_ENABLE;
    w25qxx_Handle->interface.CS_Set(W25QXX_CS_LOW);
    if (w25qxx_Handle->interface.Transmit(&w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT) !=
        SUCCESS)
    {
        w25qxx_Handle->error = W25QXX_ERROR_SPI;
        goto w25qxx_ErrorHandler;
    }

w25qxx_ErrorHandler:
    if (w25qxx_Handle == NULL)
        return W25QXX_ERROR_REFERENCE_HANDLE;

    w25qxx_Handle->interface.CS_Set(W25QXX_CS_HIGH);
    return w25qxx_Handle->error;
}

static w25qxx_Error_t w25qxx_WriteDisable(w25qxx_HandleTypeDef *w25qxx_Handle)
{
    /* Argument guards */
    if (w25qxx_Handle == NULL)
        goto w25qxx_ErrorHandler;

    /* Existing errors check */
    if (w25qxx_Handle->error != W25QXX_ERROR_NONE)
        goto w25qxx_ErrorHandler;

    /* Command */
    w25qxx_Handle->CMD = W25QXX_CMD_WRITE_DISABLE;
    w25qxx_Handle->interface.CS_Set(W25QXX_CS_LOW);
    if (w25qxx_Handle->interface.Transmit(&w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT) !=
        SUCCESS)
    {
        w25qxx_Handle->error = W25QXX_ERROR_SPI;
        goto w25qxx_ErrorHandler;
    }

w25qxx_ErrorHandler:
    if (w25qxx_Handle == NULL)
        return W25QXX_ERROR_REFERENCE_HANDLE;

    w25qxx_Handle->interface.CS_Set(W25QXX_CS_HIGH);
    return w25qxx_Handle->error;
}

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