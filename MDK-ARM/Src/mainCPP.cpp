/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "usbd_cdc_if.h"
#include "tm_stm32_nrf24l01.h"
#include <a_nRF24L01.h>
#include "a_RF24.h"
#include "stdio.h"

//#define CDC_LOG
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;
SPI_HandleTypeDef hspi1;
UART_HandleTypeDef huart4;

/* USER CODE BEGIN PV */
uint8_t print_buffer[100] ,nrf_receive [100] , radio_PayLoadData[35];
uint8_t radio_channel ,radio_PAlevel, radio_DataRate, radio_crcLength, radio_PayLoadSize;
//const uint8_t tx_address[6] = "00001";
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_UART4_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */
void make_command(uint8_t * REG, uint8_t cmd , uint8_t state);
void R_Register (uint8_t addr[], uint8_t byte_to_read ,uint8_t * readed_bytes);
const char text[] = "Hello\nThis is a test";
void Blink_LED(uint16_t Pin, uint32_t Delay);
void print_CDC(const char * str);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
	
int main(void)
{
  /* USER CODE BEGIN 1 */
	
  /* USER CODE END 1 */
 

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_UART4_Init();
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	
	#define tranacting_number 2
	const uint8_t address[6] = "00001";
	//	const uint8_t rxaddress[6] = "00001";
	//uint8_t nrf_status = 50, transmision_status  = 50; // not 0 , 01 , ff
	//uint8_t flag = 1;
	
	//while(!HAL_GPIO_ReadPin(BLUE_PB_GPIO_Port ,BLUE_PB_Pin));
	RF24 radio(NRF24L01_CE_PIN, NRF24L01_CSN_PIN);
	while(!radio.begin())
		Blink_LED(LED_RED_Pin, 200);
	radio.maskIRQ(IRQ_TX_OK_DIS ,IRQ_TX_FAIL_DIS, IRQ_RX_READY_EN);
	radio.openWritingPipe(address);
	radio.openReadingPipe(0, address);
	radio.setPALevel(RF24_PA_LOW);
	radio.setDataRate(RF24_1MBPS);
	radio.setCRCLength(RF24_CRC_16);
  radio.setChannel(10);
	radio.stopListening();
	
	
//  radio_channel = radio.getChannel();
//  radio_crcLength = radio.getCRCLength();
//  radio_PayLoadSize = radio.getPayloadSize();
//  radio_PAlevel = radio.getPALevel();
//  radio.read(radio_PayLoadData, 32);
//  sprintf((char *)print_buffer, "channel: 0x%02x \nCRC Length: 0x%02x \nPay Load Size: 0x%02x \nPA level: 0x%02x \nData Rate: 0x%02x \n Pay Load data: ",
//  radio_channel, radio_crcLength, radio_PayLoadSize, radio_PAlevel, radio_DataRate);
// // CDC_Transmit_FS(print_buffer, strlen((const char *)print_buffer));
//  CDC_Transmit_FS(print_buffer, 32);

	while(!HAL_GPIO_ReadPin(BLUE_PB_GPIO_Port ,BLUE_PB_Pin));
	Blink_LED(LED_ORANGE_Pin, 100);
	
	while (1)
  {
		uint32_t  ms = HAL_GetTick();
		while(!HAL_GPIO_ReadPin(BLUE_PB_GPIO_Port ,BLUE_PB_Pin) && (ms + 5000 > HAL_GetTick() ));
		
		//HAL_GPIO_WritePin(LED_ORANGE_GPIO_Port, LED_ORANGE_Pin, HAL_GPIO_ReadPin(INT_GPIO_Port, INT_Pin));
		if(radio.write(&text, 20))
		{
			radio.startListening();
			Blink_LED(LED_GREEN_Pin, 50);
			if(radio.available() /*&& (!HAL_GPIO_ReadPin(INT_GPIO_Port, INT_Pin))*/)
			{
				radio.read(nrf_receive , 32);
				print_CDC((const char *)nrf_receive);
				print_CDC("\n");
				Blink_LED(LED_BLUE_Pin, 100);
			}
			radio.stopListening();
		}
		else
		{
			Blink_LED(LED_RED_Pin, 50);
		}
		
	
		
		
		
		HAL_Delay(500);
			/* USER CODE END WHILE */
		/* USER CODE BEGIN 3 */
		
	}
	/* USER CODE END 3 */
}



/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
   RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 96;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV6;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{
  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256; // minimum speed
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief UART4 Initialization Function
  * @param None
  * @retval None
  */
/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 1000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

static void MX_UART4_Init(void)
{

  /* USER CODE BEGIN UART4_Init 0 */

  /* USER CODE END UART4_Init 0 */

  /* USER CODE BEGIN UART4_Init 1 */

  /* USER CODE END UART4_Init 1 */
  huart4.Instance = UART4;
  huart4.Init.BaudRate = 9600;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART4_Init 2 */

  /* USER CODE END UART4_Init 2 */

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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(CSel_GPIO_Port, CSel_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4|INT_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(TxRx_GPIO_Port, TxRx_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, LED_GREEN_Pin|LED_ORANGE_Pin|LED_RED_Pin|LED_BLUE_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : BLUE_PB_Pin */
  GPIO_InitStruct.Pin = BLUE_PB_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BLUE_PB_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : CSel_Pin */
  GPIO_InitStruct.Pin = CSel_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(CSel_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : INT_Pin */
  GPIO_InitStruct.Pin = INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : TxRx_Pin */
  GPIO_InitStruct.Pin = TxRx_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(TxRx_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LED_GREEN_Pin LED_ORANGE_Pin LED_RED_Pin LED_BLUE_Pin */
  GPIO_InitStruct.Pin = LED_GREEN_Pin|LED_ORANGE_Pin|LED_RED_Pin|LED_BLUE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
	
}

/* USER CODE BEGIN 4 */

/**
  * @brief  Print through CDC (Virtual COM port).
  * @retval None
*/
void print_CDC(const char * str)
{
	uint8_t buf[100];
	sprintf((char *)buf, str);
	CDC_Transmit_FS((uint8_t *)buf, strlen((char *)buf));
}

void print_CDC(const char * str, uint8_t var1)
{
	uint8_t buf[100];
	sprintf((char *)buf, str, var1);
	CDC_Transmit_FS((uint8_t *)buf, strlen((char *)buf));
}

void print_CDC(const char * str, uint8_t var1, uint8_t var2)
{
	uint8_t buf[100];
	sprintf((char *)buf, str, var1, var2);
	CDC_Transmit_FS((uint8_t *)buf, strlen((char *)buf));
}

/**
  * @brief  This function bliks the specified led within passed delay.
  * @retval None
*/
void Blink_LED(uint16_t Pin, uint32_t Delay)
{
	HAL_GPIO_WritePin(GPIOD, Pin, GPIO_PIN_SET); // GPIOD is LEDs port
	HAL_Delay(Delay);
	HAL_GPIO_WritePin(GPIOD, Pin, GPIO_PIN_RESET); // GPIOD is LEDs port
	HAL_Delay(Delay);
}

/* USER CODE END 4 */
/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
//	while(1)
//	{
//		HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET);
//		HAL_Delay(500);
//		HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET);
//		HAL_Delay(500);
//	}
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
