#include "trace.h"

void Trace(const char *message)
{
    msg_t msg;

    if (message == NULL || msgQueueHandle == NULL)
    {
        printf("Trace: null message or message queue handle\n");

        return; // Avoid null pointer dereference
    }

    msg.size = strlen(message);
    if (msg.size > MSG_SIZE_MAX)
        printf("Trace: message too long - %d chars, truncating\n", msg.size);

    strncpy(msg.data, message, MSG_SIZE_MAX);
    msg.data[MSG_SIZE_MAX - 1] = '\n';
    msg.data[MSG_SIZE_MAX] = '\0'; // Ensure null termination
    osMessageQueuePut(msgQueueHandle, &msg, 0, 0);
}