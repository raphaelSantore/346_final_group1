#include "UART.h"

// Alternative Function 8 of PA.0 and PA.1
// PA.0 = UART4_TX, PA.1 = UART4_RX

// Alternative Function 7 of PB.6 and PB.7
// PB.6 = USART1_TX, PB.7 = USART1_RX 

extern int count;
extern unsigned char str[6];	
extern uint32_t global_counter_UART4;

void UART_Init(void) {
	
	// Enable the clock of USART 1 & 4
	RCC->APB2ENR  |= RCC_APB2ENR_USART1EN;  // Enable USART 1 clock		
	RCC->APB1ENR1 |= RCC_APB1ENR1_UART4EN;  // Enable USART 4 clock 
	
	// Select the USART1 clock source
	// 00: PCLK selected as USART2 clock
	// 01: System clock (SYSCLK) selected as USART2 clock
	// 10: HSI16 clock selected as USART2 clock
	// 11: LSE clock selected as USART2 clock
	RCC->CCIPR &= ~RCC_CCIPR_USART1SEL;
	RCC->CCIPR |=  RCC_CCIPR_USART1SEL_0;
	
	// Select the UART4 clock source 
	RCC->CCIPR &= ~RCC_CCIPR_UART4SEL;
	RCC->CCIPR |=  RCC_CCIPR_UART4SEL_0;
	
	UART_GPIO_Init();
	USART_Init(USART1);
	USART_Init(UART4);
	
	NVIC_SetPriority(USART1_IRQn, 1);	// Set Priority to 1
	NVIC_EnableIRQ(USART1_IRQn);			// Enable interrupt of USART1 peripheral

	//NVIC_SetPriority(UART4_IRQn, 1);	// Set Priority to 1
	//NVIC_EnableIRQ(UART4_IRQn);			 	// Enable interrupt of UART4 peripheral
}

void UART_GPIO_Init(void) {
	
	// Enable the peripheral clock of GPIO Port
	RCC->AHB2ENR |=   RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN;
	
	// ********************** USART 1 ***************************
	// AF = 7, PB.6 = USART1_TX, PB.7 = USART1_RX
	//Here, we configure PB.6 and PB.7. 
	// Alternate function, High Speed, Push pull, Pull up
	// **********************************************************
	// Input(00), Output(01), AlterFunc(10), Analog(11)
	GPIOB->MODER   &= ~(0x3 << (2*7));   			// Clear bits
	GPIOB->MODER   |=   0x2 << (2*7);   
	
	// Alternative function 7 = USART 1
	GPIOB->AFR[0]  |=   0x7 << (4*7);   
	
	// GPIO Speed: Low speed (00), Medium speed (01), Fast speed (10), High speed (11)
	GPIOB->OSPEEDR |=   0x3<<(2*7); 					 	
	
	// GPIO Push-Pull: No pull-up, pull-down (00), Pull-up (01), Pull-down (10), Reserved (11)
	GPIOB->PUPDR   &= ~(0x3<<(2*7));
	GPIOB->PUPDR   |=   0x1<<(2*7);  //select internal pull up  				
	
	// GPIO Output Type: Output push-pull (0, reset), Output open drain (1) 
	GPIOB->OTYPER  &=  ~(0x1<<7) ; 
	
	// ********************** UART 4 ***************************
	// AF = 8, PA.0 = UART4_TX, PA.1 = UART4_RX
	//Here we configure PA.0 and PA.1
	// Alternate function, High Speed, Push pull, Pull up
	// **********************************************************
	// Input(00), Output(01), AlterFunc(10), Analog(11)
	GPIOA->MODER   &= ~0x3;   								 // Clear bits
	GPIOA->MODER   |=  0x2;      						 
	// Alternative function 8 = UART 4
	GPIOA->AFR[0]  &= ~0xF;         				
	GPIOA->AFR[0]  |=  0x8;         				
	// GPIO Speed: Low speed (00), Medium speed (01), Fast speed (10), High speed (11)
	GPIOA->OSPEEDR |=  0x3;				
	
	// GPIO Push-Pull: No pull-up, pull-down (00), Pull-up (01), Pull-down (10), Reserved (11)
	GPIOA->PUPDR   &= ~0x3;			
	GPIOA->PUPDR   |=  0x1;	//select internal pull up 

  // GPIO Output Type: Output push-pull (0, reset), Output open drain (1) 
	GPIOA->OTYPER  &= ~0x1;                  	
}

