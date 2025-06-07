/**IoT input pedal
 * The aim of this Project is to implement a IoT press control based on STM32 and ESP8266 microcontrollers.
 * In this project the following peripherals are implemented:
 * 1.Timer
 * 2.RTC
 * 3.GPIO
 * 4.DMA
 * 5.UART
 * In this prject first time synced by NTP from ESP8266 after that, press apply by external trigger
 * and detect by exti interrupt and finally, the result is sent from the STM32 to the ESP8266 for sending to the server.
 */

#include "stm32f103xb.h"
#include "main.h"
#include "string.h"
#include "stdbool.h"
#include "stdio.h"
#include <stdlib.h>
#include "time.h"
#include "pre_def.h"

RTC_HandleTypeDef hrtc;
TIM_HandleTypeDef htim2;
UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_tx;
DMA_HandleTypeDef hdma_usart1_rx;
RTC_TimeTypeDef sTime = {0};
RTC_DateTypeDef sDate = {0};


typedef enum{
  UART_State_IDLE,
  UART_State_Tx,
  UART_State_Rx,
  UART_State_process
}State_machine_t;
State_machine_t uart_state;

uint32_t epoch_time;
uint16_t check = 0;
bool interrupt_flag = false;
bool one_time_flag = true;

bool timer_flag = false;

bool receive_flag = true;
bool transmit_flag = false;

uint32_t current_time = 0;
uint32_t debounce = 200;
uint8_t RTC_buffer[30];
uint8_t receive_data[18];
uint8_t receive_data2[10];
const uint8_t responce_massage[13] = {"successfully\n"};
char *inputs;
int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_RTC_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  while (1)
  {
    handle_NTP_time();
    if (interrupt_routine() && one_time_flag)
    {
      get_time();
      get_date();
      buff_UART_send();
      one_time_flag = false;
    }
  }
}

// void State_Machine_init(void){
//   switch (uart_state)
//   {
//   case UART_State_IDLE:
//     uart_state = UART_State_Tx;
//     break;
//   case UART_State_Tx:
//     /* code */
//     break;
//   case UART_State_Rx:
//     /* code */
//     break;
//   case UART_State_process:
//     /* code */
//     break;
//   }
// }

/**
 * function for handle input serial
 * ability to detect "epoch:" as header and
 * find epoch time.
 */
void handle_NTP_time(void){

  if (HAL_UARTEx_ReceiveToIdle_DMA(&huart1, receive_data, sizeof(receive_data)) == HAL_OK && strncmp((char*)receive_data,"epoch:",6) == 0)
  {
    receive_flag = false;
    transmit_flag = true;
    memcpy(receive_data2,&receive_data[6],10);
    epoch_time = strtoul((char*)receive_data2,NULL, 10);
    epoch_to_timedate(epoch_time);
    HAL_UART_Transmit_DMA(&huart1,responce_massage,sizeof(responce_massage));
    // HAL_TIM_Base_Start_IT(&htim2);
    memset(receive_data, 0, sizeof(receive_data));
  }  
}
  
/**
 * GPIO and exti interrupt
 */
void led_on(void){
  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,GPIO_PIN_RESET);
}
void led_off(void){
  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,GPIO_PIN_SET);
}
void led_toggle(void){
  HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_10);
}
//handle interrupt routine and software debouncing
bool interrupt_routine(void){
  current_time = interrupt_flag?current_time:HAL_GetTick();
  interrupt_flag = (HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_5) == GPIO_PIN_SET)?false:true;
  return (HAL_GetTick()-current_time >= debounce && interrupt_flag);
}
//interrupt callback
void HAL_GPIO_EXTI_Callback(uint16_t input_callback){
  if (input_callback==GPIO_PIN_5 && !interrupt_flag)
  {
    interrupt_flag = true;
    one_time_flag = true;
  }  
}
//set pin for exti1 irqhandler
void EXTI9_5_IRQHandler(void){
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_5);
}
//peripheral initial
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET);

  /*Configure GPIO pin : PB10 */
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PA1 */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}

