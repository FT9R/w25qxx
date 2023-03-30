#include "Init.h"

void IO_Init(void)
{
	SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN);
	SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOBEN);

	/* SPI1 */
	// PA15 - NSS
	CS0_High;
	MODIFY_REG(GPIOA->MODER, GPIO_MODER_MODER15, GPIO_MODER_MODER15_0);	// 01: General purpose output mode
	CLEAR_BIT(GPIOA->OTYPER, GPIO_OTYPER_OT_15);	// 0: Output push-pull (reset state)
	MODIFY_REG(GPIOA->OSPEEDR, GPIO_OSPEEDER_OSPEEDR15, GPIO_OSPEEDER_OSPEEDR15_0 | GPIO_OSPEEDER_OSPEEDR15_1);	// 11: Very high speed
	// PB3 - SCK
	MODIFY_REG(GPIOB->MODER, GPIO_MODER_MODER3, GPIO_MODER_MODER3_1);	// 10: Alternate function mode
	MODIFY_REG(GPIOB->AFR[0], 0xF << 4*3, 5 << 4*3);	// 0101: AF5
	MODIFY_REG(GPIOB->OSPEEDR, GPIO_OSPEEDER_OSPEEDR3, GPIO_OSPEEDER_OSPEEDR3_0 | GPIO_OSPEEDER_OSPEEDR3_1);	// 11: Very high speed
	// PB4 - MISO
	MODIFY_REG(GPIOB->MODER, GPIO_MODER_MODER4, GPIO_MODER_MODER4_1);	// 10: Alternate function mode
	MODIFY_REG(GPIOB->AFR[0], 0xF << 4*4, 5 << 4*4);	// 0101: AF5
	MODIFY_REG(GPIOB->OSPEEDR, GPIO_OSPEEDER_OSPEEDR4, GPIO_OSPEEDER_OSPEEDR4_0 | GPIO_OSPEEDER_OSPEEDR4_1);	// 11: Very high speed
	// PB5 - MOSI
	MODIFY_REG(GPIOB->MODER, GPIO_MODER_MODER5, GPIO_MODER_MODER5_1);	// 10: Alternate function mode
	MODIFY_REG(GPIOB->AFR[0], 0xF << 4*5, 5 << 4*5);	// 0101: AF5
	MODIFY_REG(GPIOB->OSPEEDR, GPIO_OSPEEDER_OSPEEDR5, GPIO_OSPEEDER_OSPEEDR5_0 | GPIO_OSPEEDER_OSPEEDR5_1);	// 11: Very high speed

	//	/* PA8 - Test pin */
	//	CLEAR_BIT(GPIOA->ODR, GPIO_ODR_ODR8);
	//	MODIFY_REG(GPIOA->MODER, GPIO_MODER_MODER8, GPIO_MODER_MODER8_0);	// 01: General purpose output mode
	//	CLEAR_BIT(GPIOA->OTYPER, GPIO_OTYPER_OT_8);	// 0: Output push-pull (reset state)
	//	MODIFY_REG(GPIOA->OSPEEDR, GPIO_OSPEEDER_OSPEEDR8, GPIO_OSPEEDER_OSPEEDR8_0 | GPIO_OSPEEDER_OSPEEDR8_1);	// 11: Very high speed
}