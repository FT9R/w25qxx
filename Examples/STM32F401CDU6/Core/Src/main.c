/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
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
#include "main.h"
#include "gpio.h"
#include "spi.h"
#include "usart.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "w25qxx.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct Serial_s
{
    void (*print)(void *data);
    void (*println)(void *data);
} Serial_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define PAGE                  1498
#define UART_TRANSMIT_TIMEOUT 1000
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
w25qxx_HandleTypeDef w25qxx_Handle;
Serial_t Serial;
const uint8_t bufferWrite[] = "Hello World!";
uint8_t bufferRead[sizeof(bufferWrite)] = {0};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static void print(void *data)
{
    HAL_UART_Transmit(&huart1, data, strlen(data), UART_TRANSMIT_TIMEOUT);
}
static void println(void *data)
{
    print(data);
    HAL_UART_Transmit(&huart1, "\n", 1u, UART_TRANSMIT_TIMEOUT);
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
    /* USER CODE BEGIN 1 */

    /* USER CODE END 1 */

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* USER CODE BEGIN Init */
    Serial.print = print;
    Serial.println = println;
    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_SPI1_Init();
    MX_USART1_UART_Init();
    /* USER CODE BEGIN 2 */
    /* w25qxx demo */
    Serial.println("Interface link");
    w25qxx_Link(&w25qxx_Handle, w25qxx_SPI1_Receive, w25qxx_SPI1_Transmit, w25qxx_SPI1_CS0_Set);

    Serial.println("Device initialization");
    w25qxx_Init(&w25qxx_Handle);

    Serial.println("Forcing status registers to default state");
    w25qxx_Handle.statusRegister = 0x00;
    w25qxx_WriteStatus(&w25qxx_Handle, 1u, W25QXX_SR_VOLATILE);
    w25qxx_WriteStatus(&w25qxx_Handle, 2u, W25QXX_SR_VOLATILE);
    w25qxx_WriteStatus(&w25qxx_Handle, 3u, W25QXX_SR_VOLATILE);

    Serial.println("First approach to read");
    w25qxx_Read(&w25qxx_Handle, bufferRead, sizeof(bufferRead), W25QXX_PAGE_ADDRESS(PAGE), W25QXX_CRC,
                W25QXX_FASTREAD_NO);
    switch (w25qxx_Handle.error)
    {
    case W25QXX_ERROR_NONE:
        if (memcmp(bufferRead, bufferWrite, sizeof(bufferRead)) == 0)
        {
            Serial.println("Data already exist at target page boundaries");
        }
        break;

    case W25QXX_ERROR_CHECKSUM:
        Serial.println("Target page probably contains corrupted data or erased");
        Serial.println("Checksum error reset");
        w25qxx_ResetError(&w25qxx_Handle);

        Serial.println("Whole chip erase");
        w25qxx_Erase(&w25qxx_Handle, W25QXX_CHIP_ERASE, 0, W25QXX_WAIT_BUSY);

        Serial.println("Page programming");
        w25qxx_Write(&w25qxx_Handle, bufferWrite, sizeof(bufferWrite), W25QXX_PAGE_ADDRESS(PAGE), W25QXX_CRC,
                     W25QXX_WAIT_BUSY);

        Serial.println("Second approach to read");
        w25qxx_Read(&w25qxx_Handle, bufferRead, sizeof(bufferRead), W25QXX_PAGE_ADDRESS(PAGE), W25QXX_CRC,
                    W25QXX_FASTREAD_NO);
        if (memcmp(bufferRead, bufferWrite, sizeof(bufferRead)) == 0)
        {
            Serial.println("Writing process success");
        }
        else
        {
            Serial.println("Writing process failure");
            Error_Handler();
        }
        break;

    default:
        Error_Handler();
        break;
    }

    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Configure the main internal regulator output voltage
     */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 25;
    RCC_OscInitStruct.PLL.PLLN = 168;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 4;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();

    Serial.print("An error occured: ");
    switch (w25qxx_Handle.error)
    {
    case W25QXX_ERROR_NONE:
        break;

    case W25QXX_ERROR_STATUS_MISMATCH:
        Serial.println("status match");
        break;

    case W25QXX_ERROR_INITIALIZATION:
        Serial.println("initialization");
        break;

    case W25QXX_ERROR_ARGUMENT:
        Serial.println("argument");
        break;

    case W25QXX_ERROR_ADDRESS:
        Serial.println("address");
        break;

    case W25QXX_ERROR_SPI:
        Serial.println("spi");
        break;

    case W25QXX_ERROR_TIMEOUT:
        Serial.println("timeout");
        break;

    case W25QXX_ERROR_CHECKSUM:
        Serial.println("checksum");
        break;

    case W25QXX_ERROR_INSTRUCTION:
        Serial.println("instruction");
        break;
    }

    Serial.print("Last detected status: ");
    switch (w25qxx_Handle.status)
    {
    case W25QXX_STATUS_NOLINK:
        Serial.println("no link");
        break;

    case W25QXX_STATUS_RESET:
        Serial.println("reset");
        break;

    case W25QXX_STATUS_READY:
        Serial.println("ready");
        break;

    case W25QXX_STATUS_BUSY:
        Serial.println("busy");
        break;

    case W25QXX_STATUS_BUSY_INIT:
        Serial.println("busy init");
        break;

    case W25QXX_STATUS_BUSY_WRITE:
        Serial.println("busy write");
        break;

    case W25QXX_STATUS_BUSY_READ:
        Serial.println("busy read");
        break;

    case W25QXX_STATUS_BUSY_ERASE:
        Serial.println("busy erase");
        break;

    case W25QXX_STATUS_UNDEFINED:
        Serial.println("undefined");
        break;
    }

    while (1) {}
    /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
