#include "hardware/sync.h"
#include "pico/stdlib.h"
#include "w25qxx_Demo.h"
#include <stdio.h>

volatile uint32_t uwTick;

bool repeating_timer_callback(repeating_timer_t *t)
{
    ++uwTick;

    return true;
}

static void IO_Init(void)
{
    /* USER LED */
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, true);
}

static void SPI0_Init(uint baudrate)
{
    /* RX, TX, SCK */
    spi_init(spi_default, baudrate);
    gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);

    /* CS0 */
    gpio_init(PICO_DEFAULT_SPI_CSN_PIN);
    gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 1);
    gpio_set_dir(PICO_DEFAULT_SPI_CSN_PIN, GPIO_OUT);
}

/**
 * @brief  This function is executed in case of error occurrence
 * @retval None
 */
void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    save_and_disable_interrupts();

    while (1) {}
    /* USER CODE END Error_Handler_Debug */
}

void main()
{
    stdio_init_all();
    IO_Init();
    SPI0_Init(1e5);

    /* Timer 1kHz */
    repeating_timer_t timer;
    add_repeating_timer_us(1000, repeating_timer_callback, NULL, &timer);

    while (true)
    {
        if (w25qxx_Demo(w25qxx_Print))
            Error_Handler();
        gpio_put(PICO_DEFAULT_LED_PIN, false);
    }
}