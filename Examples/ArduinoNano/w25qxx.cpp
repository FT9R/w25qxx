#include "w25qxx.h"

/* Private function prototypes */
static w25qxx_Error_t w25qxx_PowerDown(w25qxx_HandleTypeDef *w25qxx_Handle);
static w25qxx_Error_t w25qxx_ReleasePowerDown(w25qxx_HandleTypeDef *w25qxx_Handle);
static w25qxx_Error_t w25qxx_ResetDevice(w25qxx_HandleTypeDef *w25qxx_Handle);
static w25qxx_Error_t w25qxx_ReadID(w25qxx_HandleTypeDef *w25qxx_Handle);
static w25qxx_Error_t w25qxx_WriteEnable(w25qxx_HandleTypeDef *w25qxx_Handle);
static w25qxx_Error_t w25qxx_WriteDisable(w25qxx_HandleTypeDef *w25qxx_Handle);
static w25qxx_Error_t w25qxx_StatusUpdate(w25qxx_HandleTypeDef *w25qxx_Handle, w25qxx_Status_t statusCheck,
                                          w25qxx_Status_t statusSet);
static uint16_t ModBus_CRC(const uint8_t *pBuffer, uint16_t bufSize);

w25qxx_Error_t w25qxx_Link(w25qxx_HandleTypeDef *w25qxx_Handle,
                           w25qxx_Transfer_Status_t (*fpReceive)(uint8_t *, uint16_t, uint32_t),
                           w25qxx_Transfer_Status_t (*fpTransmit)(uint8_t *, uint16_t, uint32_t),
                           void (*fpCS_Set)(w25qxx_CS_State_t))
{
    /* Avoid dereferencing the null handle */
    if (w25qxx_Handle == NULL)
        return W25QXX_ERROR_ARGUMENT;

    if (w25qxx_StatusUpdate(w25qxx_Handle, W25QXX_STATUS_NOLINK, W25QXX_STATUS_LINK) != W25QXX_ERROR_NONE)
        W25QXX_ERROR_SET(w25qxx_Handle->error);

    /* Argument guards */
    if (fpReceive == NULL)
        W25QXX_ERROR_SET(W25QXX_ERROR_ARGUMENT);
    if (fpTransmit == NULL)
        W25QXX_ERROR_SET(W25QXX_ERROR_ARGUMENT);
    if (fpCS_Set == NULL)
        W25QXX_ERROR_SET(W25QXX_ERROR_ARGUMENT);

    /* Set up handle fields to its default state */
    memset(w25qxx_Handle, 0, sizeof(w25qxx_HandleTypeDef));

    /* Link functions within handle with the user defined ones */
    w25qxx_Handle->interface.Receive = fpReceive;
    w25qxx_Handle->interface.Transmit = fpTransmit;
    w25qxx_Handle->interface.CS_Set = fpCS_Set;
    w25qxx_Handle->interface.Delay = w25qxx_Delay;

    return w25qxx_StatusUpdate(w25qxx_Handle, W25QXX_STATUS_LINK, W25QXX_STATUS_RESET);
}

w25qxx_Error_t w25qxx_Init(w25qxx_HandleTypeDef *w25qxx_Handle)
{
    /* Avoid dereferencing the null handle */
    if (w25qxx_Handle == NULL)
        return W25QXX_ERROR_ARGUMENT;

    if (w25qxx_StatusUpdate(w25qxx_Handle, W25QXX_STATUS_RESET, W25QXX_STATUS_INIT) != W25QXX_ERROR_NONE)
        W25QXX_ERROR_SET(w25qxx_Handle->error);

    /* Start operation */
    w25qxx_Handle->interface.CS_Set(W25QXX_CS_HIGH);
    w25qxx_Handle->interface.Delay(100);

    w25qxx_ReleasePowerDown(w25qxx_Handle);
    W25QXX_ERROR_CHECK;

    w25qxx_ResetDevice(w25qxx_Handle);
    W25QXX_ERROR_CHECK;

    /* Get the Manufacturer ID and Device ID */
    w25qxx_ReadID(w25qxx_Handle);
    W25QXX_ERROR_CHECK;
    if (w25qxx_Handle->ID[0] != W25QXX_MANUFACTURER_ID)
        W25QXX_ERROR_SET(W25QXX_ERROR_INITIALIZATION);

    /* Determine number of pages */
    switch (w25qxx_Handle->ID[1])
    {
    case W25Q80:
        w25qxx_Handle->numberOfPages = 8 * (W25QXX_KB_TO_BYTE(1) * W25QXX_KB_TO_BYTE(1) / W25QXX_PAGE_SIZE / 8);
        break;

    case W25Q16:
        w25qxx_Handle->numberOfPages = 16 * (W25QXX_KB_TO_BYTE(1) * W25QXX_KB_TO_BYTE(1) / W25QXX_PAGE_SIZE / 8);
        break;

    case W25Q32:
        w25qxx_Handle->numberOfPages = 32 * (W25QXX_KB_TO_BYTE(1) * W25QXX_KB_TO_BYTE(1) / W25QXX_PAGE_SIZE / 8);
        break;

    case W25Q64:
        w25qxx_Handle->numberOfPages = 64 * (W25QXX_KB_TO_BYTE(1) * W25QXX_KB_TO_BYTE(1) / W25QXX_PAGE_SIZE / 8);
        break;

    case W25Q128:
        w25qxx_Handle->numberOfPages = 128 * (W25QXX_KB_TO_BYTE(1) * W25QXX_KB_TO_BYTE(1) / W25QXX_PAGE_SIZE / 8);
        break;

    /* Unsupported device */
    default:
        W25QXX_ERROR_SET(W25QXX_ERROR_INITIALIZATION);
    }

    /* Operation succeed */
    w25qxx_Handle->interface.Delay(10);
    return w25qxx_StatusUpdate(w25qxx_Handle, W25QXX_STATUS_INIT, W25QXX_STATUS_READY);
}

