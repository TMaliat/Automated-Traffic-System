#include "CLOCK.h"
#include "GPIO.h"
#include "SYS_INIT.h"
#include "USART.h"
#include "stm32f4xx.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_STRINGS 15 /* maximum number of strings to store*/
#define MAX_STRING_LEN 50 /*maximum length of each string*/

static char input_buff[50], output_buff[50], str[50], s1[20], s2[20], s3[20], ch1, ch2, ch3;
static uint32_t in_idx, out_idx, trf, R1=1, R2=1, Y1=1, Y2=1, G1=1, G2=1, T1=1, T2=1, in=20, flag=0, totaltime =0, i;

void UART4_IRQHandler(void);
void UART5_IRQHandler(void);
void USART2_IRQHandler(void);
void UART4toUART5 (void);
void UART5toUART4 (void);
void display(void);
void getString(void);
void GPIO_Config(void);
void parseString(char *s);
void TIM6Config(void);
void TIM6_DAC_IRQHandler(void);

void display(void){
    static int prevState[2] = {-1, -1}; /*Initialize previous states to -1*/
    static int prevTime[2] = {-1, -1}; /*Initialize previous times to -1*/
    int currState = 0;
    int currTime = totaltime;
    
    if((GPIOB->IDR & (1<<3)) != 0){
        currState |= 0b10;
    }
    if(((GPIOB->IDR & (1<<10)) != 0) && ((GPIOB->IDR & (1<<4)) != 0)){
        currState |= 0b01;
    }
    else if(((GPIOB->IDR & (1<<10)) != 0) && ((GPIOB->IDR & (1<<4)) == 0)){
        currState |= 0b00;
    }
    else if(((GPIOB->IDR & (1<<10)) == 0) && ((GPIOB->IDR & (1<<4)) != 0)){
        currState |= 0b10;
    }
    else if(((GPIOB->IDR & (1<<10)) == 0) && ((GPIOB->IDR & (1<<4)) == 0)){
        currState |= 0b00;
    }
    
    /*Print current state*/
    sprintf(str,"%d traffic light 1 %s %s", currTime, (currState & 0b10) ? "OFF" : "ON", (currState & 0b01) ? "OFF ON" : "ON OFF");
    UART_SendString(USART2, "\n");
    UART_SendString(USART2,str);
    sprintf(str,"%d traffic light 2 %s %s", currTime, (currState & 0b10) ? "OFF" : "ON", (currState & 0b01) ? "ON OFF" : "OFF OFF");
    UART_SendString(USART2, "\n");
    UART_SendString(USART2,str);
		
		 /*Print road east-west and road north-south lights; Determine the state of the road lights*/
    int roadState = 0;
    if ((GPIOB->IDR & (1 << 10)) != 0) {
        roadState |= 0b10; /* Road east-west is heavy*/
    }
    if ((GPIOB->IDR & (1 << 4)) != 0) {
        roadState |= 0b01; /* Road north-south is heavy*/
    }
    sprintf(str,"%d road east west %s", currTime, (roadState & 0b10) ? "HEAVY" : "LIGHT");
    UART_SendString(USART2, "\n");
    UART_SendString(USART2,str);
    sprintf(str,"%d road north south %s", currTime, (roadState & 0b01) ? "HEAVY" : "LIGHT");
    UART_SendString(USART2, "\n");
    UART_SendString(USART2,str);
		
    
    /* Print previous 2 states*/
    for(i = 0; i < 2; i++){
        if(prevState[i] != -1){
            sprintf(str,"%d traffic light 1 %s %s", prevTime[i], (prevState[i] & 0b10) ? "OFF" : "ON", (prevState[i] & 0b01) ? "OFF ON" : "ON OFF");
            UART_SendString(USART2, "\n");
            UART_SendString(USART2,str);
            sprintf(str,"%d traffic light 2 %s %s", prevTime[i], (prevState[i] & 0b10) ? "OFF" : "ON", (prevState[i] & 0b01) ? "ON OFF" : "OFF OFF");
            UART_SendString(USART2, "\n");
            UART_SendString(USART2,str);
						sprintf(str,"%d road east west %s", prevTime[i], (prevState[i] & 0b10) ? "HEAVY" : "LIGHT");
						UART_SendString(USART2, "\n");
						UART_SendString(USART2,str);
						sprintf(str,"%d road north south %s", prevTime[i], (prevState[i] & 0b01) ? "HEAVY" : "LIGHT");
						UART_SendString(USART2, "\n");
						UART_SendString(USART2,str);
        }
    }
    
    /* Shift current state to previous state and update current state*/
    prevState[1] = prevState[0];
    prevState[0] = currState;
    prevTime[1] = prevTime[0];
    prevTime[0] = currTime;
}

