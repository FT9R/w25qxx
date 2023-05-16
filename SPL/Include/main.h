#ifndef MAIN_H
#define MAIN_H

//#define USE_IO_TERMINAL
#define PAGE	1408
#define PAGE_ADDRESS	(PAGE * W25QXX_PAGE_SIZE)
#define CS0_Pin GPIO_Pin_15
#define CS0_GPIO_Port GPIOA

#include "stm32f4xx.h"
#include "Sys_Init.h"
#include "Init.h"
#include "w25qxx.h"
#include <string.h>
#include <stdio.h>

#ifndef USE_IO_TERMINAL
#define printf(param, ...) ((void)0)
#endif

#endif