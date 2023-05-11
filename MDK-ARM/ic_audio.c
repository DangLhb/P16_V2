//file ic_audio.c
//DangLHb

#include "main.h"
#include "ic_audio.h"
#include "flash_memory.h"
#include "string.h"
#include "stdio.h"
#include <stdint.h>

extern UART_HandleTypeDef huart1;

extern uint8_t timer_interupt;
extern uint16_t time_default, time_standy;
extern uint8_t is_reset;
extern uint8_t lock_interupt;
extern uint32_t pin_control, stt_play, full_record;
uint8_t g_random_count = 0;
uint8_t c_random_cacula[100] = {3,2,1,5,1,1,4,2,5,2,4,1,1,2,3,2,4,3,5,1,4,2,
	3,5,1,2,3,1,4,3,2,1,5,1,1,4,2,5,2,4,1,1,2,5,1,2,3,1,4,3,2,1,2,3,2,4,3,5,1,4,2};
uint32_t time_record_1 = 0, time_record_2 = 0, time_record_3 = 0, time_record_4 = 0, time_record_5 = 0;

status_t status = NONE;
//ham convert pin_control -> PORTX và GPIO_PIN_Y
void Convert_pin_control_or_stt_play_to_pin(uint8_t pin,uint8_t setorreset)
{
	pin_control_t control_pin = pin;
	GPIO_TypeDef *GPIOX;
	uint16_t GPIO_Pin;
	switch (control_pin)
	{
		case M0:
			GPIOX = GPIOA;
			GPIO_Pin = GPIO_PIN_3;
			HAL_UART_Transmit(&huart1, (uint8_t *)"Control - M0 = ", 15, HAL_MAX_DELAY);
			break;
		case M1:
			GPIOX = GPIOA;
			GPIO_Pin = GPIO_PIN_4;
			HAL_UART_Transmit(&huart1, (uint8_t *)"Control - M1 = ", 15, HAL_MAX_DELAY);		
			break;
		case M2:
			GPIOX = GPIOA;
			GPIO_Pin = GPIO_PIN_5;
			HAL_UART_Transmit(&huart1, (uint8_t *)"Control - M2 = ", 15, HAL_MAX_DELAY);
			break;
		case M3:
			GPIOX = GPIOA;
			GPIO_Pin = GPIO_PIN_6;
			HAL_UART_Transmit(&huart1, (uint8_t *)"Control - M3 = ", 15, HAL_MAX_DELAY);
			break;
		case M4:
			GPIOX = GPIOA;
			GPIO_Pin = GPIO_PIN_7;
			HAL_UART_Transmit(&huart1, (uint8_t *)"Control - M4 = ", 15, HAL_MAX_DELAY);
			break;
		default:
			return;
		break;
	}
	if(setorreset == 1)
	{
		HAL_GPIO_WritePin(GPIOX,GPIO_Pin,GPIO_PIN_SET);
		HAL_UART_Transmit(&huart1, (uint8_t *)"1     ", 6, HAL_MAX_DELAY);
	}
	else if(setorreset == 0)
	{
		HAL_GPIO_WritePin(GPIOX,GPIO_Pin,GPIO_PIN_RESET);
		HAL_UART_Transmit(&huart1, (uint8_t *)"0     ", 6, HAL_MAX_DELAY);
	}
}

//ham convert pin_control =>time_record.
void convert_pin_control_to_time_record(uint32_t pin_control_t)
{
	switch(pin_control_t)
	{
		case M0:
			time_record_1 = time_default;
			write_flash(&time_record_1, TIME_RECORD_1, (sizeof(time_record_1)/4));
			break;
		case M1:
			time_record_2 = time_default;
			write_flash(&time_record_2, TIME_RECORD_2, (sizeof(time_record_2)/4));
			break;
		case M2:
			time_record_3 = time_default;
			write_flash(&time_record_3, TIME_RECORD_3, (sizeof(time_record_2)/4));
			break;
		case M3:
			time_record_4 = time_default;
			write_flash(&time_record_4, TIME_RECORD_4, (sizeof(time_record_2)/4));
			break;
		case M4:
			time_record_5 = time_default;
			write_flash(&time_record_5, TIME_RECORD_5, (sizeof(time_record_2)/4));
			break;
		default:
			break;
	}

	char msg[135];
	sprintf(msg, "\n pin_control_t = %d, time_record_1 =%d, time_record_2 =%d, time_record_3 =%d, time_record_4 =%d, time_record_5 =%d ,time_default = %d",pin_control_t,time_record_1,time_record_2,time_record_3,time_record_4,time_record_5, time_default );
	HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
}
uint32_t stt_play_to_time_record (uint32_t stt_play_t)
{
		uint32_t save_time =0;
		switch(stt_play_t)
		{
			case 1:
				save_time = time_record_1;
				break;
			case 2:
				save_time = time_record_2;
				break;
			case 3:
				save_time = time_record_3;
				break;
			case 4:
				save_time = time_record_4;
				break;
			case 5:
				save_time = time_record_5;
				break;			
		}
		return save_time;
}