void USART_Init (USART_TypeDef * USARTx) {
	// Default setting: 
	//     No hardware flow control, 8 data bits, no parity, 1 start bit and 1 stop bit		
	USARTx->CR1 &= ~USART_CR1_UE;  // Disable USART
	
	// Configure word length to 8 bit
	USARTx->CR1 &= ~USART_CR1_M;   // M: 00 = 8 data bits, 01 = 9 data bits, 10 = 7 data bits
	//reference manual, M1 is bit 28, M0 is bit 12 of CR1. 
	
	// Configure stop bits to be 1 stop bit
	//   00: 1 Stop bit;      01: 0.5 Stop bit
	//   10: 2 Stop bits;     11: 1.5 Stop bit
	USARTx->CR2 &= ~USART_CR2_STOP;   

  // Set parity control as no parity
  // 0 = no parity, 
  // 1 = parity enabled (then, program PS bit to select Even or Odd parity)
  USARTx->CR1 &= ~USART_CR1_PCE;  	
	
	// Configure oversampling mode: Oversampling by 16 
	USARTx->CR1 &= ~USART_CR1_OVER8;  // 0 = oversampling by 16, 1 = oversampling by 8	
                                    
	// Set Baudrate to 9600 using APB frequency (80,000,000 Hz)
	// If oversampling by 16, Tx/Rx baud = f_CK / USARTDIV,  
	// If oversampling by 8,  Tx/Rx baud = 2*f_CK / USARTDIV
  // When OVER8 = 0, BRR = USARTDIV
	// USARTDIV = 80MHz/9600 = 8333 = 0x208D
	USARTx->BRR  = 0x208D; // Limited to 16 bits

	USARTx->CR1  |= (USART_CR1_RE | USART_CR1_TE);  	// Transmitter and Receiver enable
	                   		
	//in this example, we only consider received data ready interrupt. 
	USARTx->CR1 |= USART_CR1_RXNEIE;  			// Received Data Ready to be Read Interrupt  
	
	USARTx->CR1 &= ~USART_CR1_TCIE;    			// Transmission Complete Interrupt 
	USARTx->CR1 &= ~USART_CR1_IDLEIE;  			// Idle Line Detected Interrupt 
	USARTx->CR1 &= ~USART_CR1_TXEIE;   			// Transmit Data Register Empty Interrupt 
	USARTx->CR1 &= ~USART_CR1_PEIE;    			// Parity Error Interrupt 
	USARTx->CR2 &= ~USART_CR2_LBDIE;				// LIN Break Detection Interrupt Enable
	USARTx->CR3 &= ~USART_CR3_EIE;					// Error Interrupt Enable (Frame error, noise error, overrun error) 
	USARTx->CR3 &= ~USART_CR3_CTSIE;				// CTS Interrupt, hardware flow control
	
	USARTx->CR1  |= USART_CR1_UE; // USART enable                 
	
	while ( (USARTx->ISR & USART_ISR_TEACK) == 0); // Verify that the USART is ready for reception
	while ( (USARTx->ISR & USART_ISR_REACK) == 0); // Verify that the USART is ready for transmission
}

void USART_Write(USART_TypeDef * USARTx, uint8_t *buffer, uint32_t nBytes, int start) {
	int i;
	// TXE is cleared by a write to the USART's TDR register.
	// TXE is set by hardware when the content of the TDR 
	// register has been transferred into the shift register.
	for (i = start; i < start+nBytes; i++) {
		while (!(USARTx->ISR & USART_ISR_TXE));   	
		// wait until TXE (TX empty) bit is set
		// Writing USART_DR automatically clears the TXE flag 	
		USARTx->TDR = buffer[i] & 0xFF;
		while (!(USARTx->ISR & USART_ISR_TC));   		  // wait until TC bit is set
		USARTx->ICR |= USART_ICR_TCCF; //interrupt flag clear register.
	}
}   
 

void USART_Delay(uint32_t us) {
	uint32_t time = 100*us/7;    
	while(--time);   
}

void My_USART_IRQHandler(USART_TypeDef * USARTx, 
	volatile uint8_t *buffer, volatile uint32_t * pRx_counter){
		
	if(USARTx->ISR & USART_ISR_RXNE) {						// Received data                         
		buffer[*pRx_counter] = USARTx->RDR;         
		// Reading USART_DR automatically clears the RXNE flag 
			
		(*pRx_counter)++;  
		if((*pRx_counter) >= BufferSize )  {
			(*pRx_counter) = 0;
		}   
	} 
	/*else if(USARTx->ISR & USART_ISR_TXE) {
 		//USARTx->ISR &= ~USART_ISR_TXE;            // clear interrupt 
		//Tx1_Counter++;
	} else if(USARTx->ISR & USART_ISR_ORE) {			// Overrun Error
		while(1);
	} else if(USARTx->ISR & USART_ISR_PE) {				// Parity Error
		while(1);
	} else if(USARTx->ISR & USART_ISR_PE) {				// USART_ISR_FE	
		while(1);
	} else if (USARTx->ISR & USART_ISR_NE){ 			// Noise Error Flag
		while(1);     
	}	
	*/
}


