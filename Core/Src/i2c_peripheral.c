/*
 * i2c_peripheral.c
 *
 *  Created on: Dec 30, 2025
 *      Author: samue
 */


#include "main.h"
#include "i2c_peripheral.h"
#include "pwmManager.h"
#include "power_sensor.h"



extern I2C_HandleTypeDef hi2c1; //external variable

#define RxSize 256
uint8_t RxData[ RxSize ];
uint8_t rxCount = 0;


int countAddr = 0;
int countrxcplt = 0;
int countError = 0;
uint32_t errorcode;

void process_data(){

	int startREG = RxData[0];

	for( int i = 1; i < rxCount; i++ ){
		registers[ startREG++ ] = RxData[ i ];
		//Add logic in case of wrapping around??
	}

	update_servos( RxData[0] , rxCount );

	memset( RxData , 0, RxSize );
	rxCount = 0;
	countError = 0;

	//debugVals( (0x40 << 1) );


}


// Functions available: HAL_I2C_Init

void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c){
	HAL_I2C_EnableListen_IT( hi2c );
}

void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode){
	if( TransferDirection == I2C_DIRECTION_TRANSMIT ){
		rxCount = 0;
		HAL_I2C_Slave_Seq_Receive_IT( hi2c, &RxData[rxCount] , 1, I2C_FIRST_FRAME ); //Receive the address
	}else{
		Error_Handler();
	}
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c){

	//This means that the RxSize was matched
	rxCount++; // There was an additional byte added. GOOD
	if( rxCount < RxSize ){
		if( rxCount < 5 ){
			HAL_I2C_Slave_Seq_Receive_IT( hi2c, &RxData[rxCount] , 1, I2C_NEXT_FRAME );
		}else{
			HAL_I2C_Slave_Seq_Receive_IT( hi2c, &RxData[rxCount] , 1, I2C_LAST_FRAME );
			process_data();
		}

	}


}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c){
	countError++;
	errorcode = HAL_I2C_GetError( hi2c );
	if( errorcode == 4 ){ //Acknowledgment
		//process_data();
	}
	HAL_I2C_EnableListen_IT( hi2c );
}