w25qxx_Error_t w25qxx_Write(w25qxx_HandleTypeDef *w25qxx_Handle, const uint8_t *buf, uint16_t dataLength,
                            uint32_t address, w25qxx_CRC_t trailingCRC, w25qxx_WaitForTask_t waitForTask)
{
    /* Avoid dereferencing the null handle */
    if (w25qxx_Handle == NULL)
        return W25QXX_ERROR_ARGUMENT;

    if (w25qxx_StatusUpdate(w25qxx_Handle, W25QXX_STATUS_READY, W25QXX_STATUS_WRITE) != W25QXX_ERROR_NONE)
        W25QXX_ERROR_SET(w25qxx_Handle->error);

    /* Argument guards */
    if (buf == NULL)
        W25QXX_ERROR_SET(W25QXX_ERROR_ARGUMENT);
    if (dataLength == 0)
        W25QXX_ERROR_SET(W25QXX_ERROR_ARGUMENT);
    w25qxx_Handle->frameLength = dataLength;
    if (trailingCRC == W25QXX_CRC)
        w25qxx_Handle->frameLength += sizeof(w25qxx_Handle->CRC16);
    if (w25qxx_Handle->frameLength > W25QXX_PAGE_SIZE)
        W25QXX_ERROR_SET(W25QXX_ERROR_ARGUMENT);
    if ((address % W25QXX_PAGE_SIZE) != 0)
        W25QXX_ERROR_SET(W25QXX_ERROR_ADDRESS);
    if (address > (W25QXX_PAGE_SIZE * (w25qxx_Handle->numberOfPages - 1)))
        W25QXX_ERROR_SET(W25QXX_ERROR_ADDRESS);

    /* Checksum calculate */
    if (trailingCRC == W25QXX_CRC)
        w25qxx_Handle->CRC16 = ModBus_CRC(buf, dataLength);

    /* Command */
    w25qxx_WriteEnable(w25qxx_Handle);
    W25QXX_ERROR_CHECK;
    w25qxx_Handle->CMD = W25QXX_CMD_PAGE_PROGRAM;
    w25qxx_Handle->interface.CS_Set(W25QXX_CS_LOW);
    W25QXX_BEGIN_TRASMIT(&w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);

    /* A23-A0 - Start address of the desired page */
    W25QXX_ADDRESS_BYTES_SWAP(address);
    W25QXX_BEGIN_TRASMIT(w25qxx_Handle->addressBytes, sizeof(w25qxx_Handle->addressBytes), W25QXX_TX_TIMEOUT);

    /* Data */
    W25QXX_BEGIN_TRASMIT((uint8_t *) buf, dataLength, W25QXX_TX_TIMEOUT);

    /* Checksum */
    if (trailingCRC == W25QXX_CRC)
        W25QXX_BEGIN_TRASMIT((uint8_t *) &w25qxx_Handle->CRC16, sizeof(w25qxx_Handle->CRC16), W25QXX_TX_TIMEOUT);
    w25qxx_Handle->interface.CS_Set(W25QXX_CS_HIGH);

    /* Task wait */
    switch (waitForTask)
    {
    case W25QXX_WAIT_NO:
        break;

    case W25QXX_WAIT_DELAY:
        w25qxx_Handle->interface.Delay(W25QXX_PAGE_PROGRAM_TIME);
        break;

    case W25QXX_WAIT_BUSY:
        if (w25qxx_WaitWithTimeout(w25qxx_Handle, W25QXX_PAGE_PROGRAM_TIME) != W25QXX_STATUS_READY)
            W25QXX_ERROR_SET(W25QXX_ERROR_TIMEOUT);
        break;

    default:
        W25QXX_ERROR_SET(W25QXX_ERROR_ARGUMENT);
        break;
    }

    return w25qxx_StatusUpdate(w25qxx_Handle, W25QXX_STATUS_WRITE, W25QXX_STATUS_READY);
}

