/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "adc.h"
#include "eth.h"
#include "fdcan.h"
#include "i2c.h"
#include "ltdc.h"
#include "quadspi.h"
#include "rng.h"
#include "rtc.h"
#include "sai.h"
#include "sdmmc.h"
#include "spi.h"
#include "usart.h"
#include "usb_otg.h"
#include "gpio.h"
#include "fmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32h750b_discovery_lcd.h"
#include "stm32_lcd.h"
#include "stm32h750b_discovery.h"
#include "stm32h750b_discovery_sdram.h"
#include "task.h"

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

/* USER CODE BEGIN PV */

//vrsta za sporočila med opravili
osMessageQueueId_t buttonQueueHandle;

//opravilo senzorja
osThreadId_t buttonTaskHandle;
const osThreadAttr_t buttonTask_attributes = {
		.name = "ButtonTask",
		.stack_size = 512 * 4,
		.priority = (osPriority_t) osPriorityNormal,

};

//opravilo za zaslon
osThreadId_t lcdTaskHandle;
const osThreadAttr_t lcdTask_attributes = {
		.name = "LCDTask",
		.stack_size = 4096 * 4,
		.priority = (osPriority_t) osPriorityBelowNormal,
};

void StartButtonTask(void *argument);
void StartLCDTask(void *argument);

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MPU_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName);
uint8_t DrawSemafor(void);
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

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_ADC3_Init();
  MX_ETH_Init();
  MX_FDCAN1_Init();
  MX_FDCAN2_Init();
  //MX_FMC_Init();
  //MX_LTDC_Init();
  MX_QUADSPI_Init();
  MX_RTC_Init();
  MX_SAI2_Init();
  //MX_SDMMC1_MMC_Init();
  MX_SPI2_Init();
  MX_USART3_UART_Init();
  MX_USB_OTG_FS_PCD_Init();
  MX_I2C4_Init();
  MX_RNG_Init();
  /* USER CODE BEGIN 2 */

  //Inicializacija SDRAMA
  /*if (BSP_SDRAM_Init(0) != BSP_ERROR_NONE)
  {
      //Če SDRAM ni pravilno inicializiran se ugasne zelena ledica
      HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET);
  }*/
  BSP_SDRAM_Init(0);
  BSP_LCD_Init(0, LCD_ORIENTATION_LANDSCAPE);
  UTIL_LCD_SetFuncDriver(&LCD_Driver);
  BSP_LCD_SetLayerVisible(0, 0, ENABLE);
  UTIL_LCD_SetLayer(0);


  //Inicializacija LCD
  /*if(BSP_LCD_Init(0, LCD_ORIENTATION_LANDSCAPE) == BSP_ERROR_NONE)
  {
  	   UTIL_LCD_SetFuncDriver(&LCD_Driver);
  	   BSP_LCD_SetLayerVisible(0, 0, ENABLE);
  	   UTIL_LCD_SetLayer(0);

  	   //Vklop napajanja in osvetlitve (za vsak slučaj)
  	   HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_SET);
  	   HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);

  	   // Enostaven preizkus piskela / ozadja
  	   UTIL_LCD_Clear(UTIL_LCD_COLOR_BLUE);
  } else {
	   //Če ne deluje LCD inicializacija se ugasne rdeča ledica
	   HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
  }*/

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();  /* Call init function for freertos objects (in cmsis_os2.c) */

  buttonQueueHandle = osMessageQueueNew(4, sizeof(uint32_t), NULL);
  if (buttonQueueHandle == NULL) { Error_Handler(); }

  buttonTaskHandle = osThreadNew(StartButtonTask, NULL, &buttonTask_attributes);
  if (buttonTaskHandle == NULL) { Error_Handler(); }

  lcdTaskHandle = osThreadNew(StartLCDTask, NULL, &lcdTask_attributes);
  if (lcdTaskHandle == NULL) { Error_Handler(); }

  MX_FREERTOS_Init();


  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_HSI
                              |RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 2;
  RCC_OscInitStruct.PLL.PLLN = 12;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 3;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOMEDIUM;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV1;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInitStruct.PLL2.PLL2M = 2;
  PeriphClkInitStruct.PLL2.PLL2N = 12;
  PeriphClkInitStruct.PLL2.PLL2P = 2;
  PeriphClkInitStruct.PLL2.PLL2Q = 2;
  PeriphClkInitStruct.PLL2.PLL2R = 2;
  PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_3;
  PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOMEDIUM;
  PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
  PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLL2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
