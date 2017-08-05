/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_hal.h"
#include "string.h" 


extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim5;
extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim7;
extern TIM_HandleTypeDef htim9;
extern ADC_HandleTypeDef hadc1;
extern UART_HandleTypeDef huart2;
extern GPIO_InitTypeDef GPIO_InitStruct;


extern void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

                                

/* Private function prototypes -----------------------------------------------*/
               
     
/* Buffer used for transmission */
uint8_t aTxStartMessage[] = "O\n";
uint8_t aTxMessage_Reset[] = "R\n";

/* Buffer used for reception */

uint8_t Start_key[RXBUFFERSIZE] = "c123456789_23456";
uint8_t aRxBuffer[RXBUFFERSIZE]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	
	
char flag_start=0;

	/* Variable used to get converted value */
__IO uint16_t uhADCxConvertedValue = 0;

	
char buf[16];

//для ПИ регулятора
//float Kp=0.55, Ti=200, Td=0.5, Tc=0.3; // для низкой скорости
float Kp=0.15, Ti=18, Td=3, Tc=0.3; // для высокой скорости


float b0=0, b1=0, b2=0;
float xz_1, xz_2, yz_1;
//levo 110 vpravo 180    normal 155
//speed vpered 109 nazad 197  normal 155

int speed=0;
	
int speed_in=0;			// -100... 0 ... 100  %
int povorot_in=0;    // -100... 0 ... 100  %
	int rasch_sto=100;
	int rasch_des=10;
	int rasch_od=1;
	
	
//available to the rest of the code
//speeds
volatile int16_t speed_encoder=0;
volatile int8_t napravlenie=0;

//distances
volatile int32_t Total_encoder_up=0;
volatile int32_t Total_encoder_down=0;

// local variables

volatile uint16_t Encoder=0;
volatile uint16_t Encoderz_1=0;
volatile uint16_t oldEncoder=0;

volatile int16_t encoderDiff=0;
volatile int16_t encoderDiffz_1=0;

char j_5=0;// для таймеров
char j_6=0;
char i_rx=0;
unsigned int j_7=0;	 
	/* Captured Value */
__IO uint32_t            uwIC2Value = 0;
/* Duty Cycle Value */
__IO uint32_t            uwDutyCycle = 0;
/* Frequency Value */
__IO uint32_t            uwFrequency = 0;
	
	char flag_regima1=0;
	
	char MX_PWM_TIM1_init=0;
	
/* USER CODE END 0 */

int main(void)
{


  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_TIM3_Init();	// асинхронный режим для энкодера
  MX_TIM1_Init();	// управления движками режим PWM_output
  MX_TIM2_Init(); // для ПИ регулятора скорости 
  MX_TIM6_Init(); // для контроля ручного управления Basic_IT
	MX_TIM5_Init(); // для регулировки скорости Basic_IT
	MX_TIM9_Init(); // для ультразвукового датчика PWM
	MX_TIM7_Init(); // для ультразвукового датчика отмерять 1ms
	MX_ADC1_Init();
  /* USER CODE BEGIN 2 */

	  /* Configure LED1 and LED2 */
  BSP_LED_Init(LED1);
  BSP_LED_Init(LED2);
	BSP_LED_Init(LED3);
  BSP_LED_Init(LED4);
	 

	BSP_LED_On(LED3);
		
	lcd_init();
	lcd_clear();
	
	
	
	
	
	
	
	connect_raspberry();
	
		
	flag_start=1;	

		
	TIM1->CCR2 = 155; // vpered nazad
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);   // vpered nazad	
	TIM1->CCR1 = 155; //povorot
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1); //povorot
	MX_PWM_TIM1_init=1; //нужно чтобы понять когда мы включаем ручное управление
	HAL_TIM_Base_Start_IT(&htim6);
	

	encodersstart();	
	
	// Для датчика приближения
	TIM9->CCR2 = 50;    //100 нету мощности   0 полная мощность
	HAL_TIM_PWM_Stop(&htim9, TIM_CHANNEL_2);  
	//HAL_TIM_Base_Start_IT(&htim7);
	
	// Меряем угол поворота	
  if(HAL_ADC_Start_IT(&hadc1) != HAL_OK)
  {
    /* Start Conversation Error */
    Error_Handler();
  }	
		
	//========Запускаем ПИ регулятор скорости======================
	
	xz_2=xz_1=0;
	yz_1=0;	
	
	b0=Kp*(1+(Td/Tc));
	b1=Kp*((Tc/Ti)-1-2*(Td/Tc));
	b2=Kp*(Td/Tc);
	HAL_TIM_Base_Start_IT(&htim2);
	//=========Запускаем ПИ регулятор==============================
	
	BSP_LED_Off(LED3);
 
