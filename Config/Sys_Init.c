#include "Sys_Init.h"

RCC_ClocksTypeDef rcc_clocks;


// setting F_CPU to 168 MHz, assuming that external crystal freq = 8 MHz
void Sys_Init(void)
{
	SystemInit();

	SET_BIT(FLASH->ACR, FLASH_ACR_PRFTEN);	// Prefetch enable
	MODIFY_REG(FLASH->ACR, FLASH_ACR_LATENCY, FLASH_ACR_LATENCY_5WS);	// 5 wait states, if 150 < HCLK <= 168

	SET_BIT(RCC->CR, RCC_CR_HSEON);	// HSE oscillator ON
	while(!READ_BIT(RCC->CR, RCC_CR_HSERDY));	// wait until HSE oscillator is ready;
	SET_BIT(RCC->CR, RCC_CR_CSSON);	// Clock security system ON (Clock detector ON if HSE oscillator is stable, OFF if not)

	SET_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC);	// HSE oscillator clock selected as PLL and PLLI2S clock entry
	MODIFY_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLLM, 4 << 0);	// 4x Division factor for the main PLL (2MHz VCO input frequency)
	MODIFY_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLLN, 168 << 6);	// 168x multiplication factor for VCO (336MHz VCO output frequency)
	MODIFY_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLLP, 0);	// 2x PLL division factor for main system clock (168MHz PLL out)
	MODIFY_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLLQ, 7 << 24);	// 2x PLL division factor for main system clock (168MHz PLL out)
	//	 RCC_PLLConfig(RCC_PLLSource_HSE, 4, 168, 2, 7);

	SET_BIT(RCC->CR, RCC_CR_PLLON);	// PLL ON
	while(!READ_BIT(RCC->CR, RCC_CR_PLLRDY));	// wait until PLL is ready

	MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, RCC_CFGR_SW_PLL);	// PLL selected as system clock

	MODIFY_REG(RCC->CFGR, RCC_CFGR_HPRE, RCC_CFGR_HPRE_DIV1);	// AHB prescaler
	MODIFY_REG(RCC->CFGR, RCC_CFGR_PPRE1, RCC_CFGR_PPRE1_DIV4);	// APB low-speed prescaler (APB1 42 MHz)
	MODIFY_REG(RCC->CFGR, RCC_CFGR_PPRE2, RCC_CFGR_PPRE2_DIV2);	// APB high-speed prescaler (APB2 84 Mhz)

	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);	// wait until system will use PLL as system clock

	SystemCoreClockUpdate();
	RCC_GetClocksFreq(&rcc_clocks);
}