w25qxx_Error_t w25qxx_Read(w25qxx_HandleTypeDef *w25qxx_Handle, uint8_t *buf, uint16_t dataLength, uint32_t address,
                           w25qxx_CRC_t trailingCRC, w25qxx_FastRead_t fastRead)
{
    /* Avoid dereferencing the null handle */
    if (w25qxx_Handle == NULL)
        return W25QXX_ERROR_ARGUMENT;

    if (w25qxx_StatusUpdate(w25qxx_Handle, W25QXX_STATUS_READY, W25QXX_STATUS_READ) != W25QXX_ERROR_NONE)
        W25QXX_ERROR_SET(w25qxx_Handle->error);

    /* Argument guards */
    if (buf == NULL)
        W25QXX_ERROR_SET(W25QXX_ERROR_ARGUMENT);
    if (dataLength == 0)
        W25QXX_ERROR_SET(W25QXX_ERROR_ARGUMENT);
    w25qxx_Handle->frameLength = dataLength;
    if (trailingCRC == W25QXX_CRC)
        w25qxx_Handle->frameLength += sizeof(w25qxx_Handle->CRC16);
    if (w25qxx_Handle->frameLength > W25QXX_PAGE_SIZE)
        W25QXX_ERROR_SET(W25QXX_ERROR_ARGUMENT);
    if ((address % W25QXX_PAGE_SIZE) != 0)
        W25QXX_ERROR_SET(W25QXX_ERROR_ADDRESS);
    if (address > (W25QXX_PAGE_SIZE * (w25qxx_Handle->numberOfPages - 1)))
        W25QXX_ERROR_SET(W25QXX_ERROR_ADDRESS);

    /* Command */
    w25qxx_Handle->CMD = (fastRead == W25QXX_FASTREAD) ? W25QXX_CMD_FAST_READ : W25QXX_CMD_READ_DATA;
    w25qxx_Handle->interface.CS_Set(W25QXX_CS_LOW);
    W25QXX_BEGIN_TRASMIT(&w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);

    /* A23-A0 - Start address of the desired page */
    W25QXX_ADDRESS_BYTES_SWAP(address);
    W25QXX_BEGIN_TRASMIT(w25qxx_Handle->addressBytes, sizeof(w25qxx_Handle->addressBytes), W25QXX_TX_TIMEOUT);

    /* 8 dummy clocks */
    if (fastRead == W25QXX_FASTREAD)
        W25QXX_BEGIN_TRASMIT(&w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);

    /* Data receive */
    W25QXX_BEGIN_RECEIVE(w25qxx_Handle->frameBuf, w25qxx_Handle->frameLength, W25QXX_RX_TIMEOUT);
    w25qxx_Handle->interface.CS_Set(W25QXX_CS_HIGH);

    /* Checksum compare */
    if (trailingCRC == W25QXX_CRC)
    {
        w25qxx_Handle->CRC16 = ModBus_CRC(w25qxx_Handle->frameBuf, dataLength);
        if (memcmp(&w25qxx_Handle->frameBuf[dataLength], &w25qxx_Handle->CRC16, sizeof(w25qxx_Handle->CRC16)) != 0)
            W25QXX_ERROR_SET(W25QXX_ERROR_CHECKSUM);
    }

    /* Copy received data without checksum to the destination buffer */
    memcpy(buf, w25qxx_Handle->frameBuf, dataLength);

    return w25qxx_StatusUpdate(w25qxx_Handle, W25QXX_STATUS_READ, W25QXX_STATUS_READY);
}

