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

w25qxx_Transfer_Status_t w25qxx_SPI1_Transmit(const uint8_t *pDataTx, uint16_t size, uint32_t timeout)
{
    if (pDataTx == NULL)
        return W25QXX_TRANSFER_ERROR;
    if (size == 0u)
        return W25QXX_TRANSFER_ERROR;

    return (HAL_SPI_Transmit(&hspi1, (uint8_t *) pDataTx, size, timeout) == HAL_OK) ? W25QXX_TRANSFER_SUCCESS
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

void w25qxx_Delay(uint32_t ms)
{
    osStatus_t delayStatus = osDelay(ms);
    if (delayStatus != osOK)
    {
        w25qxx_Print("w25qxx_Delay: OS wait for timeout error\n");
    }
}

void w25qxx_Print(const char *message)
{
    HAL_StatusTypeDef transmitStatus = HAL_UART_Transmit_IT(&huart1, (const uint8_t *) message, strlen(message));
    if (transmitStatus != HAL_OK)
    {
        printf("w25qxx_Print: UART transmission error (%d)\n", transmitStatus);
        printf("Message: %s\n", message);
    }
}