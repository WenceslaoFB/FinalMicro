/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "string.h"
#include "stdlib.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
 typedef union{
        uint8_t u8[4];
        int8_t  i8[4];

        uint16_t u16[2];
        int16_t  i16[2];

        uint32_t u32;
        int32_t  i32;

        float    f;
}_sWork;

typedef union{
    struct{
        uint8_t b0:1;
        uint8_t b1:1;
        uint8_t b2:1;
        uint8_t b3:1;
        uint8_t b4:1;
        uint8_t b5:1;
        uint8_t b6:1;
        uint8_t b7:1;
    }bit;
    uint8_t byte;
}flag;


/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define race 				flag1.bit.b0
#define ON100MS				flag1.bit.b1
#define ON10MS				flag1.bit.b2
#define stop				flag1.bit.b3
#define readyToSend			flag1.bit.b4
#define espReadyToRecieve	flag1.bit.b5
#define killRace			flag1.bit.b6
#define espConnected		flag2.bit.b0
#define sendALIVE			flag2.bit.b1

#define UART_PC		2
#define UART_ESP	1
#define LONG_CWJAP_MEGA	36
#define LONG_CWJAP_MICROS	35
#define LONG_ANS_CWJAP_MEGA	70
#define LONG_ANS_CWJAP_MICROS 70
const char CWJAP_MEGA[]="AT+CWJAP=\"FTTHBOUVET\",\"wenvla3112\"\r\n";
const char CWJAP_MICROS[]="AT+CWJAP=\"MICROS\",\"micros1234567\"\r\n";
const char CIFSR[]="AT+CIFSR\r\n";
const char CIPMUX[]="AT+CIPMUX=0\r\n";
const char CIPSTART[]="AT+CIPSTART=\"UDP\",\"192.168.1.195\",30017,3017\r\n";
const char CIPSEND[]="AT+CIPSEND=";
const char CIPCLOSE[]="AT+CIPCLOSE\r\n";
const char CWQAP[]= "AT+CWQAP\r\n";
const char CWMODE[]="AT+CWMODE=3\r\n";
const char ANS_CWQAP[]="AT+CWQAP\r\n\r\nOK\r\n";
const char ANS_CWMODE[]="AT+CWMODE=3\r\n\r\nOK\r\n";
const char ANS_CWJAP_MEGA[]="AT+CWJAP=\"FTTHBOUVET\",\"wenvla3112\"\r\nWIFI CONNECTED\r\nWIFI GOT IP\r\n\r\nOK\r\n";
const char ANS_CWJAP_MICROS[]="AT+CWJAP=\"MICROS\",\"micros1234567\"\r\nWIFI CONNECTED\r\nWIFI GOT IP\r\n\r\nOK\r\n";

const char ANS_CIPMUX[]="AT+CIPMUX=0\r\n\r\nOK\r\n";
const char ANS_CIPSTART[]="AT+CIPSTART=\"UDP\",\"192.168.1.195\",30017,3017\r\nCONNECT\r\n\r\nOK\r\n";//61
const char ANS_CIPSEND[]={};
const char AUTOMATIC_WIFI_CONNECTED[]={"WIFI CONNECTED\r\nWIFI GOT IP\r\n"};
const char WIFI_DISCONNECT[]="WIFI DISCONNECT\r\n";
const char CIFSR_STAIP[]="+CIFSR:STAIP,";
const char OK[]="\r\nOK\r\n";
const char CIPSEND1[]={'A','T','+','C','I','P','S','E','N','D','='};
const char CIPSEND2[]={'\r','\n','\r','\n','O','K','\r','\n','>'};
const char CIPSEND3[]="Recv ";
const char CIPSEND4[]={" bytes\r\n\r\nSEND OK\r\n"};//25
const char IPD[]="\r\n+IPD,";
const char UNER[]="UNER";
const char ALIVE[]={'U','N','E','R', 0x02 ,':',0xF0};
const char ACK_D0[]={'U','N','E','R',0x03,':',0xD0,0x0D,0xDC};
const int COORD_SENSORES[]={-5,-4,-3,-2,-1,1,2,3,4,5};
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */


