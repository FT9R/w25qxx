# Decsription
A simple library designed to perform basic write/read operations with serial flash memory devices of the w25qxx family.  
Data transfer is carried out by standard SPI instructions using the CLK, /CS, DI, DO pins.  
Based on the device ID, received from the IC, this library can calculate the number of pages to eliminate some address issues for write/read and erase operations.  
To make the use of the library as safe and understandable as possible, any operations with data are performed only starting from the first byte of the page (for example, for a zero page the address should be 0, for the first page - 256, etc.)
## Supported devices
* w25q80
* w25q16
* w25q32
* w25q64
* w25q128

## Conditions
`Toolchain - IAR EWARM v9.20.1`  
`Target MCU - STM32F407VGT6 (STM32F4XX_M devBoard)`  

## Quick start
### Usage with HAL
In `w25qxx_Interface.c` uncomment sections if you are going to use not only SPI1:
```
case SPIx_BASE:
HAL_SPI_Transmit(&hspix, (uint8_t *)pBuffer, lengthTX, 1000);
break;
///
case SPIx_BASE:
HAL_SPI_Receive(&hspix, (uint8_t *)pBuffer, lengthRX, 1000);
break;
```
### Usage with SPL
In `w25qxx_Interface.h` provide your own `SPI.h` and `Delay.h` includes
In `w25qxx_Interface.c` change next sections to yours:
```
SPI_Transmit(SPIx, pBuffer, lengthTX);
///
SPI_Receive(SPIx, pBuffer, lengthRX);
///
_delay_ms(ms);
```
