#include "w25qxx_Interface.h"

w25qxx_Transfer_Status_t w25qxx_SPI1_Receive(uint8_t *pDataRx, uint16_t size, uint32_t timeout)
{
    if (pDataRx == NULL)
        return W25QXX_TRANSFER_ERROR;
    if (size == 0u)
        return W25QXX_TRANSFER_ERROR;

    return (HAL_SPI_Receive(&hspi1, pDataRx, size, timeout) == HAL_OK) ? W25QXX_TRANSFER_SUCCESS
                                                                       : W25QXX_TRANSFER_ERROR;
}

w25qxx_Transfer_Status_t w25qxx_SPI1_Transmit(uint8_t *pDataTx, uint16_t size, uint32_t timeout)
{
    if (pDataTx == NULL)
        return W25QXX_TRANSFER_ERROR;
    if (size == 0u)
        return W25QXX_TRANSFER_ERROR;

    return (HAL_SPI_Transmit(&hspi1, pDataTx, size, timeout) == HAL_OK) ? W25QXX_TRANSFER_SUCCESS
                                                                        : W25QXX_TRANSFER_ERROR;
}

void w25qxx_SPI1_CS0_Set(w25qxx_CS_State_t newState)
{
    switch (newState)
    {
    case W25QXX_CS_HIGH:
        HAL_GPIO_WritePin(SPI1_CS0_PORT, SPI1_CS0_PIN, GPIO_PIN_SET);
        break;

    case W25QXX_CS_LOW:
        HAL_GPIO_WritePin(SPI1_CS0_PORT, SPI1_CS0_PIN, GPIO_PIN_RESET);
        break;

    default:
        break;
    }
}

void w25qxx_SPI1_CS1_Set(w25qxx_CS_State_t newState)
{
    switch (newState)
    {
    case W25QXX_CS_HIGH:
        HAL_GPIO_WritePin(SPI1_CS1_PORT, SPI1_CS1_PIN, GPIO_PIN_SET);
        break;

    case W25QXX_CS_LOW:
        HAL_GPIO_WritePin(SPI1_CS1_PORT, SPI1_CS1_PIN, GPIO_PIN_RESET);
        break;

    default:
        break;
    }
}

void w25qxx_SPI1_CS2_Set(w25qxx_CS_State_t newState)
{
    switch (newState)
    {
    case W25QXX_CS_HIGH:
        HAL_GPIO_WritePin(SPI1_CS2_PORT, SPI1_CS2_PIN, GPIO_PIN_SET);
        break;

    case W25QXX_CS_LOW:
        HAL_GPIO_WritePin(SPI1_CS2_PORT, SPI1_CS2_PIN, GPIO_PIN_RESET);
        break;

    default:
        break;
    }
}

void w25qxx_SPI1_CS3_Set(w25qxx_CS_State_t newState)
{
    switch (newState)
    {
    case W25QXX_CS_HIGH:
        HAL_GPIO_WritePin(SPI1_CS3_PORT, SPI1_CS3_PIN, GPIO_PIN_SET);
        break;

    case W25QXX_CS_LOW:
        HAL_GPIO_WritePin(SPI1_CS3_PORT, SPI1_CS3_PIN, GPIO_PIN_RESET);
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
    HAL_UART_Transmit(&huart1, message, strlen((const char *) message), UART_TRANSMIT_TIMEOUT);
}