volatile uint16_t ADCData[32][8];

volatile uint8_t bufferTxESP[256];
volatile uint8_t bufferRxESP[256];
volatile uint8_t bufferDebug[256];
volatile uint8_t buffer_PC_RX[256];
volatile uint8_t indexR_TxESP=0;
volatile uint8_t indexW_TxESP=0;
volatile uint8_t indexR_RxESP=0;
volatile uint8_t indexW_RxESP=0;
volatile uint8_t indexR_Debug=0;
volatile uint8_t indexW_Debug=0;
volatile uint8_t indexR_PC_RX=0;
volatile uint8_t indexW_PC_RX=0;
uint8_t AT=0;

volatile uint8_t time100ms;
volatile uint8_t time10ms;
uint8_t timeOut=0;
uint8_t timeOut2=0;
uint8_t timeOut3=0;
uint8_t timeToSendAlive=0;
uint8_t timeOutADC=0;
uint8_t timeOutPID=0;

uint8_t duty=0;
uint8_t decodeCIPSEND=0;
uint8_t decodeCIFSR=0;
char espIP[15];
uint8_t lengthIP=0;

uint32_t cmdUNERprotocol;
uint8_t bytesUNERprotocol;
uint8_t decodeIPD;
uint8_t cantBytes;
uint8_t cks;
uint8_t bytesToSend;
uint8_t comando=0xF0;
volatile uint8_t indexADC=0;
uint8_t bytesToSend_aux=0;


_sWork PWM_motor1,PWM_motor2,jobTime,error;
_sWork valueADC[8];
volatile flag flag1,flag2;

_sWork Kp,Kd,Ki;
float integral=0,derivativo=0,turn=0,Error=0,lastError=0;

uint8_t comandoActual=0;

uint8_t firstCalcu=1,timeOutArranque;
_sWork pwmBase;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void uartTX(uint8_t uart);
void initESP();
void DecodeAnsESP();
void udpCom(uint8_t cmd);
void DecodeCommands(uint8_t *buffer,uint8_t cmdPosInBuff);
void readADC();
void SerialCom(uint8_t cmd);
float findTheLine();
void DecodeQT();
void calculatePID(uint32_t pwmBase1,uint32_t pwmBase2);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	if(huart->Instance==USART1){
		indexW_RxESP++;
		HAL_UART_Receive_IT(&huart1,(uint8_t *) &bufferRxESP[indexW_RxESP], 1);
	}

	if(huart->Instance==USART2){
		indexW_PC_RX++;
		HAL_UART_Receive_IT(&huart1,(uint8_t *) &buffer_PC_RX[indexW_PC_RX], 1);
	}
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	indexADC++;
	if(indexADC == 32)
		indexADC=0;

}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if(htim->Instance == TIM4){
		HAL_ADC_Start_DMA(&hadc1, (uint32_t *)&ADCData[indexADC],8);

		time100ms--;
		if(!time100ms){
			time100ms=200;
			ON100MS = 1;
		}
		time10ms--;
		if(!time10ms){
			time10ms=20;
			ON10MS = 1;
		}

	}
}


