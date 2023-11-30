#include "w25qxx_Interface.h"

ErrorStatus w25qxx_SPI1_Receive(uint8_t *pDataRx, uint16_t size, uint32_t timeout)
{
    if ((pDataRx != NULL) && (size != 0))
        return (HAL_SPI_Receive(&hspi1, pDataRx, size, timeout) == HAL_OK) ? SUCCESS : ERROR;
    else
        return ERROR;
}

ErrorStatus w25qxx_SPI1_Transmit(uint8_t *pDataTx, uint16_t size, uint32_t timeout)
{
    if ((pDataTx != NULL) && (size != 0))
        return (HAL_SPI_Transmit(&hspi1, pDataTx, size, timeout) == HAL_OK) ? SUCCESS : ERROR;
    else
        return ERROR;
}

void w25qxx_CS1_Set(w25qxx_CS_State_t newState)
{
    switch (newState)
    {
    case W25QXX_CS_HIGH:
        HAL_GPIO_WritePin(CS0_GPIO_Port, CS0_Pin, GPIO_PIN_SET);
        break;

    case W25QXX_CS_LOW:
        HAL_GPIO_WritePin(CS0_GPIO_Port, CS0_Pin, GPIO_PIN_RESET);
        break;

    default:
        break;
    }
}

void w25qxx_Delay(uint32_t ms)
{
#ifdef USE_HAL_DRIVER
    HAL_Delay(ms);
#else
    Delay(ms);
#endif
}