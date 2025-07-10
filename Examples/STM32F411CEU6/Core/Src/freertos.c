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
    /* Infinite loop */
    for (;;)
    {
        osDelay(1);
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
        osDelay(1);
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
    msg_t msg;

    for (;;)
    {
        if (osMessageQueueGet(msgQueueHandle, &msg, NULL, 0) == osOK)
        {
            w25qxx_Print(msg.data);
        }
        osDelay(1);
    }
    /* USER CODE END TraceStart */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