w25qxx_Error_t w25qxx_Erase(w25qxx_HandleTypeDef *w25qxx_Handle, w25qxx_EraseInstruction_t eraseInstruction,
                            uint32_t address, w25qxx_WaitForTask_t waitForTask)
{
    uint32_t chipEraseTimeout = 0;

    /* Avoid dereferencing the null handle */
    if (w25qxx_Handle == NULL)
        return W25QXX_ERROR_ARGUMENT;

    if (w25qxx_StatusUpdate(w25qxx_Handle, W25QXX_STATUS_READY, W25QXX_STATUS_ERASE) != W25QXX_ERROR_NONE)
        W25QXX_ERROR_SET(w25qxx_Handle->error);

    switch (eraseInstruction)
    {
    case W25QXX_SECTOR_ERASE_4KB:
        if ((address % W25QXX_SECTOR_SIZE_4KB) != 0)
            W25QXX_ERROR_SET(W25QXX_ERROR_ADDRESS);
        if (address > ((W25QXX_PAGE_SIZE * w25qxx_Handle->numberOfPages) - W25QXX_SECTOR_SIZE_4KB))
            W25QXX_ERROR_SET(W25QXX_ERROR_ADDRESS);

        /* Command */
        w25qxx_WriteEnable(w25qxx_Handle);
        W25QXX_ERROR_CHECK;
        w25qxx_Handle->CMD = W25QXX_CMD_SECTOR_ERASE_4KB;
        w25qxx_Handle->interface.CS_Set(W25QXX_CS_LOW);
        W25QXX_BEGIN_TRASMIT(&w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);

        /* A23-A0 - Start address of the desired page */
        W25QXX_ADDRESS_BYTES_SWAP(address);
        W25QXX_BEGIN_TRASMIT(w25qxx_Handle->addressBytes, sizeof(w25qxx_Handle->addressBytes), W25QXX_TX_TIMEOUT);
        w25qxx_Handle->interface.CS_Set(W25QXX_CS_HIGH);

        /* Task wait */
        switch (waitForTask)
        {
        case W25QXX_WAIT_NO:
            break;

        case W25QXX_WAIT_DELAY:
            w25qxx_Handle->interface.Delay(W25QXX_SECTOR_ERASE_TIME_4KB);
            break;

        case W25QXX_WAIT_BUSY:
            if (w25qxx_WaitWithTimeout(w25qxx_Handle, W25QXX_SECTOR_ERASE_TIME_4KB) != W25QXX_STATUS_READY)
                W25QXX_ERROR_SET(W25QXX_ERROR_TIMEOUT);
            break;

        default:
            W25QXX_ERROR_SET(W25QXX_ERROR_ARGUMENT);
            break;
        }
        break;

    case W25QXX_BLOCK_ERASE_32KB:
        if ((address % W25QXX_BLOCK_SIZE_32KB) != 0)
            W25QXX_ERROR_SET(W25QXX_ERROR_ADDRESS);
        if (address > ((W25QXX_PAGE_SIZE * w25qxx_Handle->numberOfPages) - W25QXX_BLOCK_SIZE_32KB))
            W25QXX_ERROR_SET(W25QXX_ERROR_ADDRESS);

        /* Command */
        w25qxx_WriteEnable(w25qxx_Handle);
        W25QXX_ERROR_CHECK;
        w25qxx_Handle->CMD = W25QXX_CMD_BLOCK_ERASE_32KB;
        w25qxx_Handle->interface.CS_Set(W25QXX_CS_LOW);
        W25QXX_BEGIN_TRASMIT(&w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);

        /* A23-A0 - Start address of the desired page */
        W25QXX_ADDRESS_BYTES_SWAP(address);
        W25QXX_BEGIN_TRASMIT(w25qxx_Handle->addressBytes, sizeof(w25qxx_Handle->addressBytes), W25QXX_TX_TIMEOUT);
        w25qxx_Handle->interface.CS_Set(W25QXX_CS_HIGH);

        /* Task wait */
        switch (waitForTask)
        {
        case W25QXX_WAIT_NO:
            break;

        case W25QXX_WAIT_DELAY:
            w25qxx_Handle->interface.Delay(W25QXX_BLOCK_ERASE_TIME_32KB);
            break;

        case W25QXX_WAIT_BUSY:
            if (w25qxx_WaitWithTimeout(w25qxx_Handle, W25QXX_BLOCK_ERASE_TIME_32KB) != W25QXX_STATUS_READY)
                W25QXX_ERROR_SET(W25QXX_ERROR_TIMEOUT);
            break;

        default:
            W25QXX_ERROR_SET(W25QXX_ERROR_ARGUMENT);
            break;
        }
        break;

    case W25QXX_BLOCK_ERASE_64KB:
        if ((address % W25QXX_BLOCK_SIZE_64KB) != 0)
            W25QXX_ERROR_SET(W25QXX_ERROR_ADDRESS);
        if (address > ((W25QXX_PAGE_SIZE * w25qxx_Handle->numberOfPages) - W25QXX_BLOCK_SIZE_64KB))
            W25QXX_ERROR_SET(W25QXX_ERROR_ADDRESS);

        /* Command */
        w25qxx_WriteEnable(w25qxx_Handle);
        W25QXX_ERROR_CHECK;
        w25qxx_Handle->CMD = W25QXX_CMD_BLOCK_ERASE_64KB;
        w25qxx_Handle->interface.CS_Set(W25QXX_CS_LOW);
        W25QXX_BEGIN_TRASMIT(&w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);

        /* A23-A0 - Start address of the desired page */
        W25QXX_ADDRESS_BYTES_SWAP(address);
        W25QXX_BEGIN_TRASMIT(w25qxx_Handle->addressBytes, sizeof(w25qxx_Handle->addressBytes), W25QXX_TX_TIMEOUT);
        w25qxx_Handle->interface.CS_Set(W25QXX_CS_HIGH);

        /* Task wait */
        switch (waitForTask)
        {
        case W25QXX_WAIT_NO:
            break;

        case W25QXX_WAIT_DELAY:
            w25qxx_Handle->interface.Delay(W25QXX_BLOCK_ERASE_TIME_64KB);
            break;

        case W25QXX_WAIT_BUSY:
            if (w25qxx_WaitWithTimeout(w25qxx_Handle, W25QXX_BLOCK_ERASE_TIME_64KB) != W25QXX_STATUS_READY)
                W25QXX_ERROR_SET(W25QXX_ERROR_TIMEOUT);
            break;

        default:
            W25QXX_ERROR_SET(W25QXX_ERROR_ARGUMENT);
            break;
        }
        break;

    case W25QXX_CHIP_ERASE:
        if (address != 0)
            W25QXX_ERROR_SET(W25QXX_ERROR_ADDRESS);

        /* Choose the right timeout */
        switch (w25qxx_Handle->ID[1])
        {
        case W25Q80:
            chipEraseTimeout = CETIME_W25Q80;
            break;

        case W25Q16:
            chipEraseTimeout = CETIME_W25Q16;
            break;

        case W25Q32:
            chipEraseTimeout = CETIME_W25Q32;
            break;

        case W25Q64:
            chipEraseTimeout = CETIME_W25Q64;
            break;

        case W25Q128:
            chipEraseTimeout = CETIME_W25Q128;
            break;
        }

        /* Command */
        w25qxx_WriteEnable(w25qxx_Handle);
        W25QXX_ERROR_CHECK;
        w25qxx_Handle->CMD = W25QXX_CMD_CHIP_ERASE;
        w25qxx_Handle->interface.CS_Set(W25QXX_CS_LOW);
        W25QXX_BEGIN_TRASMIT(&w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);
        w25qxx_Handle->interface.CS_Set(W25QXX_CS_HIGH);

        /* Task wait */
        switch (waitForTask)
        {
        case W25QXX_WAIT_NO:
            break;

        case W25QXX_WAIT_DELAY:
            w25qxx_Handle->interface.Delay(chipEraseTimeout);
            break;

        case W25QXX_WAIT_BUSY:
            if (w25qxx_WaitWithTimeout(w25qxx_Handle, chipEraseTimeout) != W25QXX_STATUS_READY)
                W25QXX_ERROR_SET(W25QXX_ERROR_TIMEOUT);
            break;

        default:
            W25QXX_ERROR_SET(W25QXX_ERROR_ARGUMENT);
            break;
        }
        break;

    default:
        W25QXX_ERROR_SET(W25QXX_ERROR_INSTRUCTION);
        break;
    }

    return w25qxx_StatusUpdate(w25qxx_Handle, W25QXX_STATUS_ERASE, W25QXX_STATUS_READY);
}

