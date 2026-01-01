/*
 * xbee.c
 *
 *  Created on: Jan 2, 2026
 *      Author: relu
 */

#include "main.h"

extern struct Data_aquisition_can can_data;
extern UART_HandleTypeDef huart2;


#define UART_MSG_LEN (16)
#define DMA_UART_MSG_CNT (20)
#define DMA_BUFFER_SIZE (UART_MSG_LEN * DMA_UART_MSG_CNT)

void xbee_handler(){
	static uint8_t MPPT_data[8];

		static uint16_t mppt1;
		static uint16_t mppt2;
		static uint16_t mppt3;
		static uint16_t mppt4;

		if (can_data.mppt1.output_current >= MPPT_SIGN_ERROR_VALUE)
			can_data.mppt1.output_current = 0;

		if (can_data.mppt2.output_current >= MPPT_SIGN_ERROR_VALUE)
			can_data.mppt2.output_current = 0;

		if( can_data.mppt3.output_current >= MPPT_SIGN_ERROR_VALUE)
			can_data.mppt3.output_current = 0;

		if( can_data.mppt4.output_current >= MPPT_SIGN_ERROR_VALUE)
			can_data.mppt4.output_current = 0;

		mppt1 = (uint16_t)(can_data.mppt1.output_voltage * can_data.mppt1.output_current);
		mppt2 = (uint16_t)(can_data.mppt2.output_voltage * can_data.mppt2.output_current);
		mppt3 = (uint16_t)(can_data.mppt3.output_voltage * can_data.mppt3.output_current);
		mppt4 = (uint16_t)(can_data.mppt4.output_voltage * can_data.mppt4.output_current);

		MPPT_data[0] = (   mppt1 & 0xFF );
		MPPT_data[1] = ( ( mppt1 >> 8) & 0xFF);

		MPPT_data[2] = (   mppt2 & 0xFF );
		MPPT_data[3] = ( ( mppt2 >> 8) & 0xFF);

		MPPT_data[4] = (   mppt3 & 0xFF );
		MPPT_data[5] = ( ( mppt3 >> 8) & 0xFF);

		MPPT_data[6] = (   mppt4 & 0xFF );
		MPPT_data[7] = ( ( mppt4 >> 8) & 0xFF);



	static uint8_t dma_buffer[DMA_BUFFER_SIZE] = { 0 };
	static uint8_t *const p_buffer = (uint8_t *)&dma_buffer;
	static uint8_t dma_offset = 0;


	dma_buffer[UART_MSG_LEN * dma_offset +  0] = 0xFE; // Start padding

	dma_buffer[UART_MSG_LEN * dma_offset +  1] = (0x310 & 0x000000FF) >> 0;
	dma_buffer[UART_MSG_LEN * dma_offset +  2] = (0x310 & 0x0000FF00) >> 8;

	dma_buffer[UART_MSG_LEN * dma_offset +  3] = MPPT_data[0];
	dma_buffer[UART_MSG_LEN * dma_offset +  4] = MPPT_data[1];
	dma_buffer[UART_MSG_LEN * dma_offset +  5] = MPPT_data[2];
	dma_buffer[UART_MSG_LEN * dma_offset +  6] = MPPT_data[3];
	dma_buffer[UART_MSG_LEN * dma_offset +  7] = MPPT_data[4];
	dma_buffer[UART_MSG_LEN * dma_offset +  8] = MPPT_data[5];
	dma_buffer[UART_MSG_LEN * dma_offset +  9] = MPPT_data[6];
	dma_buffer[UART_MSG_LEN * dma_offset + 10] = MPPT_data[7];

	//TIME, no rtc so 0
	dma_buffer[UART_MSG_LEN * dma_offset + 11] = 0;
	dma_buffer[UART_MSG_LEN * dma_offset + 12] = 0;
	dma_buffer[UART_MSG_LEN * dma_offset + 13] = 0;
	dma_buffer[UART_MSG_LEN * dma_offset + 14] = 0;


	dma_buffer[UART_MSG_LEN * dma_offset + 15] = 0x7F; // End padding

	 if (++dma_offset == DMA_UART_MSG_CNT)
	 {
	 	dma_offset = 0;
	    HAL_UART_Transmit_DMA(&huart2, p_buffer, DMA_BUFFER_SIZE);
	 }
}