//opravilo za modri gumb
void StartButtonTask(void *argument)
{
	//inicializacija začetnega stanja - onemogočimo zmago z držanjem gumba
	GPIO_PinState previousState = GPIO_PIN_RESET;

	for(;;)
	{
		//trenutno stanje gumba
		GPIO_PinState currentState = HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin);
		if(currentState == GPIO_PIN_SET && previousState == GPIO_PIN_RESET){
			//trenutek/tick pošljemo v vrsto, kjer ga bo lahko bralo drugo opravilo
			uint32_t pressTick = HAL_GetTick();
			osMessageQueuePut(buttonQueueHandle, &pressTick, 0U, 0U);
		}

		//ponastavimo prejšnje stanje in počakamo za debounce
		previousState = currentState;
		osDelay(5);
	}
}

void StartLCDTask(void *argument)
{
	uint32_t press;
	osStatus_t status;

	for(;;)
	{
		osMessageQueueReset(buttonQueueHandle);
		uint8_t noPress = DrawSemafor();
		if(noPress == 1)
		{
			uint32_t randomValue;
			HAL_RNG_GenerateRandomNumber(&hrng, &randomValue);
			uint32_t waitTime = 500 + (randomValue % 4000);

			status = osMessageQueueGet(buttonQueueHandle, &press, NULL, waitTime);
			if(status == osErrorTimeout)
			{
				//ugasnemo krogce
				uint32_t goTick = HAL_GetTick();
				UTIL_LCD_Clear(UTIL_LCD_COLOR_WHITE);
				status = osMessageQueueGet(buttonQueueHandle, &press, NULL, osWaitForever);

				uint32_t reactionTime = press - goTick;
				UTIL_LCD_SetBackColor(UTIL_LCD_COLOR_WHITE);
				UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_BLACK);
				UTIL_LCD_SetFont(&Font24);
				char buffer[30];
				sprintf(buffer, "Reaction time: %lu ms", reactionTime);
				UTIL_LCD_DisplayStringAt(0, 120, (uint8_t *)buffer, CENTER_MODE);
				osDelay(3000);
			} else {
				//prižgemo rdečo luč
				HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
				UTIL_LCD_Clear(UTIL_LCD_COLOR_WHITE);
				UTIL_LCD_SetBackColor(UTIL_LCD_COLOR_WHITE);
				UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_RED);
				UTIL_LCD_SetFont(&Font24);
				UTIL_LCD_DisplayStringAt(0, 120, (uint8_t *)"TOO FAST!", CENTER_MODE);
				osDelay(3000);
				HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
			}

		} else {
			HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_RESET);
			UTIL_LCD_Clear(UTIL_LCD_COLOR_WHITE);
			UTIL_LCD_SetBackColor(UTIL_LCD_COLOR_WHITE);
			UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_RED);
			UTIL_LCD_SetFont(&Font24);
			UTIL_LCD_DisplayStringAt(0, 120, (uint8_t *)"WAIT FOR ALL LIGHTS!", CENTER_MODE);
			osDelay(3000);
			HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET);
		}



	}

}

uint8_t DrawSemafor(void)
{
	uint16_t startPositionX = 110;
	uint16_t startPositionY = 131;

	uint16_t radius = 25;
	uint16_t spacing = 65;

	uint32_t press;

	UTIL_LCD_Clear(UTIL_LCD_COLOR_WHITE);

	for(uint16_t i = 0; i < 5; i++)
	{
		UTIL_LCD_FillCircle((startPositionX + (i*spacing)), startPositionY, radius, UTIL_LCD_COLOR_RED);
		if(i == 4) return 1;

		osStatus_t status = osMessageQueueGet(buttonQueueHandle, &press, NULL, 1000);
		if(status == osOK)
		{
			return 0;
		}
	}
	return 1;
}


//Preverja stackOverflow
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    __disable_irq();
    for(;;) {
    	//Če ne deluje LCD inicializacija se ugasne rdeča ledica
    	HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
    }
}


/* USER CODE END 4 */

 /* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0xC7;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
