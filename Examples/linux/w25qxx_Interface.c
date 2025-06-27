#include "w25qxx_Interface.h"

w25qxx_Transfer_Status_t w25qxx_SPI1_Receive(uint8_t *pDataRx, uint16_t size, uint32_t timeout)
{
    (void) timeout;

    if (pDataRx == NULL)
        return W25QXX_TRANSFER_ERROR;
    if (size == 0u)
        return W25QXX_TRANSFER_ERROR;

    if (wiringPiSPIDataRW(SPI_DEV, pDataRx, size) != size)
        return W25QXX_TRANSFER_ERROR;

    return W25QXX_TRANSFER_SUCCESS;
}

w25qxx_Transfer_Status_t w25qxx_SPI1_Transmit(uint8_t *pDataTx, uint16_t size, uint32_t timeout)
{
    (void) timeout;

    if (pDataTx == NULL)
        return W25QXX_TRANSFER_ERROR;
    if (size == 0u)
        return W25QXX_TRANSFER_ERROR;

    if (wiringPiSPIDataRW(SPI_DEV, pDataTx, size) != size)
        return W25QXX_TRANSFER_ERROR;

    return W25QXX_TRANSFER_SUCCESS;
}

void w25qxx_SPI1_CS0_Set(w25qxx_CS_State_t newState)
{
    switch (newState)
    {
    case W25QXX_CS_HIGH:
        digitalWrite(SPI1_CS0_PIN, HIGH);
        break;

    case W25QXX_CS_LOW:
        digitalWrite(SPI1_CS0_PIN, LOW);
        break;

    default:
        break;
    }
}

void w25qxx_Delay(uint32_t ms)
{
    usleep(ms * 1000);
}

void w25qxx_Print(char *message)
{
    printf("%s", message);
}