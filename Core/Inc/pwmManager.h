/*
 * pwmManager.h
 *
 *  Created on: Dec 30, 2025
 *      Author: samue
 */

#ifndef INC_PWMMANAGER_H_
#define INC_PWMMANAGER_H_

#include "main.h"

struct servo;
extern uint8_t registers[256];
void servo_init();
void update_servos( uint8_t startReg, uint8_t rxCount );

#endif /* INC_PWMMANAGER_H_ */