void GPIO_Config(void){
	
	RCC -> AHB1ENR |= (1<<0); /*enables GPIOA clock*/
	RCC -> AHB1ENR |= (1<<1); /*enables GPIOB clock*/
	
	GPIOA -> OTYPER = 0; 
	GPIOA -> OSPEEDR = 01;
	GPIOB -> OSPEEDR = 01;
}

void TIM6Config(void){
	 /* Enable Timer6 Clock*/
	 RCC->APB1ENR |= (1<<4);
	
	/*Set the prescaler and the ARR*/
	TIM6->PSC = 9000-1; 
	TIM6->ARR = 10000-1;
	TIM6->DIER |=TIM_DIER_UIE;
	
	NVIC_EnableIRQ(TIM6_DAC_IRQn);
	/*Enable the Counter of the Timer and wait for the Update Flag to set*/
	TIM6->CR1 |= TIM_CR1_CEN; 
}
void parseString(char *s){
	  sscanf(s,"%s %s %s %d",s1,s2,s3,&trf);
    if(strcmp(s1,"config")==0)
		{
			if(strcmp(s3,"light")==0) /*config traffic light*/
			{
					if(trf==1)	sscanf(s,"%s %s %s %d %c %c %c %d %d %d %d",s1,s2,s3,&trf,&ch1,&ch2,&ch3,&G1,&Y1,&R1,&T1);		
					else	sscanf(s,"%s %s %s %d %c %c %c %d %d %d %d",s1,s2,s3,&trf,&ch1,&ch2,&ch3,&G2,&Y2,&R2,&T2);  						
			}
			/*config traffic monitor*/
			if(strcmp(s3, "monitor")==0)	sscanf(s, "%s %s %s %d",s1,s2,s3,&in);	
		}
		
		if(strcmp(s1,"read")==0)
		{
			if(strcmp(s3, "stat") ==0) /*read (traffic status)*/
			{
				sprintf(str,"traffic light 1 G Y R %d %d %d %d\n", G1, Y1, R1, T1);
				UART_SendString(USART2, str);
				sprintf(str,"traffic light 2 G Y R %d %d %d %d\n", G2, Y2, R2, T2);
				UART_SendString(USART2, str);
				sprintf(str,"traffic monitor %d\n", in);
				UART_SendString(USART2, str);	
			}
			if(strcmp(s3,"light")==0) /*read traffic light*/
			{
					if(trf==1)
					{
						sprintf(str,"traffic light 1 G Y R %d %d %d %d\n", G1, Y1, R1, T1);
					  UART_SendString(USART2, str);
					}		
					else
					{
						sprintf(str,"traffic light 2 G Y R %d %d %d %d\n", G2, Y2, R2, T2);
					  UART_SendString(USART2, str);
					}						
			}
			if(strcmp(s3, "monitor")==0) /*read traffic monitor*/
			{
				sprintf(str,"traffic monitor %d\n", in);
				UART_SendString(USART2, str);
				//TIM6->ARR = in * 1000 - 1; /* update the ARR with the new value of in */
			}
		}
}



void TIM6_DAC_IRQHandler(void)
{ 
	  if(TIM6->SR & (TIM_SR_UIF))
		{
				TIM6->SR &=~(TIM_SR_UIF);
				totaltime++;
				if((totaltime % in)==0)
					display();
		}
}


