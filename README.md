# Description
A simple library designed to perform basic write/read operations with serial flash memory devices of the w25qxx family. 
## Features
* Many devices on the same bus are supported with its dedicated handles:
```C
/* First device */
w25qxx_Init(&w25qxx_Handle, &hspi1, CS0_GPIO_Port, CS0_Pin);

/* Second device */
w25qxx_Init(&w25qxx_Handle1, &hspi1, CS1_GPIO_Port, CS1_Pin);

/* Third device */
w25qxx_Init(&w25qxx_Handle2, &hspi1, CS2_GPIO_Port, CS2_Pin);
```
* Data transfer is carried out by standard SPI instructions, using the CLK, /CS, DI, DO pins.  
* Based on the device ID this library can calculate the number of pages to eliminate some address issues for write/read and erase operations.
* There are several options for waiting for the end of page program/erase instruction with dedicated timeouts.
* The built-in ModBus CRC can be used to ensure data integrity.
## Note
To make the use of the library as safe and understandable as possible, any operations with data are performed only starting from the first byte of the page 
(e.g., for the first page the address should be 0, for the second page - 256, etc.).   

# Supported devices
* w25q80
* w25q16
* w25q32
* w25q64
* w25q128

# Quick start
## Common routine
* Don't forget the following line:
```C
#include "w25qxx.h"
```
* Declare the device handle:
```C
w25qxx_HandleTypeDef w25qxx_Handle;
```
* Provide defines regarding to Chip Select pin:
```C
#define CS0_Pin       GPIO_PIN_15
#define CS0_GPIO_Port GPIOA
```
* Initialize SPIx periphery
* Initialize the FLASH device:
```C
w25qxx_Init(&w25qxx_Handle, &hspi1, CS0_GPIO_Port, CS0_Pin);
```
## Interfacing with HAL
This library should work out of box together with HAL 
## Interfacing with SPL
* In `w25qxx_Interface.h` provide your own `SPI.h` and `Delay.h` includes   
* In `w25qxx_Interface.c` change next func calls to yours:
```C
SPI_Transmit(hspix, pData, size, timeout);
///
SPI_Receive(hspix, pData, size, timeout);
///
Delay(ms);
```
Or just use existing SPL SPI driver, which are oversimplificated but still in manner of HAL driver
# Example
## Conditions
`Toolchain - IAR EWARM v9.40.1`  
`Target MCU - STM32F407VGT6 (STM32F4XX_M devboard)`
## References
For application use refer to [`HAL/../main.c`](./HAL/Core/Src/main.c) or [`SPL/../main.c`](./SPL/Source/main.c) 
