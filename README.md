# Description
A simple library designed to perform basic write/read operations with serial flash memory devices of the w25qxx family.  
Data transfer is carried out by standard SPI instructions using the CLK, /CS, DI, DO pins.  
Based on the device ID, received from the IC, this library can calculate the number of pages to eliminate some address issues for write/read and erase operations.  
To make the use of the library as safe and understandable as possible, any operations with data are performed only starting from the first byte of the page (for example, for a zero page the address should be 0, for the first page - 256, etc.)
# Supported devices
* w25q80
* w25q16
* w25q32
* w25q64
* w25q128

# Example conditions
`Toolchain - IAR EWARM v9.40.1`  
`Target MCU - STM32F407VGT6 (STM32F4XX_M devBoard)`  

# Quick start
## Common routine
* Don't forget the following line:
```C
#include "w25qxx.h"
```
* Provide defines regarding to Chip Select pin:
```C
#define CS0_Pin       GPIO_PIN_15
#define CS0_GPIO_Port GPIOA
```
* Initialize SPIx periphery
* Initialize the FLASH device:
```C
w25qxx_Init(&w25qxx_Handle, SPIx, CS0_GPIO_Port, CS0_Pin);
```
where `SPIx` could be replaced by `hspix.Instance` (for HAL only)
## Interfacing with HAL
* In `w25qxx_Interface.h` uncomment definitions if you are going to use not only SPI1:
```C
/* HAL SPIx definitions */
#ifdef USE_HAL_DRIVER
#define USE_SPI1
// #define USE_SPI2
// #define USE_SPI3
#endif
```
* Now compiler is looking for hspix declaration so ensure that it really exists   
in `spi.h` that CubeMX creates. Usually it looks like that:
```C
/* USER CODE END Includes */
extern SPI_HandleTypeDef hspi1;
/* USER CODE BEGIN Private defines */
```
## Interfacing with SPL
* In `w25qxx_Interface.h` provide your own `SPI.h` and `Delay.h` includes   
* In `w25qxx_Interface.c` change next func calls to yours:
```C
SPI_Transmit(SPIx, pBuffer, lengthTX);
///
SPI_Receive(SPIx, pBuffer, lengthRX);
///
_delay_ms(ms);
```
# Example
For application use refer to [`HAL/../main.c`](./HAL/Core/Src/main.c) or [`SPL/../main.c`](./SPL/Source/main.c) 
