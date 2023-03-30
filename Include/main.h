#ifndef MAIN_H
#define MAIN_H

//#define USE_IO_TERMINAL
#define PAGE	1408
#define PAGE_ADDRESS	(PAGE * PageSize)

#include "stm32f4xx.h"
#include "Sys_Init.h"
#include <string.h>
#include <stdio.h>
#include "Delay.h"
#include "Init.h"
#include "SPI.h"
#include "w25qxx.h"

#ifndef USE_IO_TERMINAL
#define printf(param, ...) ((void)0)
#endif

#endif