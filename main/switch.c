/*
 * switch.c
 *
 *  Created on: 22 feb. 2020
 *      Author: Jan
 */
#include "switch_config.h"
#include "setting.h"
#include "driver/gpio.h"

static bool mSwitchStatus = false;
static int mSwitchPort;
static int mOn;
static int mOff;

bool xSwitchStatus(){
	return mSwitchStatus;
}

void sSwitchSet(bool pValue){
	mSwitchStatus = pValue;

    gpio_set_level(mSwitchPort, mSwitchStatus ? mOn : mOff);
}

void xSwitchOn(){
	sSwitchSet(true);
//	xInitOff(pAutoOff);
}

void xSwitchOff(){
	sSwitchSet(false);
}

bool xSwitchFlip(){
	sSwitchSet(!mSwitchStatus);
	return mSwitchStatus;
}

void xSwitchInit(){
    gpio_config_t lConf;
    uint32 lPinMask;

	if (xSettingSwitchModel() == 1){
		mSwitchPort = 2;
		mOn = 1;
		mOff = 0;
	} else {
		mSwitchPort = 0;
		mOn = 0;
		mOff = 1;
	}
    lConf.intr_type = GPIO_INTR_DISABLE;
    lConf.mode = GPIO_MODE_OUTPUT;
    lPinMask = 1ULL << mSwitchPort;
    lConf.pin_bit_mask = lPinMask;
    lConf.pull_down_en = 0;
    lConf.pull_up_en = 0;
    gpio_config(&lConf);


	sSwitchSet(false);
}
