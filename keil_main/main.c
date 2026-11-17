#include <string.h>
#include <stdio.h>
#include "stm32l476xx.h"
#include "SysClock.h"
#include "I2C.h"
#include "ssd1306.h"
#include "UART.h"

void I2C_GPIO_init(void);

volatile uint8_t USART2_Buffer_Rx[BufferSize] = {0};
volatile uint32_t Rx2_Counter = 0;

void USART2_IRQHandler(void) {
	My_USART_IRQHandler(USART2, USART2_Buffer_Rx, &Rx2_Counter);
}

int main(void) {
	int j;
	uint32_t last_rx_count = 0;
	char message[BufferSize + 1];
	
	System_Clock_Init(); 
	I2C_GPIO_init();
	I2C_Initialization(I2C1);
	UART_Init();         

	ssd1306_Init();
	ssd1306_Fill(Black);
	ssd1306_SetCursor(2, 0);
	ssd1306_WriteString("USB Ready", Font_11x18, White);
	ssd1306_UpdateScreen();
	
	while (1) {
		if (Rx2_Counter != last_rx_count) {
			last_rx_count = Rx2_Counter;



			
			for (j = 0; j < BufferSize; j++) {
				message[j] = USART_Read(USART2);
				
			}
			message[BufferSize] = '\0';

			ssd1306_Fill(Black);
			ssd1306_SetCursor(3, 0);
			ssd1306_WriteString(message, Font_11x18, White);
			ssd1306_UpdateScreen(); 
		}
	}
}