void DecodeAnsESP(){
	static uint8_t i=0,k=1,cmdPosInBuff;
//	bufferDebug[indexW_Debug]=bufferRxESP[indexR_RxESP];
//	indexW_Debug++;
	switch(AT){
		case 0:
			if(bufferRxESP[indexR_RxESP]==ANS_CWMODE[i]){
				i++;
				if(i==19){
					AT++;
					i=0;
					readyToSend=1;
				}
			}else{
				if(!timeOut2){
					indexR_RxESP=indexW_RxESP;
					readyToSend=1;
					i=0;
					break;
				}
			}
			break;

		case 1:
			if(bufferRxESP[indexR_RxESP]==AUTOMATIC_WIFI_CONNECTED[i]){
				i++;
				if(i==29){
					AT=3;
					i=0;
					readyToSend=1;
				}
			}
			else{
				if(bufferRxESP[indexR_RxESP]==WIFI_DISCONNECT[i]){
					i++;
					if(i==17){
						AT=0;
						i=0;
						readyToSend=1;
						HAL_UART_AbortReceive_IT(&huart1);
						HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET);//Reset ESP8266
						timeOut=20;
					}
				}else{
					AT=2;
					i=0;
					readyToSend=1;
					indexR_RxESP=indexW_RxESP;
					break;
				}
			}
			break;
		case 2:
			if(bufferRxESP[indexR_RxESP]==ANS_CWJAP_MEGA[i]){
				i++;

				if(i==LONG_ANS_CWJAP_MEGA){
					AT++;
					i=0;
					readyToSend=1;
				}
			}
			else{
				if(!timeOut2){
					indexR_RxESP=indexW_RxESP;
					readyToSend=1;
					i=0;
					break;
				}
			}
			break;
		case 3:
			if(bufferRxESP[indexR_RxESP]==ANS_CIPMUX[i]){
				i++;
				if(i==19){
					AT+=2;
					i=0;
					readyToSend=1;
				}
			}else{
				if(!timeOut2){
					indexR_RxESP=indexW_RxESP;
					readyToSend=1;
					i=0;
					break;
				}
			}
			break;
		case 4:
			switch(decodeCIFSR){
				case 0:
					if(bufferRxESP[indexR_RxESP]==CIFSR[i]){
						i++;

						if(i==10){
							decodeCIFSR++;
							i=0;
						}
					}else{
						if(!timeOut2){
							indexR_RxESP=indexW_RxESP;
							readyToSend=1;
							i=0;
							break;
						}
					}
					break;
				case 1:
					if(bufferRxESP[indexR_RxESP]==CIFSR_STAIP[i]){
						i++;
						if(i==13){
							decodeCIFSR++;
							i=0;
						}
					}
					break;
				case 2:
					espIP[i]=bufferRxESP[indexR_RxESP];
					i++;
					lengthIP++;
					if((bufferRxESP[indexR_RxESP]=='"')&&(lengthIP>1)){
						i=0;
						decodeCIFSR++;
					}
					break;
				case 3:
					if(bufferRxESP[indexR_RxESP]==OK[i]){
						i++;
						if(i==6){
							AT++;
							i=0;
							readyToSend=1;
							decodeCIFSR=0;
						}
					}
					break;
			}

			break;
		case 5:
			if(bufferRxESP[indexR_RxESP]==ANS_CIPSTART[i]){
				i++;
				if(i==60){
					AT++;
					i=0;
					readyToSend=1;
					espConnected=1;
				}
			}else{
				if(!timeOut2){
					indexR_RxESP=indexW_RxESP;
					readyToSend=1;
					i=0;
					break;
				}
			}
			break;
		case 6:
			switch(decodeCIPSEND){
					case 0:
						if(bufferRxESP[indexR_RxESP]==CIPSEND1[i]){
							i++;
							if(i==11){
								i=0;
								decodeCIPSEND++;
							}
						}
						else{
							if(!timeOut2){
								indexR_RxESP=indexW_RxESP;
								readyToSend=1;
								i=0;
								espReadyToRecieve=0;
								sendALIVE=0;
								timeToSendAlive=30;
								break;
							}
						}
						break;
					case 1:
						if((bufferRxESP[indexR_RxESP]==bytesToSend+'0')&&((bytesToSend<10))){
							decodeCIPSEND+=2;
						}else{

							if(bufferRxESP[indexR_RxESP]==bytesToSend/10+'0'){
								decodeCIPSEND++;
								bytesToSend_aux=bytesToSend/10;
								bytesToSend_aux*=10;
							}

						}
						break;
					case 2:
						if(bufferRxESP[indexR_RxESP]==bytesToSend-bytesToSend_aux+'0'){
							decodeCIPSEND++;
						}
						break;
					case 3:
						if(bufferRxESP[indexR_RxESP]==CIPSEND2[i]){
							i++;
							if(i==9){
								i=0;
								decodeCIPSEND++;
								espReadyToRecieve=1;
								readyToSend=1;
							}
						}else{
							if(!timeOut2){
								i=0;
								decodeCIPSEND=0;
								espReadyToRecieve=0;
								readyToSend=1;
								indexR_RxESP=indexW_RxESP;
								sendALIVE=0;
								timeToSendAlive=30;
								return;
							}
						}

						break;
					case 4:
						if(bufferRxESP[indexR_RxESP]==CIPSEND3[i]){
							i++;
							if(i==5){
								i=0;
								decodeCIPSEND++;
							}
						}else{
							if(!timeOut2){
								i=0;
								decodeCIPSEND=0;
								espReadyToRecieve=0;
								readyToSend=1;
								indexR_RxESP=indexW_RxESP;
								sendALIVE=0;
								timeToSendAlive=30;
								break;
							}
						}
						break;
					case 5:
						if((bufferRxESP[indexR_RxESP]==bytesToSend+'0')&&((bytesToSend<10))){
							decodeCIPSEND++;
							decodeCIPSEND++;
						}else{

							if(bufferRxESP[indexR_RxESP]==bytesToSend/10+'0'){
								decodeCIPSEND++;
							}else{
								i=0;
								decodeCIPSEND=0;
								espReadyToRecieve=0;
								readyToSend=1;
								indexR_RxESP=indexW_RxESP;
								sendALIVE=0;
								timeToSendAlive=30;
								break;
							}
						}
						break;
					case 6:
						if(bufferRxESP[indexR_RxESP]==bytesToSend-bytesToSend_aux+'0'){
							decodeCIPSEND++;
						}else{
							i=0;
							decodeCIPSEND=0;
							espReadyToRecieve=0;
							readyToSend=1;
							indexR_RxESP=indexW_RxESP;
							sendALIVE=0;
							timeToSendAlive=30;
							break;
						}
						break;
					case 7:
						if(bufferRxESP[indexR_RxESP]==CIPSEND4[i]){
							i++;
							if(i==19){
								i=0;
								decodeCIPSEND=0;
								readyToSend=1;
								espReadyToRecieve=0;
							}
						}else{
							if(!timeOut2){
								indexR_RxESP=indexW_RxESP;
								i=0;
								decodeCIPSEND=0;
								espReadyToRecieve=0;
								readyToSend=1;
								sendALIVE=0;
								timeToSendAlive=30;
								break;
							}
						}
						break;
				}

			break;
		case 7:
			switch(decodeIPD){
				case 0:
					if(bufferRxESP[indexR_RxESP]==IPD[i]){
						i++;

						if(i==7){
							i=0;
							decodeIPD++;
						}
					}
					else{
						if(i>0){
							indexR_RxESP=indexW_RxESP;
							i=0;
							break;
						}
					}
					break;
				case 1:
					if(bufferRxESP[indexR_RxESP]==':'){
						decodeIPD++;
					}
					break;
				case 2:
					if(bufferRxESP[indexR_RxESP]==UNER[i]){
						i++;
						if(i==4){
							i=0;
							decodeIPD++;
							cks='U'^'N'^'E'^'R';
						}
					}else{
						if(i>0){
							indexR_RxESP=indexW_RxESP;
							i=0;
							decodeIPD=0;
							break;
						}
					}
					break;
				case 3:
					bytesUNERprotocol=bufferRxESP[indexR_RxESP];
					decodeIPD++;
					cks^=bufferRxESP[indexR_RxESP];

					break;
				case 4:
					if(bufferRxESP[indexR_RxESP]==':'){
						decodeIPD++;
						cks^=bufferRxESP[indexR_RxESP];

					}else{
						indexR_RxESP=indexW_RxESP;
						i=0;
						decodeIPD=0;
						break;
					}
					break;
				case 5:
					if(k==1)
						cmdPosInBuff=indexR_RxESP;
					if(k<bytesUNERprotocol){
						cks^=bufferRxESP[indexR_RxESP];
						k++;
					}else{
						if(cks==bufferRxESP[indexR_RxESP]){
							DecodeCommands((uint8_t*)&bufferRxESP,cmdPosInBuff);
							HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
							k=1;
							i=0;
							decodeIPD=0;
						}else{
							i=0;
							decodeIPD=0;
							k=1;
						}
					}
					break;
			}
			break;
			default:

				break;
	}
	indexR_RxESP++;
}