//	HAL_Delay(1000);
		
			
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {



		//======================LCD function=================
			if(flag_regima1==0) 		//автоматическое управление
			{

				sprintf(buf,"povorot_in=%d     ",  povorot_in);
				lcd_SetPosition(0, 0);
				lcd_String(buf);
				
				sprintf(buf,"speed_in=%d       ", 		 speed_in);
				lcd_SetPosition(0, 1);
				lcd_String(buf);
			}
			
			
				if(flag_regima1==1)    //ручное управление
			{
				sprintf(buf,"speed=%d        ",		speed);
				lcd_SetPosition(0, 0);
				lcd_String(buf);
				
				sprintf(buf,"encoder=%d       ", 		 speed_encoder);
				lcd_SetPosition(0, 1);
				lcd_String(buf);
			}			
			//======================end_LCD function=================
			

			//HAL_Delay(300);

  }
}


void  HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	
	 //=========================== ЧИТАЕМ ЗНАЧЕНИЯ С ДАТЧИКА УГЛА И ИЭНКОДЕРА===================
	 if(htim==&htim5)
	 {
	
				/*##-3- Start the conversion process and enable interrupt ##################*/  
			if(HAL_ADC_Start_IT(&hadc1) != HAL_OK)
			{
				/* Start Conversation Error */
				Error_Handler();
			}		
			
			if(j_5==2){ encodersRead(); j_5=0;}			
			j_5++;
	 }
	 
	 //=========================== ОПРЕДЕЛЯЕМ РУЧНОЕ УПРАВЛЕНИЕ===================
	 if(htim==&htim6)   
	 {
			if(j_6==2)
			{
				flag_regima1=0;
					if(MX_PWM_TIM1_init==0)  // включаем PWM
					{
							MX_PWM_TIM1_init=1;
														
							MX_TIM1_Init();
							
							TIM1->CCR2 = 155; // vpered nazad  
							TIM1->CCR1 = 155; //povorot
							
							HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);   // vpered nazad
							HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1); //povorot
							
					}	

				j_6=0;	
			}				
				
			j_6++;						
	 }
	 
	 //=========================== ВЫДАЁМ ИМПУЛЬС НА ДАТЧИК ПРИБЛЕЖЕНИЯ===================
	 if(htim==&htim7) 
	 {
			if(j_7==1)
			{
				HAL_TIM_PWM_Start(&htim9, TIM_CHANNEL_2);
			}else if(j_7==3)
			{
				HAL_TIM_PWM_Stop(&htim9, TIM_CHANNEL_2); 
			}else if (j_7==4000){j_7=0;}
			
			j_7++;		
	
	 }
	 
	 //=========================== РЕГУЛИРУЕМ СКОРОСТЬ И ПОВОРОТ===================
		 if(htim==&htim2)
	 {
			  BSP_LED_Toggle(LED4);
			//speed=PIDcomtroller(speed_in - speed_encoder);
			speed=speed_in;
			if(MX_PWM_TIM1_init==1) 
			{
				
				//speed vpered 109 nazad 197 TIM1->CCR2	
				if(speed<0){TIM1->CCR2 = 155-0.42*speed;}
				if(speed>0){TIM1->CCR2 = 155-0.46*speed;}
				if(speed==0){TIM1->CCR2 = 155;}			
				//levo 110  155    180 vpravo   TIM1->CCR1	
				if(povorot_in<0){TIM1->CCR1 = 155-0.35*povorot_in;}
				if(povorot_in>0){TIM1->CCR1 = 155-0.35*povorot_in;}
				if(povorot_in==0){TIM1->CCR1 = 155;}		
			}
	 }	
	
	
}



