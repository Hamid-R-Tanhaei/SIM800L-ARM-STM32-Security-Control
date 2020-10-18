/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  * @Description : A minimal security/monitoring/controlling project with SIM800L GSM module
  * @Author : Hamid Reza Tanhaei
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usbd_customhid.h"
#include <string.h>
#include "stdio.h"
unsigned char E[]={10},G[]={34},S[]={26},C[]={13};
//#define Enter    HAL_UART_Transmit(&huart3,(unsigned char*)E,1,1000);
#define CR       HAL_UART_Transmit(&huart3,(unsigned char*)C,1,1000);
//#define GIM      HAL_UART_Transmit(&huart3,(unsigned char*)G,1,1000);
#define SUB      HAL_UART_Transmit(&huart3,(unsigned char*)S,1,1000);
/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart3;
/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART3_UART_Init(void);
//
extern USBD_HandleTypeDef hUsbDeviceFS;
uint8_t USB_RX_Buffer[64];
uint8_t USB_TX_Buffer[64]; //to send USB data to PC
volatile uint8_t Flag_Rcvd_Data_HID = 0;
//void	Delay_us(uint32_t counter);
uint16_t Flag_end_Transaction;
uint8_t host_preamble[10] = 		{'S', 'I', 'M', '8', '0', '0', 'H', 'o', 's', 't'};
uint8_t device_preamble_OK[10] = 	{'S', 'I', 'M', '8', '0', '0', 'D', 'e', 'O', 'K'};
uint8_t device_preamble_Error[10] = {'S', 'I', 'M', '8', '0', '0', 'D', 'e', 'E', 'r'};
uint16_t HostMsgVerified;
uint16_t Index = 0;
uint16_t k = 0;
uint16_t verifier = 0;
uint16_t index_checker[100];
uint16_t index_verifier = 0;
char tmp_str[66]={'\0'};
char send_str[33]={'\0'};
char recv_str[129]={'\0'};
char io_str[257]={'\0'};
char Num_str[11]={"9124264721"};
char cmd_str[3]={'\0'};
char unread_str[7]={'\0'};
char read_str[5]={'\0'};
char	Bat_pre[9] = {'\0'};
char	bat_chr[3] = {'\0'};
char	bat_chr3[4] = {'\0'};
char	sending_sms_str[65] = {'\0'};
char	bat_rep[17] = {'\0'};
uint8_t NewSMS = 0;
uint8_t Armed = 0;
uint8_t Del_flag = 0;
uint8_t Number_verified = 0;
uint8_t New_status = 0;
uint16_t 	Del_counter = 0;
uint16_t	Bat_chk_counter = 0;
uint8_t Bat_val = 0;
uint8_t	SIM_failed_response = 0;
uint8_t BatLowVal = 31;
uint8_t Bat_check = 0;
uint8_t	sending_sms_cmd = 0;
uint8_t	calling_cmd = 0;
uint16_t	SIM_count_down = 0;
uint16_t	test_key_cntr = 0;
uint8_t	In_trg = 0;
uint8_t	inPwr_enable = 0;
uint8_t	inPwr_damp_cntr = 0;
uint8_t	OnSw_enable = 0;
uint8_t	OnSw_damp_cntr = 0;
uint8_t	knock_enable = 0;
uint8_t	knock_timeout = 0;
uint8_t	knock_damp_cntr = 0;
uint8_t	PID_enable = 0;
uint8_t	PID_timeout = 0;
uint8_t	PID_damp_cntr = 0;
uint8_t report_status = 0;
////////////////
void	Initialize_SIM800(void){
	//
	HAL_Delay(10000);
	tmp_str[0] = '\0';
	sprintf(tmp_str,"AT+CMGF=1\r"); // Make SMS mode into text mode
	HAL_UART_Transmit(&huart3,(unsigned char *)tmp_str,strlen(tmp_str),1000);
	//
	HAL_Delay(2000);
	tmp_str[0] = '\0';
	sprintf(tmp_str,"AT+CFUN=1\r"); // make module ready to call and sms
	HAL_UART_Transmit(&huart3,(unsigned char *)tmp_str,strlen(tmp_str),1000);
	//
	HAL_Delay(2000);
	tmp_str[0] = '\0';
	sprintf(tmp_str,"AT+IPR=115200\r"); // set sim800 uart baudrate to 115200
	HAL_UART_Transmit(&huart3,(unsigned char *)tmp_str,strlen(tmp_str),1000);
	//
	HAL_Delay(2000);
	tmp_str[0] = '\0';
	sprintf(tmp_str,"ATE0\r"); // turn at command echo off
	HAL_UART_Transmit(&huart3,(unsigned char *)tmp_str,strlen(tmp_str),1000);
	//
	HAL_Delay(2000);
	tmp_str[0] = '\0';
	sprintf(tmp_str,"AT+CMGDA=\"DEL ALL\"\r"); // Delete all previous messages
	HAL_UART_Transmit(&huart3,(unsigned char *)tmp_str,strlen(tmp_str),1000);
	HAL_Delay(5000);
}
//////////
void	Checking_inputs(void){
	//
	LED1_GPIO_Port -> BSRR = LED1_Pin;
	if (knock_timeout > 0){
		knock_timeout--;
	}
	if (PID_timeout > 0){
		PID_timeout--;
	}
	//
	for (k = 0; k < 30; k++){
		//
		if((KEY0_GPIO_Port->IDR & KEY0_Pin) == (uint32_t)GPIO_PIN_RESET)
		{
			test_key_cntr++;
			if (test_key_cntr > 3){
				sprintf(sending_sms_str,"Key pressed \nBat: %d%%", Bat_val);
				sending_sms_cmd = 1;
				calling_cmd = 1;
				test_key_cntr = 0;
			}
		}
		else{
			test_key_cntr = 0;
		}
		//
		//if(((*InPwr_GPIO_Port).IDR & InPwr_Pin) == (uint32_t)GPIO_PIN_RESET)
		if((InPwr_GPIO_Port->IDR & InPwr_Pin) == (uint32_t)GPIO_PIN_RESET)
		{
			if (inPwr_enable == 1){
				inPwr_damp_cntr++;
				if (inPwr_damp_cntr > 100){
					//if (Armed == 1){
						strcpy(sending_sms_str, "Power went off");
						sending_sms_cmd = 1;
						calling_cmd = 1;
					//}
					inPwr_damp_cntr = 0;
					inPwr_enable = 0;
				}
			}
		}
		else{
			inPwr_enable = 1;
			inPwr_damp_cntr = 0;
		}
		//
		if (HAL_GPIO_ReadPin(OnSw_GPIO_Port, OnSw_Pin) == GPIO_PIN_SET)
		//if((OnSw_GPIO_Port->IDR & OnSw_Pin) == (uint32_t)GPIO_PIN_SET)
		{
			if (OnSw_enable == 1){
				OnSw_damp_cntr++;
				if (OnSw_damp_cntr > 20){
					if (Armed == 1){
						strcpy(sending_sms_str,"On-Switch ON");
						sending_sms_cmd = 1;
						calling_cmd = 1;
					}
					OnSw_damp_cntr = 0;
					OnSw_enable = 0;
				}
			}
		}
		else{
			OnSw_enable = 1;
			OnSw_damp_cntr = 0;
		}
		//
		//
		if (HAL_GPIO_ReadPin(OnSw_GPIO_Port, OnSw_Pin) == GPIO_PIN_RESET)
		{
			if (HAL_GPIO_ReadPin(PID_GPIO_Port, PID_Pin) == GPIO_PIN_SET){
				LED2_GPIO_Port -> BSRR = LED2_Pin;
				if ((PID_enable == 1) && (PID_timeout == 0)){
					PID_damp_cntr++;
					if (PID_damp_cntr > 5){
						if (Armed == 1){
							strcpy(sending_sms_str,"Motion sensor triggered");
							sending_sms_cmd = 1;
							calling_cmd = 1;
						}

						PID_damp_cntr = 0;
						PID_timeout = 20;
						PID_enable = 0;
					}
				}

			}
			else{
				LED2_GPIO_Port -> BRR = LED2_Pin;
				PID_enable = 1;
				PID_damp_cntr = 0;
			}

			if((knock_GPIO_Port->IDR & knock_Pin) == (uint32_t)GPIO_PIN_RESET)
			{
				if ((knock_enable == 1) && (knock_timeout == 0)){
					knock_damp_cntr++;
					if (knock_damp_cntr > 5){
						if (Armed == 1){
							strcpy(sending_sms_str,"knock sensor triggered");
							sending_sms_cmd = 1;
							calling_cmd = 1;
						}

						knock_damp_cntr = 0;
						knock_timeout = 50;
						knock_enable = 0;
					}
				}
			}
			else{
				knock_enable = 1;
				knock_damp_cntr = 0;
			}
		}
		//

		HAL_Delay(50);
	}
	//return ret_value;
	LED1_GPIO_Port -> BRR = LED1_Pin;
}
//
void	sending_sms_func(void){
	LED2_GPIO_Port -> BSRR = LED2_Pin;
	send_str[0] = '\0';
	sprintf(send_str,"AT+CSMP=17,167,0,0\r");
	HAL_UART_Transmit(&huart3,(unsigned char *)send_str,strlen(send_str),1000);
	HAL_Delay(300);
	send_str[0] = '\0';
	sprintf(send_str,"AT+CMGS=\"+98%s\"\r",Num_str);
	HAL_UART_Transmit(&huart3,(unsigned char *)send_str,strlen(send_str),1000);
	HAL_Delay(300);
	HAL_UART_Transmit(&huart3,(unsigned char *)sending_sms_str,strlen(sending_sms_str),1000);
	SUB
	HAL_Delay(1000);
	LED2_GPIO_Port -> BRR = LED2_Pin;
}
//
void	calling_func(void){
	HAL_Delay(5000);
	send_str[0] = '\0';
	sprintf(send_str,"ATD+98%s;\r",Num_str);
	HAL_UART_Transmit(&huart3,(unsigned char*)send_str,strlen(send_str),1000);

	HAL_Delay(35000);
	send_str[0] = '\0';
	sprintf(send_str,"ATH\r");
	HAL_UART_Transmit(&huart3,(unsigned char*)send_str,strlen(send_str),1000);
	HAL_Delay(1000);
}
//
void	check_for_incoming_sms(void){
	send_str[0] = '\0';
	for (k = 0; k < 128; k++)
	{recv_str[k] = '\0';}
	sprintf(send_str,"AT+CMGR=1\r");
	HAL_UART_Transmit(&huart3,(unsigned char *)send_str,strlen(send_str),1000);
	HAL_UART_Receive(&huart3,(unsigned char *)recv_str, 128, 100);
	NewSMS = 0;
	New_status = 0;
	unread_str[0] = '\0';
	read_str[0] = '\0';
	cmd_str[0] = '\0';
	for (k = 0; k < 30; k++){
		unread_str[0] = recv_str[k];
		unread_str[1] = recv_str[k+1];
		unread_str[2] = recv_str[k+2];
		unread_str[3] = recv_str[k+3];
		unread_str[4] = recv_str[k+4];
		unread_str[5] = recv_str[k+5];
		if ((unread_str[0] == 'U') && (unread_str[1] == 'N') && (unread_str[2] == 'R') && (unread_str[3] == 'E') && (unread_str[4] == 'A') && (unread_str[5] == 'D')){
			NewSMS = 1;
			break;
		}
	}
	if (NewSMS == 0){
		for (k = 0; k < 30; k++){
			read_str[0] = recv_str[k];
			read_str[1] = recv_str[k+1];
			read_str[2] = recv_str[k+2];
			read_str[3] = recv_str[k+3];
			if ((read_str[0] == 'R') && (read_str[1] == 'E') && (read_str[2] == 'A') && (read_str[3] == 'D')){
					Del_flag = 1;
				break;
			}
		}
	}
//
	if (NewSMS == 1){
		Del_flag = 1;
		Number_verified = 0;
		for (k = 0; k < 30; k++){
			//strncpy(Num_str,recv_str+k,10);
			//if (strcmp(Num_str,"912xxxxxxx") == 0){
			if ((recv_str[k] == Num_str[0]) && (recv_str[k+1] == Num_str[1]) && (recv_str[k+2] == Num_str[2]) && (recv_str[k+3] == Num_str[3]) && (recv_str[k+4] == Num_str[4]) && (recv_str[k+5] == Num_str[5]) && (recv_str[k+6] == Num_str[6]) && (recv_str[k+7] == Num_str[7]) && (recv_str[k+8] == Num_str[8]) && (recv_str[k+9] == Num_str[9])){
				Number_verified = 1;
				break;
			}
		}
		report_status = 0;
		if (Number_verified == 1){
			for (k = 50; k < 80; k++){
				//strncpy(cmd_str,recv_str+k,2);
				cmd_str[0] = recv_str[k];
				cmd_str[1] = recv_str[k+1];
				//if (strcmp(strlwr(cmd_str),"on") == 0){
				if ((cmd_str[0] == 'O') || (cmd_str[0] == 'o')){
					if ((cmd_str[1] == 'N') || (cmd_str[1] == 'n')){
						Armed = 1;
						New_status = 1;
						break;
					}
				}
				//if (strcmp(strlwr(cmd_str),"of") == 0){
				if ((cmd_str[0] == 'O') || (cmd_str[0] == 'o')){
					if ((cmd_str[1] == 'F') || (cmd_str[1] == 'f')){
						Armed = 0;
						New_status = 1;
						break;
					}
				}
				//if (strcmp(strlwr(cmd_str),"st") == 0){
				if ((cmd_str[0] == 'S') || (cmd_str[0] == 's')){
					if ((cmd_str[1] == 'T') || (cmd_str[1] == 't')){
						New_status = 1;
						report_status = 1;
						break;
					}
				}
				//
				if ((cmd_str[0] == 'R') || (cmd_str[0] == 'r')){
					if ((cmd_str[1] == 'S') || (cmd_str[1] == 's')){
						NVIC_SystemReset();
						break;
					}
				}

			}
		}
	}
	NewSMS = 0;
	if (New_status == 1){
		LED2_GPIO_Port -> BSRR = LED2_Pin;
		New_status = 0;
		sending_sms_cmd = 1;
		sending_sms_str[0] = '\0';
		if (Armed == 1){
			strcpy(sending_sms_str,"Status: On");
		}
		else{
			strcpy(sending_sms_str,"Status: Off");
		}
		if (report_status == 1){
			if((InPwr_GPIO_Port->IDR & InPwr_Pin) == (uint32_t)GPIO_PIN_RESET){
				strcat(sending_sms_str,"\nInPwr: Off");
			}else{
				strcat(sending_sms_str,"\nInPwr: On");
			}
			if (HAL_GPIO_ReadPin(OnSw_GPIO_Port, OnSw_Pin) == GPIO_PIN_SET){
				strcat(sending_sms_str,"\nIn1: On");
			}else{
				strcat(sending_sms_str,"\nIn1: Off");
			}
			sprintf(bat_rep,"\nBat: %d%%", Bat_val);
			strcat(sending_sms_str,bat_rep);
		}
		HAL_Delay(10);
		LED2_GPIO_Port -> BRR = LED2_Pin;
	}
}
////////////////////
void	Bat_check_func(void){
	tmp_str[0] = '\0';
	strcpy(tmp_str,"AT+CBC\r");
	for(k=0; k<128; k++){io_str[k] = '\0';}
	HAL_UART_Transmit(&huart3,(unsigned char *)tmp_str,strlen(tmp_str),1000);
	HAL_UART_Receive(&huart3,(unsigned char *)io_str, 128, 200);
	SIM_failed_response = 1;
	Bat_pre[0] = '\0';
	bat_chr[0] = '\0';
	bat_chr3[0] = '\0';
	Bat_val = 0;
	for (k = 0; k < 30; k++){
		Bat_pre[0] = '\0';
		strncpy(Bat_pre,io_str+k,8);
		if (strcmp(Bat_pre,"+CBC: 0,") == 0){
			SIM_failed_response = 0;
			bat_chr3[0] = io_str[k+8];
			bat_chr3[1] = io_str[k+9];
			bat_chr3[2] = io_str[k+10];
			//strncpy(bat_chr3,io_str+k+8,3);
			//if (strcmp(bat_chr3, "100") == 0){
			if ((bat_chr3[0] == '1') && (bat_chr3[1] == '0') && (bat_chr3[2] == '0')){
				Bat_val = 100;
			}
			else{
				bat_chr[0] = io_str[k+8];
				bat_chr[1] = io_str[k+9];
				//strncpy(bat_chr,io_str+k+8,2);
				Bat_val = atoi(bat_chr);
			}
			if (Bat_val > 31){
				BatLowVal = 31;
			}
			if (Bat_val < BatLowVal){
				if (BatLowVal > 10){
					BatLowVal = BatLowVal - 10;
					sending_sms_cmd = 1;
					sending_sms_str[0] = '\0';
					sprintf(sending_sms_str,"Battery Low: %d%%", Bat_val);
				}
			}
		}
	}
	if ((Bat_val < 3) || (SIM_failed_response == 1)){
		NVIC_SystemReset();
	}
}