w25qxx_Error_t w25qxx_WriteStatus(w25qxx_HandleTypeDef *w25qxx_Handle, uint8_t statusRegisterx,
                                  w25qxx_SR_Behaviour_t statusRegisterBehaviour)
{
    /* Avoid dereferencing the null handle */
    if (w25qxx_Handle == NULL)
        return W25QXX_ERROR_ARGUMENT;

    if (w25qxx_StatusUpdate(w25qxx_Handle, W25QXX_STATUS_READY, W25QXX_STATUS_WRITE) != W25QXX_ERROR_NONE)
        W25QXX_ERROR_SET(w25qxx_Handle->error);

    /* Argument guards */
    if ((statusRegisterx < 1u) || (statusRegisterx > 3u))
        W25QXX_ERROR_SET(W25QXX_ERROR_ARGUMENT);

    /* Command 1 */
    w25qxx_Handle->CMD =
        (statusRegisterBehaviour == W25QXX_SR_VOLATILE) ? W25QXX_CMD_VOLATILE_SR_WRITE_ENABLE : W25QXX_CMD_WRITE_ENABLE;
    w25qxx_Handle->interface.CS_Set(W25QXX_CS_LOW);
    W25QXX_BEGIN_TRASMIT(&w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);
    w25qxx_Handle->interface.CS_Set(W25QXX_CS_HIGH);

    /* Command 2 */
    switch (statusRegisterx)
    {
    case 1u:
        w25qxx_Handle->CMD = W25QXX_CMD_WRITE_STATUS_REGISTER1;
        break;

    case 2u:
        w25qxx_Handle->CMD = W25QXX_CMD_WRITE_STATUS_REGISTER2;
        break;

    case 3u:
        w25qxx_Handle->CMD = W25QXX_CMD_WRITE_STATUS_REGISTER3;
        break;
    }
    w25qxx_Handle->interface.CS_Set(W25QXX_CS_LOW);
    W25QXX_BEGIN_TRASMIT(&w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);

    /* Status write */
    W25QXX_BEGIN_TRASMIT(&w25qxx_Handle->statusRegister, sizeof(w25qxx_Handle->statusRegister), W25QXX_TX_TIMEOUT);
    w25qxx_Handle->interface.CS_Set(W25QXX_CS_HIGH);

    /* Task wait */
    if (statusRegisterBehaviour != W25QXX_SR_VOLATILE)
    {
        if (w25qxx_WaitWithTimeout(w25qxx_Handle, W25QXX_WRITE_STATUS_REGISTER_TIME) != W25QXX_STATUS_READY)
            W25QXX_ERROR_SET(W25QXX_ERROR_TIMEOUT);
    }

    return w25qxx_StatusUpdate(w25qxx_Handle, W25QXX_STATUS_WRITE, W25QXX_STATUS_READY);
}

