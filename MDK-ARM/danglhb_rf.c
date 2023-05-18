#include "danglhb_rf.h"
#include <stdint.h>
#include "handle_interupt.h"
#include "stdio.h"
#include "string.h"


// uint32_t data_ir_remote = 0;
uint32_t data_rf_remote = 0;
uint8_t lock = 0;

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
	uint32_t count = 0, data = 0,count_2 =0;
		//char msg[14];
	    //while ((HAL_GPIO_ReadPin (GPIOA, GPIO_PIN_0)));

    while (!(HAL_GPIO_ReadPin (GPIOA, GPIO_PIN_0)) && count <200)  // wait for the pin to go high  9,8 ms LOW
		{
			count++;
      Delay_us(1);
		}
									//sprintf(msg, "\ncount=%d",count);
									//HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
		if(count > 50)
		{
      for (int i=0; i<25; i++)
      {
                count=0;
								count_2 = 0;

                  // while ((HAL_GPIO_ReadPin (GPIOA, GPIO_PIN_0)))  // count the space length while the pin is high
                  while ((HAL_GPIO_ReadPin (GPIOA, GPIO_PIN_0)))  // count the space length while the pin is low
                 {
										count++;
										Delay_us(1);
                 }
								 //	sprintf(msg, "\ncount=%d",count);
									//HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
								 
								 	//while (!(HAL_GPIO_ReadPin (GPIOA, GPIO_PIN_0)) ); // wait for pin to go low..
						     if (count < 30) // if the space is more than 1ms, reality is  ~ 1.2 ms
                 {
										data |= (1UL << (24-i));   // write 1
                 }
                 else /*if(count >= 30)*/
									 data &= ~(1UL << (24-i));  // write 0;
								 
                	while (!(HAL_GPIO_ReadPin (GPIOA, GPIO_PIN_0)) && count_2 < 150) // wait for pin to go low..
									{
											count_2++;
										Delay_us(10);
									}
									//sprintf(msg, "\ncount=%d",count_2);
									//HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
      }
			//lock =0;
		  data_rf_remote = data;
			return 0;
		}
		return 1;
}




