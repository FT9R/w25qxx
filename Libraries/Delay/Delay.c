#include "Delay.h"


void _delay_us(const uint32_t us)
{
	if(us > (0xFFFFFF / ((SystemCoreClock / 8) / 1000000))) {assert(us < (0xFFFFFF / ((SystemCoreClock / 8) / 1000000))); return;}

	CLEAR_BIT(SysTick->CTRL, SysTick_CTRL_CLKSOURCE_Msk);	// 0: AHB/8
	SysTick->VAL = 0;	// A write of any value clears the field to 0, and also clears the COUNTFLAG bit in the STK_CTRL register to 0
	SysTick->LOAD = (((SystemCoreClock / 8) / 1000000) * us) - 1;
	SET_BIT(SysTick->CTRL, SysTick_CTRL_ENABLE_Msk);	// Counter enabled
	while (!READ_BIT(SysTick->CTRL, SysTick_CTRL_COUNTFLAG_Msk)) asm("nop");
	SysTick->CTRL = (uint32_t)0x00000000;	// Counter disabled
}

void _delay_ms(const uint32_t ms)
{
	_delay_us(ms * 1000);
}