float PIDcomtroller (signed int x)
{
	float y;
	y=x*b0+xz_1*b1+xz_2*b2+yz_1;
	xz_2=xz_1; xz_1=x;
	yz_1=y;
	
	
	if (y<-90){y=-99;}
	if (y>98){y=99;}
	
	return y;
}




void connect_raspberry(void)
{
	if(HAL_UART_Transmit_IT(&huart2, (uint8_t*)aTxStartMessage, TXSTARTMESSAGESIZE)!= HAL_OK){while(1){}}	
	while (HAL_UART_GetState(&huart2) != HAL_UART_STATE_READY){	} 
	
	
	if(HAL_UART_Receive_IT(&huart2, (uint8_t *)aRxBuffer, RXBUFFERSIZE) != HAL_OK){while(1){}}
	
	HAL_Delay(500); //ждём ключ
	
	if(strcmp((char *)Start_key,(char *)aRxBuffer)==0)  //если пришёл ключ то ок иначе ошибка ключа
	{
					
		lcd_SetPosition(0, 0);
		lcd_String("Rasberi OK    ");
	
	}else{	
		
		lcd_clear();
		lcd_SetPosition(0, 0);
		lcd_String("START Raspberry");
		while(1){} 	
	}
	
	if(HAL_UART_Transmit_IT(&huart2, (uint8_t*)aTxStartMessage, TXSTARTMESSAGESIZE)!= HAL_OK){while(1){}} // Отсылаем "О"	
	while (HAL_UART_GetState(&huart2) != HAL_UART_STATE_READY){	} 
	
	if(HAL_UART_Receive_IT(&huart2, (uint8_t *)aRxBuffer, RXBUFFERSIZE) != HAL_OK){while(1){}}
}




/**
  * @brief EXTI line detection callbacks
  * @param GPIO_Pin: Specifies the pins connected EXTI line
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{

	if(GPIO_Pin == GPIO_PIN_0)
  {		
		flag_regima1=1;		  // сработало прерывание значит у нас ручное управление
	
		if(MX_PWM_TIM1_init==1)   // если уже было инициализировано то выключаем таймер
			{		
					MX_PWM_TIM1_init=0;
							
							HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);   // vpered nazad
							HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1); //povorot
							
						__HAL_RCC_GPIOE_CLK_ENABLE();
							/*Configure GPIO pins : PE9 PE11 */
						GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_11;
						GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
						GPIO_InitStruct.Pull = GPIO_NOPULL;
						GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
						HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
			}	
		
		
			if(	HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0))
			{				 
				 HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, GPIO_PIN_SET);				 
			}else{
					HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, GPIO_PIN_RESET);
			}						
		

		j_6=0; // считаем ещё
	}
	
	
		if(GPIO_Pin == GPIO_PIN_1)
  {
		if(flag_regima1==1 && MX_PWM_TIM1_init==0)   // если у нас ручное управление
		{
			if(	HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1))
			{				 
				 HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_SET);				 
			}else{
					HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_RESET);
			}	
		}
	}
	
	
	
  if(GPIO_Pin == WAKEUP_BUTTON_PIN)
  {
		lcd_clear();
		lcd_SetPosition(0, 0);
		lcd_String("HANDBROCK");	
		HAL_Delay(1000);	
		
		
		
  }
  
  if(GPIO_Pin == TAMPER_BUTTON_PIN)
  {
	
		
  }
	
	if(GPIO_Pin == KEY_BUTTON_PIN)
  {
					
			
  }
		

	
}


/**
  * @brief  Tx Transfer completed callback
  * @param  UartHandle: UART handle. 
  * @note   This example shows a simple way to report end of IT Tx transfer, and 
  *         you can add your own implementation. 
  * @retval None
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *UartHandle)
{
  /* Turn LED1 on: Transfer in transmission process is correct */
  BSP_LED_Toggle(LED1);
}

