/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/


#include "stm32f10x.h"
			

int main(void)
{
	/*enable clocks*/
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
	RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;

	/* PC13 (built-in LED) = out, pp, 2MHz */
	GPIOC->CRH |= GPIO_CRH_MODE13_1;
	GPIOC->CRH &= ~GPIO_CRH_CNF13_0;
	/* PC14 (GND) = out, pp, 2MHz */
	GPIOC->CRH |= GPIO_CRH_MODE14_1;
	GPIOC->CRH &= ~GPIO_CRH_CNF14_0;
	GPIOC->BSRR |= GPIO_BSRR_BR14;

	/* PC15 (VDD)= out, pp, 2MHz */
	GPIOC->CRH |= GPIO_CRH_MODE15_1;
	GPIOC->CRH &= ~GPIO_CRH_CNF15_0;
	GPIOC->BSRR |= GPIO_BSRR_BS15;

	/* PA0 (CLK) = out, pp, 2MHz */
	GPIOA->CRL |= GPIO_CRL_MODE0_1;
	GPIOA->CRL &= ~GPIO_CRL_CNF0_0;
	/* PA1 (SI) = out, pp, 2MHz */
	GPIOA->CRL |= GPIO_CRL_MODE1_1;
	GPIOA->CRL &= ~GPIO_CRL_CNF1_0;

	/* PA2 (AO) = analog */
	GPIOA->CRL &= ~GPIO_CRL_MODE2_0;
	GPIOA->CRL &= ~GPIO_CRL_MODE2_1;
	GPIOA->CRL &= ~GPIO_CRL_CNF2_0;
	GPIOA->CRL &= ~GPIO_CRL_CNF2_1;

	/* PA9 (USART1 TX) = out, alt, 2MHz */
	GPIOA->CRH |= GPIO_CRH_CNF9_1;
	GPIOA->CRH &= ~GPIO_CRH_CNF9_0;
	GPIOA->CRH |= GPIO_CRH_MODE9_1;
	GPIOA->CRH &= ~GPIO_CRH_MODE9_0;

	/* PA10 (USART1 RX) = in, float*/
	/* default configuration */

	/*Configure USART1: 8N1 9600bps*/
	USART1->BRR = 8000000/9600; //BaudRate_Register=CPU_Freq / Desired_Baudrate
	/*Default configuration: 8N1 */
	/* Enable: USART, Rx not empty interrupt, Tx, Rx */
	USART1->CR1 = USART_CR1_UE | USART_CR1_RXNEIE| USART_CR1_TE | USART_CR1_RE;

	/* configure ADC1: order of channels*/
	ADC1->SQR3 |= 0b00010; /*1st: channel 2 (on PA2 pin)*/
	/* number of channels =0b00000 +1 = 1 */
		/*
		 * Set conversion time
		 * I don't care about speed, so I set the longest possible
		 * (but most precise): 239.5 cycles
		 * (With 8MHz (default) clock it takes ~30us
		 */
	ADC1->SMPR2 |= 0b000000111;

	/* Discontinuous mode: 1 channel per conversion */
	ADC1->CR1 |= ADC_CR1_DISCEN;
	/* Start conversion with writing SWSTART bit */
	ADC1->CR2 |= ADC_CR2_EXTTRIG | ADC_CR2_EXTSEL_0 | ADC_CR2_EXTSEL_1 | ADC_CR2_EXTSEL_2;
	/* Turn on ADC1 */
	ADC1->CR2 |= ADC_CR2_ADON;

	/*
	* Start calibration procedure
	* and wait till it ends
	*/

	ADC1->CR2 |= ADC_CR2_CAL;
	while(ADC1->CR2 & ADC_CR2_CAL);

	// camera data should be read with constant frequency
	// otherwise data would differ between read

	// read frequency depends on camera model and lighting
	// (how fast do the capacitors saturate?)
	SysTick_Config(8000000 * 0.01);
	NVIC_EnableIRQ(USART1_IRQn);
	for(;;);
}

// set/clear camera pins
void SI_HIGH(){
	GPIOA->BSRR |= GPIO_BSRR_BS1;
}
void SI_LOW(){
	GPIOA->BSRR |= GPIO_BSRR_BR1;
}
void CLK_HIGH(){
	GPIOA->BSRR |= GPIO_BSRR_BS0;
}
void CLK_LOW(){
	GPIOA->BSRR |= GPIO_BSRR_BR0;
}

//
void DELAY(){
	//with slower camera or faster uC clock there might be need for delay
	//one can put here a simple 'for' loop or timer trigger and 'while' condition
	//(in latter case one must configure a timer first)
}

#define PIXELS 128
int16_t cam[PIXELS];
int i;

int16_t x;
__attribute__((interrupt)) void SysTick_Handler(void){
    SI_HIGH();
    DELAY();
    CLK_HIGH();
    DELAY();
    SI_LOW();
    DELAY();
    ADC1->CR2 |= ADC_CR2_SWSTART;
    cam[0] = ADC1->DR;
	CLK_LOW();

    for(i=1;i<PIXELS;i++)
    {
        DELAY();
        CLK_HIGH();
        DELAY();
        ADC1->CR2 |= ADC_CR2_SWSTART;
    	cam[i] = ADC1->DR;						// return value
        CLK_LOW();
    }

    DELAY();
    CLK_HIGH();
    DELAY();
    CLK_LOW();
}

// USART interrupt:
// wait till something is received, then send captured image
int j;
__attribute__((interrupt)) void USART1_IRQHandler(void){
	if(USART1->SR & USART_SR_RXNE){
		USART1->SR &= ~USART_SR_RXNE;
		for (j=0; j<128;j++){
			x=cam[j];
			//send LSB part first
			while(!(USART1->SR & USART_SR_TXE)); // wait for empty transmit register
			USART1->DR = (x&0xff);
			//send MSB part
			while(!(USART1->SR & USART_SR_TXE));
			USART1->DR = (x&0xff00)>>8;
		}
		GPIOC->BSRR |= ((GPIOC->ODR & GPIO_ODR_ODR13) ? GPIO_BSRR_BR13 : GPIO_BSRR_BS13); //blink
	}
}
