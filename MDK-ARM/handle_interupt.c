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

extern event event_interupt; 
extern status_t status;
extern UART_HandleTypeDef huart1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern uint8_t interupt_timer_3;
extern uint8_t timer_standby;

void handel_no_event(void)
{
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
	}
	if(timer_standby == 3)
	{
		HAL_UART_Transmit(&huart1, (uint8_t *)"TIME-OUT-enter standby", sizeof("TIME-OUT-enter standby"), HAL_MAX_DELAY);
		Enter_Standby_Mode();	
	}
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
			
			case GPIO_PIN_15:	//PLAY  
				event_interupt = PLAY_EVENT;
				HAL_UART_Transmit(&huart1, (uint8_t *)"interrupt-play_press = 1       ", 31, HAL_MAX_DELAY);				
			break;
			
			case GPIO_PIN_11:	//RECORD
				event_interupt = RECORD_EVENT;
				HAL_UART_Transmit(&huart1, (uint8_t *)"interrupt-record_press = 1     ", 31, HAL_MAX_DELAY);							
			break;
			
			case GPIO_PIN_0:	//IR -> RF        			
				if(!encode_rf(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)))
				{
					uint32_t data_rf = get_result_rf();
					HAL_UART_Transmit(&huart1, (uint8_t *)"---------------------", 21, HAL_MAX_DELAY);
					char msg[59];
					sprintf(msg, "\n data_ir = %x",data_rf );
					HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
					if((data_rf == 0xff02fd))	// can doc log xem data la gi?
					{
						HAL_UART_Transmit(&huart1, (uint8_t *)"get_result_ir", 14, HAL_MAX_DELAY);
						data_rf = 0;
						event_interupt = RF_EVENT;
					}
				}				
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
	

	char msg[59];
	sprintf(msg, "\n time_standy = %d, ",time_standy);
	HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);

	if(time_standy == TIME_GO_TO_STANDBY)
	{
		HAL_UART_Transmit(&huart1, (uint8_t *)"Time out- Enter standby   ", 26, HAL_MAX_DELAY);		
		Enter_Standby_Mode();
	}
	//event_interupt = NO_EVENT;
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
	
		//HAL_UART_Transmit(&huart1, (uint8_t *)"DangLHB- enter standymode      ", sizeof("DangLHB- enter standymode      "), HAL_MAX_DELAY);		

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

/*
//Ham xu li khi co su kien bam nut Back
void Handle_Back_Button_Event(void)
{
	if(status != RECORDING)
	{
		HAL_UART_Transmit(&huart1, (uint8_t *)"Handle_Back_Button_Event - !is_record        ", 45, HAL_MAX_DELAY);
		if(status == PLAYING)
		{
			HAL_UART_Transmit(&huart1, (uint8_t *)"Handle_Back_Button_Event - enter Stop_play        ", 50, HAL_MAX_DELAY);
			Stop_play();
		}		
		if(stt_play > 1 && full_record == 0)		//truong hop neu da ghi am duoc tren 1 ban ghi am thi se thuc hien chon ban ghi am dang truoc.
			stt_play -= 1;
		else if(full_record == 1)	// truong hop da ghi am duoc full 5 ban. chon ban ghi am dang truoc.
			stt_play -= 1;
		else if(stt_play == 1 && pin_control > 0 && full_record == 0)		// truong hop da ghi am duoc it nhat 1 ban nhung chua full, thuc hien chon theo kieu quay vong.
			stt_play = pin_control; 
		
		char msg[51];
		sprintf(msg, "\n full_record = %d, stt_play =%d, pin_control = %d",full_record,stt_play,pin_control );
		HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
		if(stt_play == 0 && full_record == 1)			// truong hop khi da ghi am full va thuc hien chon theo kieu quay vong.
			stt_play = 5;
		if(stt_play > 0)
		Start_play();
		time_default = 0;
		time_standy = 0;
		//event_interupt = 0;
		//HAL_Delay(1000);		
		//event_interupt = NO_EVENT;		
	} 
			//back_press =0;
	//write_flash(&stt_play, STT_PLAY_T, (sizeof(stt_play)/4));	// Luu gia tri vao flash
	HAL_Delay(500);	
	event_interupt = NO_EVENT;
}
*/
/*
//Ham xu li nut LOOP_RANDOM_RANDOM
uint8_t g_set_status_loop_random = 1;
void Handle_loop_random_Button_Event(void)
{
	if(status == RECORDING)
	{
		Stop_record();
		//event_interupt = NO_EVENT;
	}
	if(status == PLAYING)
	{
			Stop_play();
			//event_interupt = NO_EVENT;
	}
	if(g_set_status_loop_random == 0)
	{
		g_status_loop = LOOP;
//		if(stt_play !=0)
//		{														//Set lai so stt_play
//			stt_play +=1;
//		}
		g_set_status_loop_random = 1;
	}
	else if(g_set_status_loop_random == 1)
	{
		g_status_loop = RANDOM;
		g_set_status_loop_random = 0;
	}
	event_interupt = NO_EVENT;
}
*/
/*
void Hanlde_IR_event(void)
{
	g_IR_allow_play = 1;
	if(status == RECORDING)
	{
		Stop_record();
	}
	if(status == PLAYING)
	{
			Stop_play();
	}
	// call handle_no_event() to play record with no delay
	handel_no_event();	
	//event_interupt = NO_EVENT;
}
*/
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
			HAL_UART_Transmit(&huart1, (uint8_t *)"RECORD_EVENT", 12, HAL_MAX_DELAY);
			Handle_Record_Button_Event();
		break;
//		case BACK_EVENT:
//			HAL_UART_Transmit(&huart1, (uint8_t *)"BACK_EVENT", 10, HAL_MAX_DELAY);
//			Handle_Back_Button_Event();
//		break;
//		case LOOP_RANDOM_EVENT:
//			HAL_UART_Transmit(&huart1, (uint8_t *)"LOOP_RANDOM_EVENT", 17, HAL_MAX_DELAY);
//			Handle_loop_random_Button_Event();
//		break;
		case PLAY_EVENT:
			HAL_UART_Transmit(&huart1, (uint8_t *)"PLAY_EVENT", 10, HAL_MAX_DELAY);
			Handle_Play_Button_Event();
		break;
//		case IR_EVENT:
//			HAL_UART_Transmit(&huart1, (uint8_t *)"IR_EVENT", 8, HAL_MAX_DELAY);
			//HAL_Delay(100);
		
//			if(stt_play !=0)
//				stt_play = pin_control +1; 
//			Hanlde_IR_event();
//			HAL_Delay(1500);
//			event_interupt = NO_EVENT;
			//Enter_Standby_Mode();
			
//		break;
//		case POWER_OFF:
//			Enter_Standby_Mode();
//			break;
		default:
		break;
	}
}