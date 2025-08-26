#include"main.h"

extern I2C_HandleTypeDef hi2c1;
extern struct Data_aquisition_can can_data;
extern struct buttons_layout buttons;
extern uint8_t Can_error_counter;
extern struct Telemetry_RTC RealTimeClock;
extern struct Modules_Activity ActivityCheck;
extern struct buttons_layout buttons;
extern CAN_HandleTypeDef hcan;

struct pop_up_error pop_up_error_condition = {0};

extern int battery_voltage;
extern int battery_current;

void Display_Init()
{
	HD44780_Init(4);
	HD44780_SetBacklight(255);
	HD44780_Clear();
}

void MAIN_Display(char *buffer)
{
	//first row
	HD44780_SetCursor(0, 0);
	snprintf(buffer, 21, "%4.1f |V %3.0f  | %4.3f",
			can_data.bms.maximum_cell_temperature,
			 (can_data.invertor.motor_velocity.Float32) * 3.6F,
			can_data.bms.maximum_cell_voltage);
	HD44780_PrintStr(buffer);

	//second row
	HD44780_SetCursor(0, 1);
	snprintf(buffer, 21, "%4.1f |B %3.0f %%| %4.3f",
			can_data.bms.minimum_cell_temperature,
			can_data.bms.State_Of_Charge,
			can_data.bms.minimum_cell_voltage);
	HD44780_PrintStr(buffer);

	//third row
	if (can_data.mppt1.output_current >= MPPT_SIGN_ERROR_VALUE)
		can_data.mppt1.output_current = 0;

	if (can_data.mppt2.output_current >= MPPT_SIGN_ERROR_VALUE)
		can_data.mppt2.output_current = 0;

	if( can_data.mppt3.output_current >= MPPT_SIGN_ERROR_VALUE)
		can_data.mppt3.output_current = 0;

	if( can_data.mppt4.output_current >= MPPT_SIGN_ERROR_VALUE)
		can_data.mppt4.output_current = 0;

	float bms_power      = (float)(battery_current * battery_voltage)
							/1000000.0f;

	HD44780_SetCursor(0, 2);
	snprintf(buffer, 21, "POWER:    %6.1f   ", bms_power);
	HD44780_PrintStr(buffer);

	//forth row
	HD44780_SetCursor(0, 3);
	//for testing

	char drive_state[7] = "  IDLE";

#if ( PIT_TESTING == 1)
	can_data.bms.state = DRIVE;
#endif

	switch(can_data.bms.state)
	{
	case IDLE:
		strncpy(drive_state, "  IDLE", sizeof(drive_state));
		break;
	case PRE_CHARGE:
		strncpy(drive_state, "PRECRG", sizeof(drive_state));
		break;
	case DRIVE:
		if( buttons.wheel.brake_swap == BUTTON_IS_PRESSED)
				strncpy(drive_state, "REGEN ", sizeof(drive_state));
		else if( buttons.panel.drv_forward == BUTTON_IS_PRESSED )
		{
			if( buttons.wheel.cruise_on == BUTTON_IS_PRESSED )
				strncpy(drive_state, "CRUISE", sizeof(drive_state));
			else
				strncpy(drive_state, "FORWRD", sizeof(drive_state));
		}
		else if( buttons.panel.drv_reverse == BUTTON_IS_PRESSED )
				strncpy(drive_state, "REVERS", sizeof(drive_state));
		else
				strncpy(drive_state, "NEUTRU", sizeof(drive_state));
		break;
	case ERR:
		break;
	}

	float mppt_power = ( (can_data.mppt1.output_current * can_data.mppt1.output_voltage) +
						 (can_data.mppt2.output_current * can_data.mppt2.output_voltage) +
						 (can_data.mppt3.output_current * can_data.mppt3.output_voltage) +
						 (can_data.mppt4.output_current * can_data.mppt4.output_voltage) );

#if( GUN_POINT_ROAD_TESTING == 1)
	if( buttons.wheel.cruise_down == BUTTON_IS_PRESSED)
	snprintf(buffer,21,"%5.1f FAN %s %s", 	mppt_power,
										  	GetSign(buttons.wheel.blink_left,
											buttons.wheel.blink_right,
											buttons.wheel.avarie),
											drive_state);
	else
	snprintf(buffer,21,"%5.1f     %s %s", 	mppt_power,
											GetSign(buttons.wheel.blink_left,
											buttons.wheel.blink_right,
											buttons.wheel.avarie),
											drive_state);
#endif

	HD44780_PrintStr(buffer);

}

