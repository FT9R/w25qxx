#pragma once

#include "w25qxx.h"

#define TARGET_PAGE 1725

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief w25qxx basic demonstration function
 * @param fpPrint: a pointer to function that prints debug messages
 * @return 0 - success
 */
uint8_t w25qxx_Demo(void (*fpPrint)(const uint8_t *message));

#ifdef __cplusplus
}
#endif