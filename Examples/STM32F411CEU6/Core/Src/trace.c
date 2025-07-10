#include "trace.h"

void Trace(char *message)
{
    msg_t msg;

    if (message == NULL || msgQueueHandle == NULL)
    {
        printf("Trace: null message or message queue handle\n");

        return; // Avoid null pointer dereference
    }

    if (strlen(message) > MSG_SIZE_MAX)
    {
        printf("Trace: message too long, truncating\n");
        message[MSG_SIZE_MAX - 1] = '\n';
        message[MSG_SIZE_MAX] = '\0'; // Ensure null termination
    }

    msg.size = strlen(message);
    strncpy(msg.data, message, MSG_SIZE_MAX);
    osMessageQueuePut(msgQueueHandle, &msg, 0, 0);
}