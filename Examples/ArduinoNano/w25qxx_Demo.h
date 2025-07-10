#pragma once

#include "w25qxx.h"

#define DEMO_TARGET_PAGE 31

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief w25qxx basic demonstration function
 * @param fpPrint a pointer to function that prints debug messages
 * @param forceChipErase pass `true` to initiate chip erase before target page read
 * @return
 */
uint8_t w25qxx_Demo(void (*fpPrint)(char *message), bool forceChipErase);

#ifdef __cplusplus
}
#endif