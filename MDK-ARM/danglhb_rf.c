#include "danglhb_rf.h"
#include <stdint.h>
#include "main.h"


// uint32_t data_ir_remote = 0;
uint32_t data_rf_remote = 0;



static inline uint32_t LL_SYSTICK_IsActiveCounterFlag(void)
{
  return ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == (SysTick_CTRL_COUNTFLAG_Msk));
}


static uint32_t getCurrentMicros(void)
{
  /* Ensure COUNTFLAG is reset by reading SysTick control and status register */
  LL_SYSTICK_IsActiveCounterFlag();
  uint32_t m = HAL_GetTick();
  const uint32_t tms = SysTick->LOAD + 1;
  __IO uint32_t u = tms - SysTick->VAL;
  if (LL_SYSTICK_IsActiveCounterFlag()) {
    m = HAL_GetTick();
    u = tms - SysTick->VAL;
  }
  return (m * 1000 + (u * 1000) / tms);
}


uint32_t get_result_rf(void)
{
	return data_rf_remote;
}

static void Delay_us(uint32_t us)
{
	uint32_t last_time = getCurrentMicros();
	while((getCurrentMicros() - last_time) < us);
}

uint8_t encode_rf(uint8_t state)
{
	uint32_t count = 0, data = 0;
	uint8_t lock = 0;
	if(lock == 1 || state != 0)
		return 1;
	lock = 1;
	while (HAL_GPIO_ReadPin (GPIOA, GPIO_PIN_5));   // wait for the pin to go low
	
		count = 0;
    while (!(HAL_GPIO_ReadPin (GPIOA, GPIO_PIN_5)))  // wait for the pin to go high  9,8 ms LOW
		{
			count++;
      Delay_us(100);
		}
		if(count > 70)
		{
			// while (HAL_GPIO_ReadPin (GPIOA, GPIO_PIN_5));   // wait for the pin to go low
      for (int i=0; i<25; i++)
      {
                count=0;
                  // while (!(HAL_GPIO_ReadPin (GPIOA, GPIO_PIN_5))); // wait for pin to go high.. this is 562.5us LOW
                	while ((HAL_GPIO_ReadPin (GPIOA, GPIO_PIN_5))); // wait for pin to go low..

                  // while ((HAL_GPIO_ReadPin (GPIOA, GPIO_PIN_5)))  // count the space length while the pin is high
                  while (!(HAL_GPIO_ReadPin (GPIOA, GPIO_PIN_5)))  // count the space length while the pin is low
                 {
                  count++;
                  Delay_us(100);
                 }

                 if (count > 5) // if the space is more than 0.5ms, reality is  0.9 ms
                 {
                  data |= (1UL << (24-i));   // write 1
                 }

                 else data &= ~(1UL << (24-i));  // write 0
      }
		  data_rf_remote = data;
			Delay_us(100);
		  lock = 0;
			return 0;
		}
		lock = 0;
		return 1;
}




