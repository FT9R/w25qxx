/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "main.h"
#include "task.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "trace.h"
#include "w25qxx_Demo.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define W25QXX_FLAG_IDLE  (1 << 0)
#define W25QXX_FLAG_BUSY  (1 << 1)
#define W25QXX_FLAG_ERASE (1 << 2)
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for blinkTask */
osThreadId_t blinkTaskHandle;
const osThreadAttr_t blinkTask_attributes = {
    .name = "blinkTask",
    .stack_size = 64 * 4,
    .priority = (osPriority_t) osPriorityBelowNormal6,
};
/* Definitions for w25qxxTask */
osThreadId_t w25qxxTaskHandle;
const osThreadAttr_t w25qxxTask_attributes = {
    .name = "w25qxxTask",
    .stack_size = 512 * 4,
    .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for traceTask */
osThreadId_t traceTaskHandle;
const osThreadAttr_t traceTask_attributes = {
    .name = "traceTask",
    .stack_size = 512 * 4,
    .priority = (osPriority_t) osPriorityBelowNormal7,
};
/* Definitions for msgQueue */
osMessageQueueId_t msgQueueHandle;
const osMessageQueueAttr_t msgQueue_attributes = {.name = "msgQueue"};
/* Definitions for uartReadySem */
osSemaphoreId_t uartReadySemHandle;
const osSemaphoreAttr_t uartReadySem_attributes = {.name = "uartReadySem"};
/* Definitions for w25qxxEvent */
osEventFlagsId_t w25qxxEventHandle;
const osEventFlagsAttr_t w25qxxEvent_attributes = {.name = "w25qxxEvent"};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void BlinkStart(void *argument);
void w25qxxStart(void *argument);
void TraceStart(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void MX_FREERTOS_Init(void)
{
    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* USER CODE BEGIN RTOS_MUTEX */
    /* add mutexes, ... */
    /* USER CODE END RTOS_MUTEX */

    /* Create the semaphores(s) */
    /* creation of uartReadySem */
    uartReadySemHandle = osSemaphoreNew(1, 1, &uartReadySem_attributes);

    /* USER CODE BEGIN RTOS_SEMAPHORES */
    /* add semaphores, ... */
    /* USER CODE END RTOS_SEMAPHORES */

    /* USER CODE BEGIN RTOS_TIMERS */
    /* start timers, add new ones, ... */
    /* USER CODE END RTOS_TIMERS */

    /* Create the queue(s) */
    /* creation of msgQueue */
    msgQueueHandle = osMessageQueueNew(10, sizeof(msg_t), &msgQueue_attributes);

    /* USER CODE BEGIN RTOS_QUEUES */
    /* add queues, ... */
    /* USER CODE END RTOS_QUEUES */

    /* Create the thread(s) */
    /* creation of blinkTask */
    blinkTaskHandle = osThreadNew(BlinkStart, NULL, &blinkTask_attributes);

    /* creation of w25qxxTask */
    w25qxxTaskHandle = osThreadNew(w25qxxStart, NULL, &w25qxxTask_attributes);

    /* creation of traceTask */
    traceTaskHandle = osThreadNew(TraceStart, NULL, &traceTask_attributes);

    /* USER CODE BEGIN RTOS_THREADS */
    /* add threads, ... */
    /* USER CODE END RTOS_THREADS */

    /* Create the event(s) */
    /* creation of w25qxxEvent */
    w25qxxEventHandle = osEventFlagsNew(&w25qxxEvent_attributes);

    /* USER CODE BEGIN RTOS_EVENTS */
    /* add events, ... */
    /* USER CODE END RTOS_EVENTS */
}

/* USER CODE BEGIN Header_BlinkStart */
/**
 * @brief  Function implementing the blinkTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_BlinkStart */
void BlinkStart(void *argument)
{
    /* USER CODE BEGIN BlinkStart */
    uint32_t flag;
    uint32_t blink_interval = 10;

    /* Infinite loop */
    for (;;)
    {
        flag = osEventFlagsWait(w25qxxEventHandle, W25QXX_FLAG_IDLE | W25QXX_FLAG_BUSY | W25QXX_FLAG_ERASE,
                                osFlagsWaitAny | osFlagsNoClear, osWaitForever);

        switch (flag)
        {
        case W25QXX_FLAG_IDLE:
            blink_interval = 5000;
            break;
        case W25QXX_FLAG_BUSY:
            blink_interval = 500;
            break;
        case W25QXX_FLAG_ERASE:
            blink_interval = 50;
            break;
        default:
            osEventFlagsClear(w25qxxEventHandle, 0xff); // Race condition potential
            printf("BlinkStart: few flags detected 0x%08x\n", flag);
        }

        HAL_GPIO_TogglePin(LEDB_GPIO_Port, LEDB_Pin);
        osDelay(blink_interval);
    }
    /* USER CODE END BlinkStart */
}

/* USER CODE BEGIN Header_w25qxxStart */
/**
 * @brief Function implementing the w25qxxTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_w25qxxStart */
void w25qxxStart(void *argument)
{
    /* USER CODE BEGIN w25qxxStart */
    /* Infinite loop */
    for (;;)
    {
        w25qxx_Demo(Trace, true);
        osDelay(osWaitForever);
    }
    /* USER CODE END w25qxxStart */
}

/* USER CODE BEGIN Header_TraceStart */
/**
 * @brief Function implementing the traceTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_TraceStart */
void TraceStart(void *argument)
{
    /* USER CODE BEGIN TraceStart */
    osStatus_t uartReadySemAcqStatus;
    osStatus_t msgQueueGetStatus;
    msg_t msg;

    /* Infinite loop */
    for (;;)
    {
        /* Wait for tx complete signal from uart ISR */
        uartReadySemAcqStatus = osSemaphoreAcquire(uartReadySemHandle, 100);
        if (uartReadySemAcqStatus != osOK)
        {
            printf("Error acquiring semaphore in TraceStart: %d\n", uartReadySemAcqStatus);
            continue;
        }

        /* Get message from queue to be printed */
        msgQueueGetStatus = osMessageQueueGet(msgQueueHandle, &msg, NULL, osWaitForever);
        if (msgQueueGetStatus != osOK)
        {
            printf("Error getting message from queue in TraceStart: %d\n", msgQueueGetStatus);
            continue;
        }
        else
            w25qxx_Print(msg.data);

        /* Clear event flags for blink task */
        osEventFlagsClear(w25qxxEventHandle, 0xff);
        if (strstr(msg.data, "erase..."))
            osEventFlagsSet(w25qxxEventHandle, W25QXX_FLAG_ERASE);
        else if (strstr(msg.data, "success"))
            osEventFlagsSet(w25qxxEventHandle, W25QXX_FLAG_IDLE);
        else
            osEventFlagsSet(w25qxxEventHandle, W25QXX_FLAG_BUSY);
    }
    /* USER CODE END TraceStart */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
