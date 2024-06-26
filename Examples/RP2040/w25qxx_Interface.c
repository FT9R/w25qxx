#include "w25qxx_Interface.h"

w25qxx_Transfer_Status_t w25qxx_SPI1_Receive(uint8_t *pDataRx, uint16_t size, uint32_t timeout)
{
    if (pDataRx == NULL)
        return W25QXX_TRANSFER_ERROR;
    if (size == 0u)
        return W25QXX_TRANSFER_ERROR;

    spi_read_blocking(spi_default, 0x00, pDataRx, size);

    return W25QXX_TRANSFER_SUCCESS;
}

w25qxx_Transfer_Status_t w25qxx_SPI1_Transmit(uint8_t *pDataTx, uint16_t size, uint32_t timeout)
{
    if (pDataTx == NULL)
        return W25QXX_TRANSFER_ERROR;
    if (size == 0u)
        return W25QXX_TRANSFER_ERROR;

    spi_write_blocking(spi_default, pDataTx, size);

    return W25QXX_TRANSFER_SUCCESS;
}

void w25qxx_SPI1_CS0_Set(w25qxx_CS_State_t newState)
{
    switch (newState)
    {
    case W25QXX_CS_HIGH:
        gpio_put(PICO_DEFAULT_SPI_CSN_PIN, true);
        break;

    case W25QXX_CS_LOW:
        gpio_put(PICO_DEFAULT_SPI_CSN_PIN, false);
        break;

    default:
        break;
    }
}

void w25qxx_Delay(uint32_t ms)
{
    extern volatile uint32_t uwTick;
    uint32_t tickStart = uwTick;

    /* Add a freq to guarantee minimum wait */
    if (ms < W25QXX_MIN_DELAY)
        ++ms;

    while ((uwTick - tickStart) < ms) {}
}

void w25qxx_Print(const uint8_t *message)
{
    printf("%s", message);
}