uint8_t posMINCenter=0,posMINRight=0,posMINLeft=0;
uint16_t sensorValue=0;
float xMin=0,fx2_fx3,fx2_fx1,x2_x1,x2_x3,x2_x1cuad,x2_x3cuad,denominador;
uint8_t f=0;

float findTheLine(){//ALGORITMO CUADRATICO
	float aux[10];

	sensorValue = valueADC[0].u16[0];


	posMINCenter=0;
	while(f<8){					//ENCUENTRO LA MENOR LECTURA
		if(sensorValue > valueADC[f].u16[0]){
			sensorValue=valueADC[f].u16[0];
			posMINCenter=f;
		}
		aux[f+1]=valueADC[f].u16[0];
		f+=1;
	}
	f=0;
	posMINCenter+=1;
	aux[0]=aux[2];
	aux[9]=aux[7];

	posMINRight=posMINCenter-1;
	posMINLeft=posMINCenter+1;
	fx2_fx3=aux[posMINCenter]-aux[posMINRight];
	fx2_fx1=aux[posMINCenter]-aux[posMINLeft];
	x2_x1=COORD_SENSORES[posMINCenter]-COORD_SENSORES[posMINLeft];
	x2_x1cuad=(x2_x1*x2_x1);
	x2_x3=COORD_SENSORES[posMINCenter]-COORD_SENSORES[posMINRight];
	x2_x3cuad=(x2_x3*x2_x3);
	denominador=(2*(x2_x1*fx2_fx3-x2_x3*fx2_fx1));
	if(denominador!=0){
		xMin=COORD_SENSORES[posMINCenter]-( x2_x1cuad*fx2_fx3 - x2_x3cuad*fx2_fx1 ) / denominador;
	}
	return -xMin;
}

