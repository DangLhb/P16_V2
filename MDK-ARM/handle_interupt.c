//file handle_interupt.c
//DangLHb

#include "main.h"
#include "ic_audio.h"
#include "flash_memory.h"
#include "string.h"
#include "stdio.h"
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include "handle_interupt.h"
#include "ir_remote.h"
#include "danglhb_rf.h"


#define MAX_TIME_RECORD 60
#define TIME_GO_TO_STANDBY 600
#define TIME_GO_TO_SLEEP 2
uint16_t time_default = 0, time_standy = 0;
uint32_t pin_control = 0, stt_play = 0, full_record = 0;
//status_loop g_status_loop = LOOP;
//uint8_t g_IR_allow_play = 0;
//uint8_t g_count_loop = 0;
uint8_t g_count_time_blink_led = 0;

extern event event_interupt; 
extern status_t status;
extern UART_HandleTypeDef huart1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern uint8_t interupt_timer_3;
extern uint8_t timer_standby;
extern uint8_t g_allow_toggle_led;
extern uint8_t lock;

void handel_no_event(void)
{
	//if(g_allow_toggle_led)
	//{
		//HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_12);
	//}
	//else HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_RESET);
	
	
		char msg[59];
	if(status == STOPRECORD || status == STOPPLAY)
	{
		HAL_TIM_Base_Stop_IT(&htim3);
		timer_standby = 0;
		HAL_TIM_Base_Start_IT(&htim3);
		status = NONE;	
	}
	if(status == RECORDING || status == PLAYING)
	{
		HAL_TIM_Base_Stop_IT(&htim3);
		timer_standby = 0;
			//if(g_allow_toggle_led)
		//HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_12);
		//HAL_Delay(1000);
	}
	if(status == NONE)
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_11,GPIO_PIN_RESET);		//standby cordex
	if(timer_standby == 5)
	{
				HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_RESET);	//tat nguon rf
		//HAL_UART_Transmit(&huart1, (uint8_t *)"TIME-OUT-enter standby", sizeof("TIME-OUT-enter standby"), HAL_MAX_DELAY);
		Enter_Standby_Mode();	
	}
	//Blink_Led();
}


void HAL_GPIO_EXTI_Callback(uint16_t button)
{

	if(event_interupt == NO_EVENT)
	{		
		switch(button)
		{
//			case GPIO_PIN_12:	//BACK
//				event_interupt = LOOP_RANDOM_EVENT;			// DangLHb modify
//				HAL_UART_Transmit(&huart1, (uint8_t *)"interrupt-loop_back_press = 1       ", 36, HAL_MAX_DELAY);
//			break;
			case GPIO_PIN_0:	//IR -> RF
				if(!encode_rf(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)))
				{
					uint32_t data_rf = get_result_rf();
					char msg[59];
					sprintf(msg, "\n data_rf = %x",data_rf );
					HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
					if(data_rf == 0x1c5f35e || data_rf == 0x1dc5f35 || data_rf == 0x18be6bd || data_rf == 0x15ee2f9 || data_rf == 0x1dc5f35 || data_rf == 0x1000f15 || data_rf == 0x1c4801a || data_rf == 0xc50dbd || data_rf == 0x16286de)	// can doc log xem data la gi? - TH khac 3ffffff
					{
						event_interupt = RF_EVENT;
						data_rf = 0;
						lock = 0;
					}
				}
				timer_standby = 0;
				//else lock = 0;
				//HAL_UART_Transmit(&huart1, (uint8_t *)"(-.-)", sizeof("(-.-)"), HAL_MAX_DELAY);
			break;
			
			case GPIO_PIN_15:	//PLAY  
				event_interupt = PLAY_EVENT;
				HAL_GPIO_WritePin(GPIOB,GPIO_PIN_11,GPIO_PIN_SET);	
				HAL_UART_Transmit(&huart1, (uint8_t *)"interrupt-play_press = 1       ", 31, HAL_MAX_DELAY);				
			break;
			
			case GPIO_PIN_11:	//RECORD
				event_interupt = RECORD_EVENT;
				HAL_GPIO_WritePin(GPIOB,GPIO_PIN_11,GPIO_PIN_SET);		
				HAL_UART_Transmit(&huart1, (uint8_t *)"interrupt-record_press = 1     ", 31, HAL_MAX_DELAY);							
			break;
			

	//		case GPIO_PIN_4:
	//			HAL_UART_Transmit(&huart1, (uint8_t *)"button- poweroff", sizeof("button- poweroff"), HAL_MAX_DELAY);	
	//			event_interupt = POWER_OFF;
	//		break;				
			default:
			break;		
		}			
	}
	char msg[59];
	sprintf(msg, "\n event_interupt = %d, status =%d",event_interupt,status);
	HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
}


//Ham xu li khi co ngat timer
void Handle_Timer_2_Interupt(void)
{
		//HAL_UART_Transmit(&huart1, (uint8_t *)"hi", 2, HAL_MAX_DELAY);

	uint32_t time_record = stt_play_to_time_record(stt_play) - 2;
	if(time_default < MAX_TIME_RECORD)
		time_default  +=1;
	if(time_standy < TIME_GO_TO_STANDBY)
		time_standy +=1;
	if(time_default == MAX_TIME_RECORD && ( status == RECORDING))
	{
		HAL_UART_Transmit(&huart1, (uint8_t *)"Time out- stop record     ", 26, HAL_MAX_DELAY);		
		//Stop_record();
		//event_interupt = NO_EVENT;
		event_interupt = RECORD_EVENT;
	}
	if((time_default == time_record) && (status == PLAYING))
	{
		HAL_UART_Transmit(&huart1, (uint8_t *)"Time out- stop play     ", 26, HAL_MAX_DELAY);		
		//Stop_play();
		event_interupt = PLAY_EVENT;
	}
	//if(timer_standby == 4)
		//lock = 0;

	char msg[59];
	sprintf(msg, "\n time_standy = %d, ",time_standy);
	HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);

	//if(time_standy == TIME_GO_TO_STANDBY)
	//{
		//HAL_UART_Transmit(&huart1, (uint8_t *)"Time out- Enter standby   ", 26, HAL_MAX_DELAY);		
		//Enter_Standby_Mode();
	//}
	//event_interupt = NO_EVENT;
}

