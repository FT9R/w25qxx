#ifndef MAIN_H
#define MAIN_H

#include "init.h"
#include "w25qxx.h"
#include <stdbool.h>

/* Definitions */
// #define USE_IO_TERMINAL
#define PAGE         1408
#define PAGE_ADDRESS (PAGE * W25QXX_PAGE_SIZE)

/* Macro */
#ifndef USE_IO_TERMINAL
#define printf(param, ...) ((void) 0)
#endif

void Error_Handler(void);

#endif