void getString(void){
    uint8_t ch,idx = 0;
    ch = UART_GetChar(USART2);
    while(ch != '.'){
        input_buff[idx++] = ch;
        ch = UART_GetChar(USART2);
    }      
    input_buff[idx] = '\0'; 
    flag = 1;		
}

void USART2_IRQHandler(void){
    USART2->CR1 &= ~(USART_CR1_RXNEIE);
    getString();
    USART2->CR1 |= (USART_CR1_RXNEIE);
}

void UART4_IRQHandler(void)
{   
    if (UART4->SR & USART_SR_RXNE){
        
        output_buff[out_idx] = (uint8_t) UART4->DR;
        
        UART4->SR &= ~(USART_SR_RXNE);
			  if(output_buff[out_idx]=='.')
					parseString(output_buff);
    }
    
    if (UART4->SR & USART_SR_TXE){

        UART4->DR = input_buff[in_idx];
        
        UART4->SR &= ~(USART_SR_TXE);
        UART4->CR1 &= ~(USART_CR1_TXEIE);
    }
}

void UART5_IRQHandler(void){
    
    if (UART5->SR & USART_SR_RXNE){   
        
        output_buff[out_idx] = (uint8_t) UART5->DR; 
        
        UART5->SR &= ~(USART_SR_RXNE);
        
    }
    if (UART5->SR & USART_SR_TXE){

        UART5->DR = input_buff[in_idx];      
        
        UART5->SR &= ~(USART_SR_TXE);
        UART5->CR1 &= ~USART_CR1_TXEIE;
    }
}


void UART5toUART4 (void){
	uint32_t i = 0;
	in_idx=0;
	out_idx =0;
	if(strlen(input_buff) != 0 ){
		for (i = 0; i < strlen(input_buff);i++){
		 /*to transmit data from UART5 to UART4*/
			UART4->CR1 |= USART_CR1_TXEIE; 	
			while((UART4->CR1 & USART_CR1_TXEIE));	
			ms_delay(1);
			in_idx++;
			out_idx++;
		}
		output_buff[out_idx++] = '\0';	
		/*UART_SendString(USART2,output_buff);*/
		strcpy(input_buff, "");
		UART_SendString(USART2,"\n");
	}
	parseString(output_buff);
}

int main(void)
{   		
	initClock();
	sysInit();
	UART2_Config();
	UART4_Config();
	UART5_Config();
	GPIO_Config();
	TIM6Config();
	
	GPIO_InitTypeDef op;
	GPIO_InitTypeDef ip;
	op.Mode = 01;
	ip.Mode= 00;
	
	GPIO_Init(GPIOA, &op);
	GPIO_Init(GPIOB, &ip);
	srand(89);	
 
	NVIC_SetPriority(USART2_IRQn, 1);
	NVIC_EnableIRQ(USART2_IRQn);
	NVIC_SetPriority(UART4_IRQn, 1);
	NVIC_EnableIRQ(UART4_IRQn);
	NVIC_SetPriority(UART5_IRQn, 1);
	NVIC_EnableIRQ(UART5_IRQn);
	
	UART_SendString(USART2,"Automated Traffic System\n");/*for checking*/
	
	while(1){
		
		int v1, v2; 
		v1 = (rand() % 2); /*traffic load(3rd led) of road1*/
		v2 = (rand() % 2); /*traffic load(3rd led) of road2*/
		
		if(flag==1){
			UART5toUART4();
			flag = 0;
		}

		if(v1>0) GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
		if(v2>0) GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
		
		GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET); /*red of road1*/
		ms_delay(R1*1000);
		
		if(v1 == 1 && v2==0) /*road1 heavy*/
		{
			GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
			GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET); /*red for road2*/
			ms_delay((R1+T1)*1000);
		}
		
		if(v1 == 0 && v2==1) /*road2 heavy*/
		{
			GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);
			GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
			ms_delay((R2+T2)*1000);
			
		}
		if((v1 ==0 && v2==0) || (v1==1 && v2==1))
		{
			GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
			GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);
			ms_delay(R2*1000);
		}
		
		GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
		GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
		GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);
		GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);
		
	}
}