void readADC(){
static uint8_t firstRead=1;

	if(indexADC>2){
		firstRead=0;
		for(uint8_t c=0;c<8;c++){
			valueADC[c].u32=( ADCData[indexADC-1][c]+ADCData[indexADC-2][c]+ADCData[indexADC-3][c] ) / 3 ;
		}
	}
	else{
		if(!firstRead){
			for(uint8_t c=0;c<8;c++){
				switch(indexADC){
					case 2:
						valueADC[c].u32=(ADCData[1][c]+ADCData[0][c]+ADCData[31][c]) / 3;
						break;
					case 1:
						valueADC[c].u32=( ADCData[0][c]+ADCData[31][c]+ADCData[30][c] ) / 3 ;
						break;
					case 0:
						valueADC[c].u32=( ADCData[31][c]+ADCData[30][c]+ADCData[29][c] ) / 3;
						break;
				}
			}
		}
	}
}

void uartTX(uint8_t uart){

	if(uart == UART_PC ){
		if((huart2.Instance->SR & UART_FLAG_TXE)==UART_FLAG_TXE){
			huart2.Instance->DR=bufferDebug[indexR_Debug];
			indexR_Debug++;
		}
	}

	if(uart == UART_ESP ){
		if((huart1.Instance->SR & UART_FLAG_TXE)==UART_FLAG_TXE){
			huart1.Instance->DR=bufferTxESP[indexR_TxESP];
			indexR_TxESP++;
		}
	}
}

void SerialCom(uint8_t cmd){
	uint8_t cks,h=0;
	switch(cmd){
		case 0xA1:

			memcpy((uint8_t*)&bufferDebug[indexW_Debug],UNER,4);
			indexW_Debug+=4;
			cks='U'^'N'^'E'^'R';

			bufferDebug[indexW_Debug]=22;
			indexW_Debug+=1;
			cks^=22;

			bufferDebug[indexW_Debug]=':';
			indexW_Debug+=1;
			cks^=':';

			bufferDebug[indexW_Debug]=0xA1;
			indexW_Debug+=1;
			cks^=0xA1;
			h=0;
			while(h<8){
				bufferDebug[indexW_Debug]=valueADC[h].u8[0];
				indexW_Debug+=1;
				cks^=valueADC[h].u8[0];
				bufferDebug[indexW_Debug]=valueADC[h].u8[1];
				indexW_Debug+=1;
				cks^=valueADC[h].u8[1];
				h+=1;
			}

			bufferDebug[indexW_Debug]=error.u8[0];
			cks^=error.u8[0];
			indexW_Debug+=1;

			bufferDebug[indexW_Debug]=error.u8[1];
			cks^=error.u8[1];
			indexW_Debug+=1;

			bufferDebug[indexW_Debug]=error.u8[2];
			cks^=error.u8[2];
			indexW_Debug+=1;

			bufferDebug[indexW_Debug]=error.u8[3];
			cks^=error.u8[3];
			indexW_Debug+=1;

			bufferDebug[indexW_Debug]=cks;
			indexW_Debug+=1;

			break;
	}
}

