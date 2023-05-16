#include "danglhb_rf.h"
#include <stdint.h>
#include "handle_interupt.h"
#include "stdio.h"
#include "string.h"


// uint32_t data_ir_remote = 0;
uint32_t data_rf_remote = 0;

extern event event_interupt; 
extern UART_HandleTypeDef huart1;



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
	//event_interupt = DANGLHB;
	uint32_t count = 0, data = 0;
	uint8_t lock = 0;
	if(lock == 1 || state != 0)
		return 1;
	lock = 1;
	
	//	count = 0;
	    //while ((HAL_GPIO_ReadPin (GPIOA, GPIO_PIN_0)));

    while (!(HAL_GPIO_ReadPin (GPIOA, GPIO_PIN_0)))  // wait for the pin to go high  9,8 ms LOW
		{
		//	if(count < 200)
			//{
			count++;
      Delay_us(1);
			//}else break;
		}
		if(count > 40)
		{
      for (int i=0; i<25; i++)
      {
                count=0;
                	while ((HAL_GPIO_ReadPin (GPIOA, GPIO_PIN_0))); // wait for pin to go low..

                  // while ((HAL_GPIO_ReadPin (GPIOA, GPIO_PIN_0)))  // count the space length while the pin is high
                  while (!(HAL_GPIO_ReadPin (GPIOA, GPIO_PIN_0)))  // count the space length while the pin is low
                 {
										count++;
										Delay_us(1);
                 }

                 if (count > 40) // if the space is more than 1ms, reality is  ~ 1.2 ms
                 {
										data |= (1UL << (24-i));   // write 1
                 }

                 else if(count < 40)
									 data &= ~(1UL << (24-i));  // write 0;
								 else break;
      }
		  data_rf_remote = data;
			return 0;
		}
		return 1;
}