void MPPT_Display(char *buffer)
{
	if (can_data.mppt1.output_current >= MPPT_SIGN_ERROR_VALUE)
		can_data.mppt1.output_current = 0;

	HD44780_SetCursor(0, 0);
	snprintf(buffer, 21, "MPPT1:  %6.2f      ",
			can_data.mppt1.output_current * can_data.mppt1.output_voltage);
	HD44780_PrintStr(buffer);

	if (can_data.mppt2.output_current >= MPPT_SIGN_ERROR_VALUE)
		can_data.mppt2.output_current = 0;

	HD44780_SetCursor(0, 1);
	snprintf(buffer, 21, "MPPT2:  %6.2f      ",
			can_data.mppt2.output_current * can_data.mppt2.output_voltage);
	HD44780_PrintStr(buffer);

	if (can_data.mppt3.output_current >= MPPT_SIGN_ERROR_VALUE)
		can_data.mppt3.output_current = 0;
	HD44780_SetCursor(0, 2);
	snprintf(buffer, 21, "MPPT3:  %6.2f      ",
			can_data.mppt3.output_current * can_data.mppt3.output_voltage);
	HD44780_PrintStr(buffer);

	if (can_data.mppt4.output_current >= MPPT_SIGN_ERROR_VALUE)
			can_data.mppt4.output_current = 0;
		HD44780_SetCursor(0, 3);
		snprintf(buffer, 21, "MPPT4:  %6.2f      ",
				can_data.mppt4.output_current * can_data.mppt4.output_voltage);
		HD44780_PrintStr(buffer);

}



void BOOT_Display(char* buffer)
{

	HD44780_SetCursor(0, 0);
	snprintf(buffer, 21, "SD: %s Ora:%02d.%02d.%02d",      GetString(ActivityCheck.sd_card),
														   RealTimeClock.hour,
														   RealTimeClock.minutes,
														   RealTimeClock.seconds);
	HD44780_PrintStr(buffer);

	HD44780_SetCursor(0, 1);
	snprintf(buffer,21,"INV: %s   BMS: %s", GetString(ActivityCheck.invertor), GetString(ActivityCheck.bms));
	HD44780_PrintStr(buffer);

	HD44780_SetCursor(0,2);
	snprintf(buffer,21,"TEL: %s   AUX: %s ", GetString(ActivityCheck.telemetry), GetString(ActivityCheck.auxiliary));
	HD44780_PrintStr(buffer);

	HD44780_SetCursor(0, 3);
	snprintf(buffer,21,"SP: %s %s %s %s ",  GetString(ActivityCheck.mppt1),
											GetString(ActivityCheck.mppt2),
											GetString(ActivityCheck.mppt3),
											GetString(ActivityCheck.mppt4));

	HD44780_PrintStr(buffer);

	memset(&ActivityCheck, 0, sizeof(ActivityCheck)); //set all the flags to 0

}


void Pop_Up_Error_Display(char* buffer)
{

}

char* GetString(uint8_t status)
{
	return ( status == 1 ) ? " ON": "OFF";
}

char* GetSign(uint8_t status_left, uint8_t status_right,uint8_t status_avarie)
{
	static char buffer[3];

	if( status_avarie == BUTTON_IS_PRESSED )
	{
		buffer[0] = '!';
		buffer[1] = '!';
		buffer[2] = '!';
	}

	else if( status_left == BUTTON_IS_PRESSED )
	{
		buffer[0] = '<';
		buffer[1] = '<';
		buffer[2] = '<';
	}
	else if( status_right == BUTTON_IS_PRESSED )
	{
		buffer[0] = '>';
		buffer[1] = '>';
		buffer[2] = '>';
	}
	else
	{
		buffer[0] = ' ';
		buffer[1] = ' ';
		buffer[2] = ' ';
	}

	return buffer;
}


