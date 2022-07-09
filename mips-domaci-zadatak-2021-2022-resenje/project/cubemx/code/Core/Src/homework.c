/*
 * homework.c
 *
 *  Created on: Jan 6, 2022
 *      Author: Marko Micovic
 */

#include "homework.h"

#include "FreeRTOS.h"
#include "task.h"

#include <stdlib.h>
#include <string.h>

#include "driver_lcd.h"
#include "driver_uart.h"
#include "driver_motor.h"
#include "driver_temp.h"

FanState fanState = TURNED_OFF;

static uint32_t tempValue;
static char tempText[4];
static void homeworkTask(void *parameters)
{
	char message[16] = "Temperatura: ";
	LCD_CommandEnqueue(LCD_INSTRUCTION,
	LCD_SET_DD_RAM_ADDRESS_INSTRUCTION | 0x00);
	for (uint32_t i = 0; i < 16; i++)
	{
		LCD_CommandEnqueue(LCD_DATA, message[i]);
		UART_AsyncTransmitCharacter(message[i]);
	}

	while (1)
	{
		tempValue = TEMP_GetCurrentValue();
		itoa(tempValue, tempText, 10);

		// control fan speed
		FanState fanStateTarget;
		if (tempValue < 30)
		{
			fanStateTarget = TURNED_OFF;
		}
		else if (tempValue < 35)
		{
			fanStateTarget = SLOW;
		}
		else
		{
			fanStateTarget = FAST;
		}
		for (uint32_t i = 0; i < abs(fanStateTarget - fanState); i++)
		{
			if (fanStateTarget > fanState)
			{
				MOTOR_SpeedIncrease();
			}
			else
			{
				MOTOR_SpeedDecrease();
			}
		}
		fanState = fanStateTarget;

		// write temperature value
		LCD_CommandEnqueue(LCD_INSTRUCTION,
		LCD_SET_DD_RAM_ADDRESS_INSTRUCTION | 0x0D);
		for (uint32_t i = 0; i < strlen(tempText); i++)
		{
			LCD_CommandEnqueue(LCD_DATA, tempText[i]);
			UART_AsyncTransmitCharacter(tempText[i]);
		}

		vTaskDelay(pdMS_TO_TICKS(200));

		// clear temperature value
		LCD_CommandEnqueue(LCD_INSTRUCTION,
		LCD_SET_DD_RAM_ADDRESS_INSTRUCTION | 0x0D);
		for (uint32_t i = 0; i < strlen(tempText); i++)
		{
			LCD_CommandEnqueue(LCD_DATA, ' ');
			UART_AsyncTransmitCharacter('\b');
		}
	}
}

void homeworkInit()
{
	LCD_Init();
	UART_Init();
	MOTOR_Init();
	TEMP_Init();
	xTaskCreate(homeworkTask, "homeworkTask", 64, NULL, 5, NULL);
}

