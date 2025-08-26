#include"main.h"

extern uint8_t GPS_reading_status;
extern union reinterpret_cast decimalLong;
extern union reinterpret_cast decimalLat;
extern uint8_t satelliteNumber;

extern CAN_HandleTypeDef hcan;
extern struct buttons_layout buttons;
extern struct Data_aquisition_can can_data;


/* FREERTOS TASK FOR TRANSMITING THE MESSAGES FOR BMS, INVERTOR AND AUXILIARY.
 * RUNS EVERY 100MS.
 * HAS THE BIGGEST PRIORITY, SO IT WILL PREEPT EVERYONE ONCE IT IS IN PENDING STATE.
 */
void Can_transmit_handler() // 100 MS
{
	TickType_t xLastWakeTime;
	const TickType_t xPeriod = pdMS_TO_TICKS(100);

	xLastWakeTime = xTaskGetTickCount();

	while ( pdTRUE )
	{
		vTaskDelayUntil(&xLastWakeTime, xPeriod);

/********** BATTERY MANAGEMENT SYSTEM CONTROL ***********/
//		bms_state = get_bms_state(); //Battery was running in standalone.

		MPPT_Transmit();

/********** INVERTOR ACCELERATION / REGENERATION BREAK / CRUISE CONTROL ***********/

		motor_control();

/********** AUXILIARY CONTROL ***********/
		auxiliary_control();

	}
}
/* THE MOST IMPORTANT INFORMATION WAS TRASNMITTED, MEANING THE NEXT ONE IS THE GPS* THIS FUNCTION WORKS ONLY AFTER THE BMS, INV OR AUXILIARY DATA WAS TRASMITT */
void MPPT_Transmit()
{
	static const CAN_TxHeaderTypeDef MPPT_header =
	{ MPPT_POWER_TX_FOR_TELEMERTY, 0x00, CAN_RTR_DATA, CAN_ID_STD, 8, DISABLE };
	static uint32_t MPPT_mailbox;
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

	HAL_CAN_AddTxMessage(&hcan, &MPPT_header, MPPT_data, &MPPT_mailbox);
}