w25qxx_Error_t w25qxx_ReadStatus(w25qxx_HandleTypeDef *w25qxx_Handle, uint8_t statusRegisterx)
{
    /* Avoid dereferencing the null handle */
    if (w25qxx_Handle == NULL)
        return W25QXX_ERROR_ARGUMENT;

    if (w25qxx_StatusUpdate(w25qxx_Handle, W25QXX_STATUS_READY, W25QXX_STATUS_READ) != W25QXX_ERROR_NONE)
        W25QXX_ERROR_SET(w25qxx_Handle->error);

    /* Argument guards */
    if ((statusRegisterx < 1u) || (statusRegisterx > 3u))
        W25QXX_ERROR_SET(W25QXX_ERROR_ARGUMENT);

    /* Command */
    switch (statusRegisterx)
    {
    case 1u:
        w25qxx_Handle->CMD = W25QXX_CMD_READ_STATUS_REGISTER1;
        break;

    case 2u:
        w25qxx_Handle->CMD = W25QXX_CMD_READ_STATUS_REGISTER2;
        break;

    case 3u:
        w25qxx_Handle->CMD = W25QXX_CMD_READ_STATUS_REGISTER3;
        break;
    }
    w25qxx_Handle->interface.CS_Set(W25QXX_CS_LOW);
    W25QXX_BEGIN_TRASMIT(&w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);

    /* Status read */
    W25QXX_BEGIN_RECEIVE(&w25qxx_Handle->statusRegister, sizeof(w25qxx_Handle->statusRegister), W25QXX_RX_TIMEOUT);
    w25qxx_Handle->interface.CS_Set(W25QXX_CS_HIGH);

    return w25qxx_StatusUpdate(w25qxx_Handle, W25QXX_STATUS_READ, W25QXX_STATUS_READY);
}

w25qxx_Error_t w25qxx_ResetError(w25qxx_HandleTypeDef *w25qxx_Handle)
{
    /* Avoid dereferencing the null handle */
    if (w25qxx_Handle == NULL)
        return W25QXX_ERROR_ARGUMENT;

    /* Existing errors check */
    if (w25qxx_Handle->error == W25QXX_ERROR_NONE)
        return w25qxx_Handle->error;

    /* Reset error */
    w25qxx_Handle->error = W25QXX_ERROR_NONE;

    /* Try to get response from device */
    if (w25qxx_WaitWithTimeout(w25qxx_Handle, W25QXX_RESPONSE_TIMEOUT) != W25QXX_STATUS_READY)
        W25QXX_ERROR_SET(W25QXX_ERROR_TIMEOUT);

    return w25qxx_StatusUpdate(w25qxx_Handle, w25qxx_Handle->status, W25QXX_STATUS_READY);
}

w25qxx_Status_t w25qxx_BusyCheck(w25qxx_HandleTypeDef *w25qxx_Handle)
{
    /* Avoid dereferencing the null handle */
    if (w25qxx_Handle == NULL)
        return W25QXX_STATUS_UNDEFINED;

    /* Existing errors check */
    if (w25qxx_Handle->error != W25QXX_ERROR_NONE)
        return W25QXX_STATUS_UNDEFINED;

    /* Get busy bit state */
    if (w25qxx_ReadStatus(w25qxx_Handle, 1u) != W25QXX_ERROR_NONE)
        return W25QXX_STATUS_UNDEFINED;

    /* Return device busy status */
    return READ_BIT(w25qxx_Handle->statusRegister, 1u << 0) ? W25QXX_STATUS_UNDEFINED : W25QXX_STATUS_READY;
}

