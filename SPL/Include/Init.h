#ifndef INIT_H
#define INIT_H

#include "SPI.h"
#include "sys_init.h"

#define CS0_Pin       GPIO_Pin_15
#define CS0_GPIO_Port GPIOA

extern SPI_HandleTypeDef hspi1;

void IO_Init(void);

#endif