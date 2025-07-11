#pragma once

#include "cmsis_os.h"
#include "string.h"
#include <stdio.h>

#define MSG_SIZE_MAX 100

typedef struct
{
    char data[MSG_SIZE_MAX + 1]; // +1 for null terminator
    size_t size;
} msg_t;

extern osMessageQueueId_t msgQueueHandle;

/**
 * @brief Puts a trace message to the message queue
 * @param message pointer to the message string to be traced
 */
void Trace(const char *message);