void  initESP(){

	if(readyToSend){
		switch(AT){
			case 0:
				memcpy((uint8_t *)&bufferTxESP[indexW_TxESP],CWMODE,13);
				indexW_TxESP+=13;
				timeOut2=10;
				readyToSend=0;
				break;
			case 1:
				timeOut2=100;
				readyToSend=0;
				break;
			case 2:
				memcpy((uint8_t *)&bufferTxESP[indexW_TxESP],CWJAP_MEGA,LONG_CWJAP_MEGA);
				indexW_TxESP+=LONG_CWJAP_MEGA;
				timeOut2=50;
				readyToSend=0;
				break;
			case 3:
				memcpy((uint8_t *)&bufferTxESP[indexW_TxESP],CIPMUX,13);
				indexW_TxESP+=13;
				timeOut2=20;
				readyToSend=0;
				break;
			case 4:
				memcpy((uint8_t *)&bufferTxESP[indexW_TxESP],CIFSR,10);
				indexW_TxESP+=10;
				timeOut2=20;
				readyToSend=0;
				break;
			case 5:
				memcpy((uint8_t *)&bufferTxESP[indexW_TxESP],CIPSTART,46);
				indexW_TxESP+=45;
				timeOut2=20;
				readyToSend=0;
				break;
			case 6:
				duty++;
				break;
		}
	}
}






void udpCom(uint8_t cmd){
	if(readyToSend){
		switch(cmd){
			case 0xF0:
				if(!espReadyToRecieve){
					AT=6;
					memcpy((uint8_t*)&bufferTxESP[indexW_TxESP],CIPSEND,11);
					indexW_TxESP+=11;
					memcpy((uint8_t*)&bufferTxESP[indexW_TxESP],"8\r\n",3);
					indexW_TxESP+=3;
					bytesToSend=8;
					timeOut2=2;
					readyToSend=0;
				}
				else{
					memcpy((uint8_t*)&bufferTxESP[indexW_TxESP],ALIVE,7);
					indexW_TxESP+=7;
					bufferTxESP[indexW_TxESP]='U'^'N'^'E'^'R'^0x02^':'^0xF0;
					indexW_TxESP+=1;
					espReadyToRecieve=0;
					sendALIVE=0;
					readyToSend=0;
				}
				break;
			case 0xD0:
				if(!espReadyToRecieve){
					AT=6;
					memcpy((uint8_t*)&bufferTxESP[indexW_TxESP],CIPSEND,11);
					indexW_TxESP+=11;
					memcpy((uint8_t*)&bufferTxESP[indexW_TxESP],"9\r\n",3);
					indexW_TxESP+=3;
					bytesToSend=9;
					timeOut2=20;
					readyToSend=0;
				}
				else{
					memcpy((uint8_t*)&bufferTxESP[indexW_TxESP],ACK_D0,9);
					indexW_TxESP+=9;
					bufferTxESP[indexW_TxESP]='U'^'N'^'E'^'R'^0x03^':'^0xD0^0x0D;
					indexW_TxESP++;
					espReadyToRecieve=0;
					duty++;
				}
				break;
		}
	}
}

