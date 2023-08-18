#include "w25qxx_Interface.h"

void w25qxx_SPI_Transmit(SPI_TypeDef *SPIx, const uint8_t *pBuffer, uint32_t lengthTX)
{
#ifdef USE_HAL_DRIVER
    switch ((unsigned long) SPIx)
    {
#ifdef USE_SPI1
    case SPI1_BASE:
        HAL_SPI_Transmit(&hspi1, (uint8_t *) pBuffer, lengthTX, W25QXX_TX_TIMEOUT);
        break;
#endif
#ifdef USE_SPI2
    case SPI2_BASE:
        HAL_SPI_Transmit(&hspi2, (uint8_t *) pBuffer, lengthTX, W25QXX_TX_TIMEOUT);
        break;
#endif
#ifdef USE_SPI3
    case SPI3_BASE:
        HAL_SPI_Transmit(&hspi3, (uint8_t *) pBuffer, lengthTX, W25QXX_TX_TIMEOUT);
        break;
#endif
    }
#else
    SPI_Transmit(SPIx, pBuffer, lengthTX);
#endif
}

void w25qxx_SPI_Receive(SPI_TypeDef *SPIx, uint8_t *pBuffer, uint32_t lengthRX)
{
#ifdef USE_HAL_DRIVER
    switch ((unsigned long) SPIx)
    {
#ifdef USE_SPI1
    case SPI1_BASE:
        HAL_SPI_Receive(&hspi1, (uint8_t *) pBuffer, lengthRX, W25QXX_RX_TIMEOUT);
        break;
#endif
#ifdef USE_SPI2
    case SPI2_BASE:
        HAL_SPI_Receive(&hspi2, (uint8_t *) pBuffer, lengthRX, W25QXX_RX_TIMEOUT);
        break;
#endif
#ifdef USE_SPI3
    case SPI3_BASE:
        HAL_SPI_Receive(&hspi3, (uint8_t *) pBuffer, lengthRX, W25QXX_RX_TIMEOUT);
        break;
#endif
    }
#else
    SPI_Receive(SPIx, pBuffer, lengthRX);
#endif
}

void w25qxx_Delay(const uint32_t ms)
{
#ifdef USE_HAL_DRIVER
    HAL_Delay(ms);
#else
    _delay_ms(ms);
#endif
}
