#include "HardwareSerial.h"
#include "w25qxx_Interface.h"

w25qxx_Transfer_Status_t w25qxx_SPI1_Receive(uint8_t *pDataRx, uint16_t size, uint32_t timeout)
{
    if (pDataRx == NULL)
        return W25QXX_TRANSFER_ERROR;
    if (size == 0u)
        return W25QXX_TRANSFER_ERROR;

    SPI.transfer(pDataRx, size);

    return W25QXX_TRANSFER_SUCCESS;
}

w25qxx_Transfer_Status_t w25qxx_SPI1_Transmit(uint8_t *pDataTx, uint16_t size, uint32_t timeout)
{
    if (pDataTx == NULL)
        return W25QXX_TRANSFER_ERROR;
    if (size == 0u)
        return W25QXX_TRANSFER_ERROR;

    /* Workaround */
    /* In case of buffer transfers the received data is stored in the buffer in-place
     * (the old data is replaced with the data received) */
    while (size--)
        SPI.transfer(*pDataTx++);

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
    extern volatile uint32_t uwTick;
    uint32_t tickStart = uwTick;

    /* Add a freq to guarantee minimum wait */
    if (ms < W25QXX_MIN_DELAY)
        ++ms;

    while ((uwTick - tickStart) < ms) {}
}

void w25qxx_Print(const uint8_t *message)
{
    Serial.print((const char *)message);
}