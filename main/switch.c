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

void sSwitchSet(bool pValue){
	mSwitchStatus = pValue;

//	GPIO_OUTPUT_SET(mSwitchPort, mSwitchStatus ? mOn : mOff);
}

void xSwitchOn(){
	sSwitchSet(true);
//	xInitOff(pAutoOff);
}

void xSwitchOff(){
	sSwitchSet(false);
}