w25qxx_Status_t w25qxx_WaitWithTimeout(w25qxx_HandleTypeDef *w25qxx_Handle, uint32_t timeout)
{
    extern volatile uint32_t uwTick;
    uint32_t tickStart = uwTick;

    /* Avoid dereferencing the null handle */
    if (w25qxx_Handle == NULL)
        return W25QXX_STATUS_UNDEFINED;

    /* Existing errors check */
    if (w25qxx_Handle->error != W25QXX_ERROR_NONE)
        return W25QXX_STATUS_UNDEFINED;

    /* Argument guards */
    if (timeout == 0)
        return W25QXX_STATUS_UNDEFINED;

    /* Command */
    w25qxx_Handle->CMD = W25QXX_CMD_READ_STATUS_REGISTER1;
    w25qxx_Handle->interface.CS_Set(W25QXX_CS_LOW);
    if (w25qxx_Handle->interface.Transmit(&w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT) !=
        W25QXX_TRANSFER_SUCCESS)
    {
        w25qxx_Handle->error = W25QXX_ERROR_SPI;
        w25qxx_Handle->interface.CS_Set(W25QXX_CS_HIGH);

        return W25QXX_STATUS_UNDEFINED;
    }

    /* Start polling */
    while (true)
    {
        /* Get status register 1 data */
        if (w25qxx_Handle->interface.Receive(&w25qxx_Handle->statusRegister, sizeof(w25qxx_Handle->statusRegister),
                                             W25QXX_RX_TIMEOUT) != W25QXX_TRANSFER_SUCCESS)
        {
            w25qxx_Handle->error = W25QXX_ERROR_SPI;
            w25qxx_Handle->interface.CS_Set(W25QXX_CS_HIGH);

            return W25QXX_STATUS_UNDEFINED;
        }

        /* Get busy bit state */
        if (!READ_BIT(w25qxx_Handle->statusRegister, 1u << 0))
        {
            w25qxx_Handle->interface.CS_Set(W25QXX_CS_HIGH);

            return W25QXX_STATUS_READY;
        }

        /* Timeout check */
        if ((uwTick - tickStart) > timeout)
        {
            w25qxx_Handle->interface.CS_Set(W25QXX_CS_HIGH);

            return W25QXX_STATUS_UNDEFINED;
        }
    }
}

/**
 * @section Private functions
 */
static w25qxx_Error_t w25qxx_PowerDown(w25qxx_HandleTypeDef *w25qxx_Handle)
{
    /* Avoid dereferencing the null handle */
    if (w25qxx_Handle == NULL)
        return W25QXX_ERROR_ARGUMENT;

    /* Existing errors check */
    if (w25qxx_Handle->error != W25QXX_ERROR_NONE)
        return w25qxx_Handle->error;

    /* Command */
    w25qxx_Handle->CMD = W25QXX_CMD_POWER_DOWN;
    w25qxx_Handle->interface.CS_Set(W25QXX_CS_LOW);
    W25QXX_BEGIN_TRASMIT(&w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);
    w25qxx_Handle->interface.CS_Set(W25QXX_CS_HIGH);
    w25qxx_Handle->interface.Delay(1);

    return w25qxx_Handle->error;
}

static w25qxx_Error_t w25qxx_ReleasePowerDown(w25qxx_HandleTypeDef *w25qxx_Handle)
{
    /* Avoid dereferencing the null handle */
    if (w25qxx_Handle == NULL)
        return W25QXX_ERROR_ARGUMENT;

    /* Existing errors check */
    if (w25qxx_Handle->error != W25QXX_ERROR_NONE)
        return w25qxx_Handle->error;

    /* Command */
    w25qxx_Handle->CMD = W25QXX_CMD_RELEASE_POWER_DOWN;
    w25qxx_Handle->interface.CS_Set(W25QXX_CS_LOW);
    W25QXX_BEGIN_TRASMIT(&w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);
    w25qxx_Handle->interface.CS_Set(W25QXX_CS_HIGH);
    w25qxx_Handle->interface.Delay(1);

    return w25qxx_Handle->error;
}