//ham start record
void Start_record(void)
{
			HAL_UART_Transmit(&huart1, (uint8_t *)"Enter Start_record   ", 21, HAL_MAX_DELAY);
			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1, GPIO_PIN_RESET);	// REC = 0 - switch to record mode
			HAL_UART_Transmit(&huart1, (uint8_t *)"Start_record - REC = 0   ", 25, HAL_MAX_DELAY);
			set_record_to_record(read_switch_mode());		//set pin_control
			Convert_pin_control_or_stt_play_to_pin(pin_control,0);
			time_default = 0;
			time_standy = 0;
			status = RECORDING;
			HAL_Delay(1000);
}

//ham stop record
void Stop_record(void)
{
			//uint32_t read =0;
			Convert_pin_control_or_stt_play_to_pin(pin_control,1);
			convert_pin_control_to_time_record(pin_control);
			stt_play = pin_control + 1;
			if(stt_play == 5)
			{
				//stt_play = 1;
				full_record =1;
			}
			HAL_UART_Transmit(&huart1, (uint8_t *)"Check_log", 9, HAL_MAX_DELAY);		
			pin_control +=1;
			if(pin_control == 5)
				pin_control = 0;
			
			status = STOPRECORD;
			time_default =0;
			time_standy = 0;
			HAL_Delay(10);		
			write_flash(&pin_control, PIN_CONTROL_T, (sizeof(pin_control)/4));
			HAL_Delay(10);
			write_flash(&full_record, FULL_RECORD, (sizeof(full_record)/4));
			HAL_Delay(10);
			write_flash(&stt_play, STT_PLAY_T, (sizeof(stt_play)/4));
			char msg[80];
			sprintf(msg, "\n stt_play = %d, pin_control =%d, full_record = %d",stt_play,pin_control,full_record);
			HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
}

//ham start play
void Start_play(void)
{
	HAL_UART_Transmit(&huart1, (uint8_t *)"Enter Start_play - REC =1     ", 30, HAL_MAX_DELAY);
	set_record_to_play(read_switch_mode());		//set stt_play
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1, GPIO_PIN_SET);	// REC = 1
	HAL_Delay(100);
	Convert_pin_control_or_stt_play_to_pin(stt_play - 1,0);
	HAL_Delay(100);
	Convert_pin_control_or_stt_play_to_pin(stt_play - 1,1);
	
	time_default = 0;
	time_standy = 0;
	status = PLAYING;
}


//ham dung play
void Stop_play(void)
{
	HAL_UART_Transmit(&huart1, (uint8_t *)"Enter Stop_play      ", 22, HAL_MAX_DELAY);	
	Convert_pin_control_or_stt_play_to_pin(stt_play - 1,0);
	HAL_Delay(10);
  Convert_pin_control_or_stt_play_to_pin(stt_play - 1,1);
	status = STOPPLAY;
	time_default = 0;
	time_standy = 0;
	HAL_Delay(10);	
}

//doc trang thai thanh switch
uint32_t read_switch_mode(void)
{
	//HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5
	if(!HAL_GPIO_ReadPin (GPIOB, GPIO_PIN_4))
		return 1;
	else if(!HAL_GPIO_ReadPin (GPIOB, GPIO_PIN_5))
		return 2;
	else if(!HAL_GPIO_ReadPin (GPIOB, GPIO_PIN_6))
		return 3;
	else if(!HAL_GPIO_ReadPin (GPIOB, GPIO_PIN_7))
		return 4;
	else if(!HAL_GPIO_ReadPin (GPIOB, GPIO_PIN_8))
		return 5;
	else if(!HAL_GPIO_ReadPin (GPIOB, GPIO_PIN_9))
		return 6;
	else return 7;
}

//set stt_play tu thanh switch
void set_record_to_play(uint32_t switch_mode)
{
	//char msg[59];
	switch(switch_mode)
	{
		case 1:
			stt_play = 1;
		break;
		case 2:
			stt_play = 2;
		break;
		case 3:
			stt_play = 3;
		break;
		case 4:
			stt_play = 4;
		break;
		case 5:
			stt_play = 5;
		break;
		case 6:
		// Neu o mode random thì sao? thì stt play se bang so random, . 
		//Van de la phai biet da ghi am duoc ban nao de random vao nhung ban da ghi am.Khi play thi se kiem tra time record cua no co > 0 k?, neu k thi tu chuyen ban ghi am khac.
			do 
			{
				stt_play = c_random_cacula[g_random_count];
			}
			while(stt_play_to_time_record(stt_play) == 0);	// check time_record de xem co ton tai ban ghi am hay k? Neu k thi thuc hien set lait stt play
		break;
		case 7:
			// khong doc duoc trang thai cua switch
		break;
		default:
		break;
	}
}
//set pin_control tu thanh switch
void set_record_to_record(uint32_t switch_mode)
{
	switch(switch_mode)
	{
		case 1:
			pin_control = 0;
		break;
		case 2:
			pin_control = 1;
		break;
		case 3:
			pin_control = 2;
		break;
		case 4:
			pin_control = 3;
		break;
		case 5:
			pin_control = 4;
		break;
		case 6:
			// can hoi lai kich ban de xu li
		break;
		case 7:
		break;
		default:
		break;
	}	
}