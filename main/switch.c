/*
 * switch.c
 *
 *  Created on: 22 feb. 2020
 *      Author: Jan
 */
#include "switch_config.h"

static bool mSwitchStatus = false;

bool xSwitchStatus(){
	return mSwitchStatus;
}