static w25qxx_Error_t w25qxx_ResetDevice(w25qxx_HandleTypeDef *w25qxx_Handle)
{
    /* Avoid dereferencing the null handle */
    if (w25qxx_Handle == NULL)
        return W25QXX_ERROR_ARGUMENT;

    /* Existing errors check */
    if (w25qxx_Handle->error != W25QXX_ERROR_NONE)
        return w25qxx_Handle->error;

    /* Command 1 */
    w25qxx_Handle->CMD = W25QXX_CMD_ENABLE_RESET;
    w25qxx_Handle->interface.CS_Set(W25QXX_CS_LOW);
    W25QXX_BEGIN_TRASMIT(&w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);
    w25qxx_Handle->interface.CS_Set(W25QXX_CS_HIGH);

    /* Command 2 */
    w25qxx_Handle->CMD = W25QXX_CMD_RESET_DEVICE;
    w25qxx_Handle->interface.CS_Set(W25QXX_CS_LOW);
    W25QXX_BEGIN_TRASMIT(&w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);
    w25qxx_Handle->interface.CS_Set(W25QXX_CS_HIGH);
    w25qxx_Handle->interface.Delay(1);

    return w25qxx_Handle->error;
}

static w25qxx_Error_t w25qxx_ReadID(w25qxx_HandleTypeDef *w25qxx_Handle)
{
    /* Avoid dereferencing the null handle */
    if (w25qxx_Handle == NULL)
        return W25QXX_ERROR_ARGUMENT;

    /* Existing errors check */
    if (w25qxx_Handle->error != W25QXX_ERROR_NONE)
        return w25qxx_Handle->error;

    /* Command */
    w25qxx_Handle->CMD = W25QXX_CMD_MANUFACTURER_DEVICE_ID;
    w25qxx_Handle->interface.CS_Set(W25QXX_CS_LOW);
    W25QXX_BEGIN_TRASMIT(&w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);

    /* 24-bit address (A23-A0) of 000000h */
    W25QXX_ADDRESS_BYTES_SWAP((uint32_t) 0);
    W25QXX_BEGIN_TRASMIT(w25qxx_Handle->addressBytes, sizeof(w25qxx_Handle->addressBytes), W25QXX_TX_TIMEOUT);

    /* Get Manufacturer ID and Device ID */
    W25QXX_BEGIN_RECEIVE(w25qxx_Handle->ID, sizeof(w25qxx_Handle->ID), W25QXX_RX_TIMEOUT);
    w25qxx_Handle->interface.CS_Set(W25QXX_CS_HIGH);

    return w25qxx_Handle->error;
}

static w25qxx_Error_t w25qxx_WriteEnable(w25qxx_HandleTypeDef *w25qxx_Handle)
{
    /* Avoid dereferencing the null handle */
    if (w25qxx_Handle == NULL)
        return W25QXX_ERROR_ARGUMENT;

    /* Existing errors check */
    if (w25qxx_Handle->error != W25QXX_ERROR_NONE)
        return w25qxx_Handle->error;

    /* Command */
    w25qxx_Handle->CMD = W25QXX_CMD_WRITE_ENABLE;
    w25qxx_Handle->interface.CS_Set(W25QXX_CS_LOW);
    W25QXX_BEGIN_TRASMIT(&w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);
    w25qxx_Handle->interface.CS_Set(W25QXX_CS_HIGH);

    return w25qxx_Handle->error;
}

static w25qxx_Error_t w25qxx_WriteDisable(w25qxx_HandleTypeDef *w25qxx_Handle)
{
    /* Avoid dereferencing the null handle */
    if (w25qxx_Handle == NULL)
        return W25QXX_ERROR_ARGUMENT;

    /* Existing errors check */
    if (w25qxx_Handle->error != W25QXX_ERROR_NONE)
        return w25qxx_Handle->error;

    /* Command */
    w25qxx_Handle->CMD = W25QXX_CMD_WRITE_DISABLE;
    w25qxx_Handle->interface.CS_Set(W25QXX_CS_LOW);
    W25QXX_BEGIN_TRASMIT(&w25qxx_Handle->CMD, sizeof(w25qxx_Handle->CMD), W25QXX_TX_TIMEOUT);
    w25qxx_Handle->interface.CS_Set(W25QXX_CS_HIGH);

    return w25qxx_Handle->error;
}

static w25qxx_Error_t w25qxx_StatusUpdate(w25qxx_HandleTypeDef *w25qxx_Handle, w25qxx_Status_t statusCheck,
                                          w25qxx_Status_t statusSet)
{
    /* Existing errors check */
    if (w25qxx_Handle->error != W25QXX_ERROR_NONE)
        return w25qxx_Handle->error;

    /* Actual status check */
    if (w25qxx_Handle->status != statusCheck)
        W25QXX_ERROR_SET(W25QXX_ERROR_STATUS);

    /* Status update */
    w25qxx_Handle->status = statusSet;

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