//
void Handle_Timer_4_Interupt(void)
{
	if(status == RECORDING)
	{
		if(g_count_time_blink_led == 0)
		{
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12, GPIO_PIN_RESET);
			g_count_time_blink_led ++;
		}
		else {
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12, GPIO_PIN_SET);
			g_count_time_blink_led ++;
			if(g_count_time_blink_led > 5)
				g_count_time_blink_led =0;
		}
	}
	else if(status == NONE)
	{
		if(g_count_time_blink_led == 0)
		{
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12, GPIO_PIN_RESET);
			g_count_time_blink_led ++;
		}
		else {
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12, GPIO_PIN_SET);
			g_count_time_blink_led ++;
			if(g_count_time_blink_led > 25)
				g_count_time_blink_led =0;
		}
	}
}
//Ham Enter Sleep mode
void StanbyMode(void)
{
	//MX_GPIO_Deinit();
	__HAL_RCC_PWR_CLK_ENABLE();
	HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN1);
	__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
	HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);
	HAL_PWR_EnterSTANDBYMode();
}


// ham goi vao che do standby
void Enter_Standby_Mode(void)
{

	if(status == RECORDING)
		Stop_record();
	if(status == PLAYING)
		Stop_play();
	//lock_interupt = 0;
	HAL_TIM_Base_Stop_IT(&htim2);
	HAL_TIM_Base_Stop_IT(&htim3);
	HAL_Delay(200);
	event_interupt = NO_EVENT;
	

		//goi ham vao che do standy.
		__HAL_RCC_PWR_CLK_ENABLE();
   if(__HAL_PWR_GET_FLAG(PWR_FLAG_SB))
	 {
		 __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU|PWR_FLAG_SB);
	 } 
	//sprintf(msg, "\nENTER standby mode");
	//HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), 100);
	StanbyMode();	
	//}
}


//Ham xu li khi co su kien bam nut RECORD
void Handle_Record_Button_Event(void)
{
	if(status == PLAYING)
	{
		HAL_UART_Transmit(&huart1, (uint8_t *)"Handle_Record_Button_Event - enter stop play      ", 50, HAL_MAX_DELAY);
		Stop_play();
	}
	if(status != RECORDING)
	{
		HAL_UART_Transmit(&huart1, (uint8_t *)"Handle_Record_Button_Event - enter start record   ", 50, HAL_MAX_DELAY);
		Start_record();
		event_interupt = NO_EVENT;
	}
	else
	{
		HAL_UART_Transmit(&huart1, (uint8_t *)"Handle_Record_Button_Event - enter stop record    ", 50, HAL_MAX_DELAY);
		Stop_record();
		event_interupt = NO_EVENT;			
	}
}


//Ham xu li khi co su kien bam nut Play
void Handle_Play_Button_Event(void)
{
		HAL_UART_Transmit(&huart1, (uint8_t *)"Handle_Play_Button_Event - stt_play     ", 40, HAL_MAX_DELAY);
		if(status == RECORDING)
		{
			HAL_UART_Transmit(&huart1, (uint8_t *)"Handle_Play_Button_Event - enter Stop_record      ", 50, HAL_MAX_DELAY);			
			Stop_record();
		}
		if(status != PLAYING)
		{
			HAL_UART_Transmit(&huart1, (uint8_t *)"Handle_Play_Button_Event - enter Start_play       ", 50, HAL_MAX_DELAY);			
			Start_play();
			event_interupt = NO_EVENT;
		}			
		else
			Stop_play();
			event_interupt = NO_EVENT;
}

void handle_event(event even_t)
{
	//HAL_UART_Transmit(&huart1, (uint8_t *)"handle_event", 12, HAL_MAX_DELAY);
	switch(even_t)
	{
		case NO_EVENT:
		// check status
		// enter sleep
			handel_no_event();
		break;
    case RECORD_EVENT:
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_11,GPIO_PIN_SET);			
			HAL_UART_Transmit(&huart1, (uint8_t *)"RECORD_EVENT", 12, HAL_MAX_DELAY);
			Handle_Record_Button_Event();
		break;
		case PLAY_EVENT:
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_11,GPIO_PIN_SET);
			HAL_UART_Transmit(&huart1, (uint8_t *)"PLAY_EVENT", 10, HAL_MAX_DELAY);
			Handle_Play_Button_Event();
		break;
		case RF_EVENT:
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_11,GPIO_PIN_SET);			
			HAL_UART_Transmit(&huart1, (uint8_t *)"handele RF_EVENT", sizeof("handele RF_EVENT"), HAL_MAX_DELAY);
			Handle_Play_Button_Event();
			//HAL_Delay(1500);
			event_interupt = NO_EVENT;
			break;
		case DANGLHB:
			HAL_Delay(1000);
			event_interupt = NO_EVENT;
			break;
			
		default:
		break;
	}
}