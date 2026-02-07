/*
 * power_sensor.h
 *
 *  Created on: Jan 7, 2026
 *      Author: samue
 */

#ifndef INC_POWER_SENSOR_H_
#define INC_POWER_SENSOR_H_

extern I2C_HandleTypeDef hi2c2;

void initialize_all_ina();
uint16_t readVal(uint16_t devAddress , uint8_t reg);
void debugVals(uint16_t devAddress );


#endif /* INC_POWER_SENSOR_H_ */