/**
 * UART configuration
 */
//Changing data format and sending via uart DMA
void buff_UART_send(void)
{
  sprintf((char*)RTC_buffer,"Time:%02d:%02d:%02d Date:%02d-%02d-%02d\n"
  ,sTime.Hours,sTime.Minutes,sTime.Seconds,sDate.Year,sDate.Month,sDate.Date);
  if (HAL_UART_Transmit_DMA(&huart1,RTC_buffer, strlen((char*)RTC_buffer)) != HAL_OK) {
        // Error_Handler();
    }
}
//uart successful reseive callback
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
  if (huart->Instance == USART1)
  {
  }
}
//uart successful send callback
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance==USART1)
  {
  }
}
//peripheral initial
static void MX_USART1_UART_Init(void)
{
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
}

/** 
 * DMA configuration
 */
void DMA1_Channel5_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_usart1_rx);
}


//peripheral initial
static void MX_DMA_Init(void)
{

  __HAL_RCC_DMA1_CLK_ENABLE();

  hdma_usart1_rx.Instance = DMA1_Channel5;
  hdma_usart1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
  hdma_usart1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_usart1_rx.Init.MemInc = DMA_MINC_ENABLE;
  hdma_usart1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_usart1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
  hdma_usart1_rx.Init.Mode = DMA_NORMAL;
  hdma_usart1_rx.Init.Priority = DMA_PRIORITY_LOW;
  HAL_DMA_Init(&hdma_usart1_rx);
  __HAL_LINKDMA(&huart1, hdmarx, hdma_usart1_rx);

  hdma_usart1_tx.Instance = DMA1_Channel4;
  hdma_usart1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
  hdma_usart1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_usart1_tx.Init.MemInc = DMA_MINC_ENABLE;
  hdma_usart1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_usart1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
  hdma_usart1_tx.Init.Mode = DMA_NORMAL;
  hdma_usart1_tx.Init.Priority = DMA_PRIORITY_LOW;
  HAL_DMA_Init(&hdma_usart1_tx);
  __HAL_LINKDMA(&huart1, hdmatx, hdma_usart1_tx);

  HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);
  HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);

}

/**
 * RTC configuration
 */
void epoch_to_timedate(uint32_t epoch){
  time_t rawtime = epoch;
  struct tm * ti;
  ti = localtime(&rawtime);
  RTC_time_init(ti->tm_hour,ti->tm_min,ti->tm_sec,ti->tm_year,ti->tm_mon,ti->tm_mday);
}
//get last time
void get_time(void){
  if(HAL_RTC_GetTime(&hrtc,&sTime,RTC_FORMAT_BIN) !=HAL_OK){Error_Handler();}
}
//get last date
void get_date(void){
  if(HAL_RTC_GetDate(&hrtc,&sDate,RTC_FORMAT_BIN) !=HAL_OK){Error_Handler();}
}
//apply epoch to RTC
void RTC_time_init(uint8_t hour, uint8_t min, uint8_t sec, uint16_t year,uint8_t mon, uint8_t day){
  sTime.Hours = hour;
  sTime.Minutes = min;
  sTime.Seconds = sec;
  HAL_RTC_SetTime(&hrtc,&sTime,RTC_FORMAT_BIN);

  sDate.Year = year + 0x76c;
  // check = year + 0x76c;
  sDate.Month = mon + 1;
  sDate.Date = day;
  HAL_RTC_SetDate(&hrtc,&sDate,RTC_FORMAT_BIN);
}
//peripheral initial
static void MX_RTC_Init(void)
{
  hrtc.Instance = RTC;
  hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
  hrtc.Init.OutPut = RTC_OUTPUTSOURCE_ALARM;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
}


/**
 * Timer 2 
 */
//end of timer operation
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
  if (htim->Instance == TIM2)
  {
    led_off();
    HAL_TIM_Base_Stop_IT(&htim2);
  }
}
//peripheral initial
static void MX_TIM2_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 18000;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 1999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_UPDATE);

  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
 * RRC configuration
 */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
 * error handler
 */
void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif
