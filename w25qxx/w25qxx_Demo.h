#pragma once

#include "w25qxx.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief w25qxx basic demonstration function
 * @param fpPrint a pointer to function that prints debug messages
 * @param forceChipErase pass `true` to initiate chip erase before target page read
 * @return `0` on success, `1` on failure
 */
uint8_t w25qxx_Demo(w25qxx_print_fp fpPrint, bool forceChipErase);

#ifdef __cplusplus
}
#endif