/**
  * @brief  Rx Transfer completed callback
  * @param  UartHandle: UART handle
  * @note   This example shows a simple way to report end of IT Rx transfer, and 
  *         you can add your own implementation.
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
	if(flag_start==1){
	
	
						//=======================USART_function===================
			if(aRxBuffer[0]=='s' && aRxBuffer[5]=='t' && aRxBuffer[15]=='v')    // "s-001t+034reserv" пришло указание скорости и угла
			{
				if(aRxBuffer[1]=='-'){rasch_sto=-100;rasch_des=-10;rasch_od=-1;}
					else{rasch_sto=100;rasch_des=10;rasch_od=1;}
				if(aRxBuffer[2]=='-'){rasch_sto=0;rasch_des=-10;rasch_od=-1;}
					else if(aRxBuffer[2]==' '){rasch_sto=0;}
				if(aRxBuffer[3]=='-'){rasch_sto=0;rasch_des=0;rasch_od=-1;}
					else if(aRxBuffer[3]==' '){rasch_sto=0; rasch_des=0;}
				speed_in=rasch_sto*(aRxBuffer[2]-0x30)+rasch_des*(aRxBuffer[3]-0x30)+rasch_od*(aRxBuffer[4]-0x30);
				
				if(speed_in<-100 && speed_in>100){speed_in=0;}
				
				if(aRxBuffer[6]=='-'){rasch_sto=-100;rasch_des=-10;rasch_od=-1;}
					else{rasch_sto=100;rasch_des=10;rasch_od=1;}
				if(aRxBuffer[7]=='-'){rasch_sto=0;rasch_des=-10;rasch_od=-1;}
					else if(aRxBuffer[7]==' '){rasch_sto=0;}
				if(aRxBuffer[8]=='-'){rasch_sto=0;rasch_des=0;rasch_od=-1;}
					else if(aRxBuffer[8]==' '){rasch_sto=0; rasch_des=0;}
				povorot_in=rasch_sto*(aRxBuffer[7]-0x30)+rasch_des*(aRxBuffer[8]-0x30)+rasch_od*(aRxBuffer[9]-0x30);
				
				if(povorot_in<-100 && povorot_in>100){povorot_in=0;}
				
				for(i_rx=0;i_rx<16;i_rx++){aRxBuffer[i_rx]=0;}// очещаем буфер	
						
			}			

		//=======================end_USART_function===================
	
	
		if(HAL_UART_Receive_IT(&huart2, (uint8_t *)aRxBuffer, RXBUFFERSIZE) != HAL_OK){while(1){}}		
	}

  /* Turn LED2 on: Transfer in reception process is correct */
  BSP_LED_Toggle(LED2);
}

/**
  * @brief  UART error callbacks
  * @param  UartHandle: UART handle
  * @note   This example shows a simple way to report transfer error, and you can
  *         add your own implementation.
  * @retval None
  */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *UartHandle)
{
  /* Turn LED3 on: Transfer error in reception/transmission process */
  BSP_LED_Toggle(LED3); 
	
	lcd_clear();
	lcd_SetPosition(0, 0);
	lcd_String("UART ERROR");	
	HAL_Delay(1000);	
}





void encodersstart (void)
{
	 /* Start the encoder interface */
  HAL_TIM_Encoder_Start_IT(&htim3,  TIM_CHANNEL_1);
	HAL_TIM_Encoder_Start(&htim3,  TIM_CHANNEL_2);

	Encoderz_1=0x7FFF;
	Encoder = 0x7FFF;
	oldEncoder = 0x7FFF;
	__HAL_TIM_SetCounter(&htim3, 0x7FFF);  // Period =0xFFFF

	HAL_TIM_Base_Start_IT(&htim5);

}


void encodersRead (void)
{
	
		Encoderz_1=Encoder*0.85+Encoderz_1*0.15;
	
		encoderDiff=Encoderz_1-oldEncoder;
		
		encoderDiffz_1= encoderDiff*0.99+encoderDiffz_1*0.01;
		
		oldEncoder=Encoderz_1;
		
		speed_encoder=encoderDiffz_1/4.7;
				
}



	

/**
  * @brief  Conversion complete callback in non blocking mode 
  * @param  AdcHandle : AdcHandle handle
  * @note   This example shows a simple way to report end of conversion, and 
  *         you can add your own implementation.    
  * @retval None
  */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* AdcHandle)
{
	
  /* Get the converted value of regular channel */
  uhADCxConvertedValue = HAL_ADC_GetValue(AdcHandle);
}


/* USER CODE END 5 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void _Error_Handler(char * file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler_Debug */ 
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