//////////////////////
void	Del_all_sms(void){
	LED2_GPIO_Port -> BSRR = LED2_Pin;
	HAL_Delay(200);
	send_str[0] = '\0';
	sprintf(send_str,"AT+CMGDA=\"DEL ALL\"\r");
	HAL_UART_Transmit(&huart3,(unsigned char *)send_str,strlen(send_str),1000);
	HAL_Delay(2000);
	LED2_GPIO_Port -> BRR = LED2_Pin;
}
//////////////////////
int main(void)
{
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();
	SystemClock_Config();
	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_USART3_UART_Init();
	MX_USB_DEVICE_Init();
	HAL_Delay(10);
	Flag_end_Transaction = 0;
	Flag_Rcvd_Data_HID = 0;
	Index = 0;
	//k = 0;
	verifier = 0;
	HostMsgVerified = 0;
	HAL_GPIO_WritePin(SIM_RST_GPIO_Port, SIM_RST_Pin, GPIO_PIN_RESET);
	HAL_Delay(2000);
	HAL_GPIO_WritePin(SIM_RST_GPIO_Port, SIM_RST_Pin, GPIO_PIN_SET);
	Initialize_SIM800();
//
	HAL_Delay(2000);
	Bat_check_func();
	sending_sms_cmd = 1;
	sending_sms_str[0] = '\0';
	sprintf(sending_sms_str,"Power up \nBat: %d%%", Bat_val);

	while (1)
  {
		if (Flag_Rcvd_Data_HID == 1)
		{
			Flag_Rcvd_Data_HID = 0;
			for (int i = 0; i < 64; i++){
				tmp_str[i] = USB_RX_Buffer[i];
			}
			for (k=0; k<128; k++){io_str[k] = '\0';}
			HAL_UART_Transmit(&huart3,(unsigned char *)tmp_str,strlen(tmp_str),2000);
			CR
			HAL_UART_Receive(&huart3,(unsigned char *)io_str, 128, 2000);
			for (uint8_t i = 0; i < 64; i++)
			{
				USB_TX_Buffer[i] = io_str[i]; // USB_RX_Buffer[i] + 5;
				//USB_TX_Buffer[i] = tmp_str[i]; // USB_RX_Buffer[i] + 5;
			}
			USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS,USB_TX_Buffer,64);
			HAL_Delay(100);
		}

		else{
			Checking_inputs();
			check_for_incoming_sms();
			if (sending_sms_cmd == 1){
				Checking_inputs();
				sending_sms_func();
				sending_sms_cmd = 0;
			}
			if (calling_cmd == 1){
				calling_func();
				calling_cmd = 0;
			}

			//
			Del_counter++;
			if (Del_counter > 100){
				Del_counter = 0;
				Del_flag = 1;
			}
			Bat_chk_counter++;
			if (Bat_chk_counter > 130){
				Bat_chk_counter = 0;
				Bat_check = 1;
			}
			//
			if (Bat_check == 1){
				HAL_Delay(1000);
				Bat_check_func();
				Bat_check = 0;
			}
			/////////////////
			if (Del_flag == 1){
				Checking_inputs();
				Del_all_sms();
				Del_flag = 0;
			}
		}
  } // endof While(1)
return 0;
}// endof Main
/////////////////////////////////////////////////////////////////
/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;	//RCC_HSE_BYPASS; //RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SIM_RST_GPIO_Port, SIM_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : PC13 PC14 PC15 */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 PA1 PA2 PA3 
                           PA4 PA5 PA6 PA7 
                           PA8 PA9 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3 
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7 
                          |GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  //
  /*Configure GPIO pin : LED2_Pin */
    GPIO_InitStruct.Pin = LED2_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB1 PB2 PB12 
                           PB14 PB6
                           PB7 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_12 
                          |GPIO_PIN_14|GPIO_PIN_6
                          |GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  //
  /*Configure GPIO pin : LED1_Pin */
    GPIO_InitStruct.Pin = LED1_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED1_GPIO_Port, &GPIO_InitStruct);


  /*Configure GPIO pin : SIM_RST_Pin */
  GPIO_InitStruct.Pin = SIM_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SIM_RST_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : KEY0_Pin */
  GPIO_InitStruct.Pin = KEY0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(KEY0_GPIO_Port, &GPIO_InitStruct);

  //
  HAL_GPIO_WritePin(SIM_RST_GPIO_Port, SIM_RST_Pin, GPIO_PIN_RESET);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
