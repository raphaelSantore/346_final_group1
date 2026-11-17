#ifndef __STM32L4_UART_H
#define __STM32L4_UART_H

#include "stm32l476xx.h"

#define BufferSize 8

void UART_Init(void);
void UART_GPIO_Init(void);
void USART_Init(USART_TypeDef * USARTx);

//void USART_Write(USART_TypeDef * USARTx, uint8_t *buffer, uint32_t nBytes);
void USART_Write(USART_TypeDef * USARTx, uint8_t *buffer, uint32_t nBytes, int start);

void USART_Delay(uint32_t us);

void My_USART_IRQHandler(
	USART_TypeDef * USARTx, 
	volatile uint8_t *buffer, 
	volatile uint32_t * pRx_counter);

#endif /* __STM32L4_UART_H */
