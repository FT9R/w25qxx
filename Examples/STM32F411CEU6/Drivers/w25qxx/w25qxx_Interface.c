#include "w25qxx_Interface.h"

#define W25QXX_DELAY_ROUNDUP 100
#define SPI1_CS0_PIN         SPI1_CS0_Pin
#define SPI1_CS0_PORT        SPI1_CS0_GPIO_Port

/**
 * @brief Rounds up an unsigned integer to the nearest multiple of another unsigned integer
 * @param value value to round up
 * @param roundTo value to round up to (must be non-zero)
 * @return Rounded-up value
 */
static uint32_t UInt_RoundUp(uint32_t value, uint32_t roundTo);

w25qxx_Transfer_Status_t w25qxx_SPI_Receive(void *handle, uint8_t *pDataRx, uint16_t size, uint32_t timeout)
{
    if (handle == NULL)
        return W25QXX_TRANSFER_ERROR;
    if (pDataRx == NULL)
        return W25QXX_TRANSFER_ERROR;
    if (size == 0u)
        return W25QXX_TRANSFER_ERROR;

    return (HAL_SPI_Receive(handle, pDataRx, size, timeout) == HAL_OK) ? W25QXX_TRANSFER_SUCCESS
                                                                       : W25QXX_TRANSFER_ERROR;
}

w25qxx_Transfer_Status_t w25qxx_SPI_Transmit(void *handle, const uint8_t *pDataTx, uint16_t size, uint32_t timeout)
{
    if (handle == NULL)
        return W25QXX_TRANSFER_ERROR;
    if (pDataTx == NULL)
        return W25QXX_TRANSFER_ERROR;
    if (size == 0u)
        return W25QXX_TRANSFER_ERROR;

    return (HAL_SPI_Transmit(handle, (uint8_t *) pDataTx, size, timeout) == HAL_OK) ? W25QXX_TRANSFER_SUCCESS
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

uint32_t w25qxx_Delay(uint32_t ms)
{
    uint32_t msRounded = UInt_RoundUp(ms, W25QXX_DELAY_ROUNDUP);
    osStatus_t delayStatus = osDelay(msRounded);
    if (delayStatus != osOK)
    {
        w25qxx_Print("w25qxx_Delay: OS wait for timeout error\n");

        return 0;
    }

    return msRounded;
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

/**
 * @section Private Functions
 */
static uint32_t UInt_RoundUp(uint32_t value, uint32_t roundTo)
{
    if (roundTo == 0)
        return value;

    return ((value + roundTo - 1) / roundTo) * roundTo;
}