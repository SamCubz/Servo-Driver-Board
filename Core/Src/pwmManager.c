/*
 * pwmManager.c
 *
 *  Created on: Dec 30, 2025
 *      Author: samue
 */


#include "pwmManager.h"
#include "main.h"
#include "i2c_peripheral.h"
#include <math.h>

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;


uint8_t registers[256] = {0};
uint16_t pulseLength;

typedef struct{
	uint8_t * servoRegisters;
	TIM_HandleTypeDef * htim;
	uint32_t Channel;
} servo;

servo servo_list[16];

void start_servo( servo * servo1 ){
	HAL_TIM_PWM_Start( servo1->htim, servo1->Channel);
}

void servo_init(){
	servo_list[0] = (servo){ &registers[6] , &htim2 , TIM_CHANNEL_1 };
	start_servo( &servo_list[0] );
}



//HAL_StatusTypeDef HAL_TIM_OC_Start(TIM_HandleTypeDef *htim, uint32_t Channel);
//HAL_StatusTypeDef HAL_TIM_OC_Stop(TIM_HandleTypeDef *htim, uint32_t Channel);

uint16_t angle_to_counts( uint16_t angle ){
	uint16_t min_counts = 210; //204.8
	//mid_counts is 307.2
	uint16_t max_counts = 405; //409.6

	if( angle > 180 ){
		angle = 180;
	}else if( angle < 0 ){
		angle = 0;
	}

	return min_counts + (angle * (max_counts - min_counts))/ 180;

}

void set_pulse( servo * servos ){

	uint8_t LEDx_ON_L = servos->servoRegisters[0];
	uint8_t LEDx_ON_H = servos->servoRegisters[1];
	uint8_t LEDx_OFF_L = servos->servoRegisters[2];
	uint8_t LEDx_OFF_H = servos->servoRegisters[3];

	uint16_t LEDx_ON = (LEDx_ON_H & 0x0F) << 8 | LEDx_ON_L;
	uint16_t LEDx_OFF = (LEDx_OFF_H & 0x0F) << 8 | LEDx_OFF_L;

	uint16_t pulseLength;
	if( (LEDx_OFF_H >> 4) & 0x01 ){ //Full ON
		pulseLength = 0;
	}else if( (LEDx_ON_H >> 4) & 0x01 ){ //Full ON
		pulseLength = 4095;
	}else{
		pulseLength = ( LEDx_OFF - LEDx_ON ) & 0x0FFF;
	}

	__HAL_TIM_SET_COMPARE( servos->htim , servos->Channel , pulseLength );
}

void update_servos( uint8_t startReg, uint8_t rxCount ){

	//rxCount == 5!

	//Data is already loaded, just need to update the actual servos and set the pulse!
	//Find the servo number!
	uint8_t servo_beg = (startReg - 6) / 4;
	uint8_t servo_end = (startReg + rxCount - 2 - 6) / 4;
	for( int i = servo_beg; i <= servo_end;  i++ ){
		set_pulse( &servo_list[i] );
	}
}