void DecodeQT(){
	static uint8_t i=0,step=0,cksQT,counter=1,cmdPosInBuff;

	switch(step){
		case 0:
			if(buffer_PC_RX[indexR_PC_RX]==UNER[i]){
				i++;
				if(i==4){
					step++;
					i=0;
					cksQT='U'^'N'^'E'^'R';
				}
			}else
				i=0;

			break;
		case 1:
			bytesUNERprotocol=buffer_PC_RX[indexR_PC_RX];
			step++;
			cksQT^=bytesUNERprotocol;
			break;
		case 2:
			if(buffer_PC_RX[indexR_PC_RX]==':'){
				step++;
				cksQT^=':';
			}else{
				step=0;
			}
			break;
		case 3:
			if(counter==1)
				cmdPosInBuff=indexR_PC_RX;
			if(counter<bytesUNERprotocol){
				cksQT^=buffer_PC_RX[indexR_PC_RX];
				counter++;
			}else{
				if(cksQT==buffer_PC_RX[indexR_PC_RX]){
					DecodeCommands((uint8_t*)&buffer_PC_RX, cmdPosInBuff);
				}
				step=0;
				counter=1;
			}
	}
	indexR_PC_RX++;
}

void DecodeCommands(uint8_t *buffer,uint8_t indexCMD){
	uint8_t i=1;


	switch(buffer[indexCMD]){
		case 0xB0://STOP AUTITO
				comandoActual=0xB0;
				stop=1;
				break;
		case 0xF1: //START AUTITO
				PWM_motor1.u8[0]=buffer[indexCMD+i];
				i++;
				PWM_motor1.u8[1]=buffer[indexCMD+i];
				i++;
				PWM_motor1.u8[2]=buffer[indexCMD+i];
				i++;
				PWM_motor1.u8[3]=buffer[indexCMD+i];
				PWM_motor2.u32=PWM_motor1.u32;
				comandoActual=0xF1;
				race=1;
				timeOutPID=2;
				break;
		case 0xC0:	//SETEAR PARAMETROS CONTROL PID
				Kp.u8[0]=buffer[indexCMD+i];
				i++;
				Kp.u8[1]=buffer[indexCMD+i];
				i++;
				Kp.u8[2]=buffer[indexCMD+i];
				i++;
				Kp.u8[3]=buffer[indexCMD+i];
				i++;
				Kp.u8[0]=buffer[indexCMD+i];
				i++;
				Kd.u8[1]=buffer[indexCMD+i];
				i++;
				Kd.u8[2]=buffer[indexCMD+i];
				i++;
				Kd.u8[3]=buffer[indexCMD+i];
				i++;
				Ki.u8[0]=buffer[indexCMD+i];
				i++;
				Ki.u8[1]=buffer[indexCMD+i];
				i++;
				Ki.u8[2]=buffer[indexCMD+i];
				i++;
				Ki.u8[3]=buffer[indexCMD+i];
				i++;
				comandoActual=0xC0;

				break;
		case 0xF0: //ALIVE
				duty=2;
				readyToSend=1;
				AT=6;
				break;

		case 0xD0://JOB TIME
				PWM_motor1.u8[0]=buffer[indexCMD+i];
				i++;
				PWM_motor1.u8[1]=buffer[indexCMD+i];
				i++;
				PWM_motor1.u8[2]=buffer[indexCMD+i];
				i++;
				PWM_motor1.u8[3]=buffer[indexCMD+i];
				i++;
				PWM_motor2.u8[0]=buffer[indexCMD+i];
				i++;
				PWM_motor2.u8[1]=buffer[indexCMD+i];
				i++;
				PWM_motor2.u8[2]=buffer[indexCMD+i];
				i++;
				PWM_motor2.u8[3]=buffer[indexCMD+i];
				i++;

				jobTime.u8[0]=bufferRxESP[indexCMD+i];
				i++;
				jobTime.u8[1]=bufferRxESP[indexCMD+i];
				i++;
				jobTime.u8[2]=bufferRxESP[indexCMD+i];
				i++;
				jobTime.u8[3]=bufferRxESP[indexCMD+i];
				i++;
				jobTime.u32=jobTime.u32/100;
				__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_1,PWM_motor1.u32);
				__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_2,0);
				__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_3,0);
				__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_4,PWM_motor2.u32);
				race=1;
				killRace=1;
				duty=2;
				comandoActual=0xD0;
				comando=0xD0;
				readyToSend=1;
				break;



	}
}

