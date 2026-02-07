/*
 * power_sensor.c
 *
 *  Created on: Jan 7, 2026
 *      Author: samue
 */


#include "main.h"
#include "power_sensor.h"


extern osThreadId ina226AlertHandle;


typedef struct {
	uint16_t address;
	double maxCurrent;
	double rShunt;
	double maxPower;
} ina_obj ;


ina_obj inaSet[1];

uint16_t debug[8];


void errorCall(){
	//Breakpoint
}


void initialize_all_ina(){
	inaSet[0] = (ina_obj){ (0x40 << 1) , 1.0 , 0.02 , 0.25 };
	initialize_ina( inaSet[0].address, inaSet[0].maxCurrent, inaSet[0].rShunt, inaSet[0].maxPower);
}


//Initialize Device Registers
void initialize_ina( uint16_t devAddress, double maxCurrent, double rShunt, double maxPower ){
	/*
		HAL_StatusTypeDef HAL_I2C_Mem_Write(
			I2C_HandleTypeDef *hi2c,
			uint16_t DevAddress, ==> address of the target
			uint16_t MemAddress, ==> address internal memory
			uint16_t MemAddSize, ==> size of memory address: 8 bits
			uint8_t *pData, ==> pointer to the data buffer
			uint16_t Size, ==> amount of data to be sent (sizeof(dataBuffer))
			uint32_t Timeout ==> HAL status??
		)
	*/
	//Resetting the ina226 just in case
	uint8_t buffer[2] = { 0x80 , 0x00 };
	if(HAL_I2C_Mem_Write( &hi2c2, devAddress, 0x00, I2C_MEMADD_SIZE_8BIT, buffer, sizeof(buffer), HAL_MAX_DELAY) != HAL_OK ){
		errorCall();
	}

	HAL_Delay(10);


	buffer[0] = 0x41;
	buffer[1] = 0x27;
	//Writing the configuration register
	if( HAL_I2C_Mem_Write( &hi2c2, devAddress, 0x00, I2C_MEMADD_SIZE_8BIT, buffer, sizeof(buffer), HAL_MAX_DELAY) != HAL_OK ){
		errorCall();
	}

	HAL_Delay(10);


	double current_lsb = maxCurrent / 32768.0;
	uint16_t calibrationReg = (uint16_t)( 0.00512 / ( current_lsb * rShunt ) );
	buffer[0] = (uint8_t) (calibrationReg >> 8); //Most significant bit
	buffer[1] = (uint8_t) (calibrationReg & 0xFF); //Lease Significant byte
	if( HAL_I2C_Mem_Write( &hi2c2, devAddress, 0x05, I2C_MEMADD_SIZE_8BIT, buffer, sizeof(buffer), HAL_MAX_DELAY) != HAL_OK ){
		errorCall();
	}

	HAL_Delay(10);

	//Want the alert when the power is over the limit, bit 11
	buffer[0] = 0x08;
	buffer[1] = 0x00;
	if(HAL_I2C_Mem_Write( &hi2c2, devAddress, 0x06, I2C_MEMADD_SIZE_8BIT, buffer, sizeof(buffer), HAL_MAX_DELAY) != HAL_OK ){
		errorCall();
	}

	HAL_Delay(10);

	//Alert limit register ==> what to compare it to

	double power_lsb = 25.0 * current_lsb;
	//uint16_t alertLimit = (uint16_t)( maxPower / power_lsb );
	uint16_t alertLimit = 700;
	buffer[0] = (uint8_t) (alertLimit >> 8);
	buffer[1] = (uint8_t) (alertLimit & 0xFF);
	if(HAL_I2C_Mem_Write( &hi2c2, devAddress, 0x07, I2C_MEMADD_SIZE_8BIT, buffer, sizeof(buffer), HAL_MAX_DELAY) != HAL_OK ){
		errorCall();
	}


	HAL_Delay(10);


	//Completed the setup for the INA226 hopefully?
	debugVals( devAddress );

}


//
uint16_t readVal(uint16_t devAddress , uint8_t reg){
	uint8_t buffer[2];
	HAL_I2C_Mem_Read(
			&hi2c2,
			devAddress,
			reg,
			I2C_MEMADD_SIZE_8BIT,
			buffer,
			sizeof(buffer),
			HAL_MAX_DELAY
			);
	return ((uint16_t)buffer[0] << 8) | buffer[1];
}


// Debugging values
void debugVals(uint16_t devAddress ){
	 debug[0] = readVal( devAddress , 0x00); //Configuration
	 debug[1] = readVal( devAddress , 0x01); //Shunt
	 debug[2] = readVal( devAddress , 0x02); //Vbus
	 debug[3] = readVal( devAddress , 0x03); //Power
	 debug[4] = readVal( devAddress , 0x04); //Current
	 debug[5] = readVal( devAddress , 0x05); //Callibration
	 debug[6] = readVal( devAddress , 0x06); //Mask
	 debug[7] = readVal( devAddress , 0x07); //Limit
}


// ISR is interrupt service routine
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){

	switch (GPIO_Pin){
		case INA226_Alert_Pin:
			if( ina226AlertHandle != NULL ){
				BaseType_t ina226Woken = pdFALSE;

				//debug[6] = readVal( (0x40 << 1) , 0x06);
				//Send the data to the INA226!
				vTaskNotifyGiveFromISR( (TaskHandle_t)ina226AlertHandle, &ina226Woken );

				portYIELD_FROM_ISR( ina226Woken );
			}

		default:
			//debug[6] = readVal( (0x40 << 1) , 0x06);
	}

}