void calculatePID(uint32_t pwmBase1,uint32_t pwmBase2){
	float pwm1,pwm2;

	integral+=error.f;
	derivativo=error.f-lastError;
	turn= Kp.f*error.f + Kd.f*derivativo + Ki.f*integral;
	pwm1=pwmBase1-turn;
	pwm2=pwmBase2+turn;

	if(pwm1>200)
		pwm1=200;
	if(pwm2>200)
		pwm2=200;
	__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_1,pwm1);
	__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_2,0);
	__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_3,0);
	__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_4,pwm2);
	lastError=error.f;
}

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
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start(&htim3);

  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  HAL_TIM_Base_Start_IT(&htim4);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET);//ESP8266 Pin Enable
  timeOut2=30;
  timeOut3=10;
  timeToSendAlive=30;
  timeOut=30;
  readyToSend=1;
  race=0;
  killRace=0;
  stop=0;
  duty=0;
  espConnected=0;
  sendALIVE=0;

  PWM_motor1.u32=0;
  PWM_motor2.u32=0;
  time100ms=200;
  time10ms=20;
  timeOutADC=0;
  HAL_UART_Receive_IT(&huart2, (uint8_t *)&buffer_PC_RX[indexW_PC_RX],1);
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  if(ON100MS){
		ON100MS=0;
	  	if(timeOut>0)
			timeOut--;
		if(timeOut2>0)
			timeOut2--;
		if(timeOut3>0)
			timeOut3--;
		if(jobTime.u32>0)
			jobTime.u32--;
		if((timeToSendAlive>0)&&(espConnected))
			timeToSendAlive--;

	  }

	  if(ON10MS){
		  ON10MS=0;
		  if(timeOutADC>0)
			timeOutADC--;

		  if(timeOutPID>0)
				timeOutPID--;
	  }

	  if(!timeOut3){
		  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
		  timeOut3=10;

	  }
	  if( (!timeToSendAlive) && (espConnected) ){
		  sendALIVE=1;
		  espReadyToRecieve=0;
		  timeToSendAlive=30;
		  readyToSend=1;
	  }
	  if(sendALIVE)
		  udpCom(0xF0);


	  switch(duty){
	  	  case 0:
	  		  if(!timeOut){
	  			 initESP();
	  			 HAL_UART_Receive_IT(&huart1, (uint8_t *)&bufferRxESP[indexW_RxESP], 1);
	  		  	 duty++;
	  		  }
	  		  break;
	  	  case 1:
	  		  initESP();
	  		  break;
	  }

	  if(!timeOutADC){
		readADC();
		error.f=findTheLine();
		timeOutADC=2;
//		SerialCom(0xA1);
	  }

	  if( (race) && (!timeOutPID) ){
		  calculatePID(PWM_motor1.u32,PWM_motor2.u32);
		  timeOutPID=2;

	  }

	  if( ( ( (!jobTime.u32) && (killRace) ) ) || (stop) ) {
		  stop=0;
		  race=0;
		  killRace=0;
		__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_1,0);
		__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_2,0);
		__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_3,0);
		__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_4,0);
	  }


	  if(indexR_PC_RX!=indexW_PC_RX)
		  DecodeQT();


	  if(indexR_TxESP!=indexW_TxESP)
		  uartTX(UART_ESP);


	  if(indexR_Debug!=indexW_Debug)
		  uartTX(UART_PC);



	  if(indexR_RxESP!=indexW_RxESP)
		  DecodeAnsESP();


	  if((HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10)==GPIO_PIN_RESET)&&(!timeOut)){
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET);//ESP8266 Pin Enable
		  timeOut=30;
		  AT=1;
		  duty=0;
	  }
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
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
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ENABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 8;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SEQ_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = 3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_8;
  sConfig.Rank = 4;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_11;
  sConfig.Rank = 5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_10;
  sConfig.Rank = 6;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_12;
  sConfig.Rank = 7;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_13;
  sConfig.Rank = 8;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

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
  hi2c1.Init.ClockSpeed = 100000;
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

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 99;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 210;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 999;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 42;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
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
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
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
  __disable_irq();
  while (1)
  {
  }
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
