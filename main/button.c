#include "switch_config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "driver/gpio.h"
#include "switch.h"
#include "logger.h"
#include "setting.h"

int mButtonPin = -1;
bool mButtonEnabled = false;

TaskHandle_t mProcessTask;

void hButtonProcess(void *pParameters){
	bool lSwitchStatus;
    enum LogItem lLogAction;
    TickType_t lDelay;
    uint8 lHit;
    bool lBlock;

    lHit = 0;
    lBlock = false;
	while (1){
		if (gpio_get_level(mButtonPin) == 0){
			if (!lBlock){
				lHit++;
				if (lHit > 5){
					lSwitchStatus = xSwitchFlip();
					if (lSwitchStatus){
						lLogAction = LogPutSwitchOn;
					} else {
						lLogAction = LogPutSwitchOff;
					}
					xLogEntry(lLogAction, 0);
					lBlock = true;
				}
			}
			lDelay = pdMS_TO_TICKS(50);
		} else {
			lHit = 0;
			lBlock = false;
			lDelay = pdMS_TO_TICKS(250);
		}
		vTaskDelay(lDelay);
	}
}

void sButtonDisable(){
    if (mButtonPin < 0){
    	if (xSettingSwitchModel() == 1){
    		mButtonPin = 0;
    	} else {
    		mButtonPin = 2;
    	}
    }

    if (mProcessTask != NULL){
    	vTaskDelete(mProcessTask);
    	mProcessTask = NULL;
    }
    mButtonEnabled = false;
}

void sButtonEnable(){
    if (mButtonPin < 0){
    	if (xSettingSwitchModel() == 1){
    		mButtonPin = 0;
    	} else {
    		mButtonPin = 2;
    	}
    }

    mProcessTask = NULL;
	xTaskCreate(hButtonProcess, "Button", configMINIMAL_STACK_SIZE, NULL, 6, &mProcessTask);
    mButtonEnabled = true;
}

void xButtonSet(){
	if (xSettingButton() != mButtonEnabled){
		if (xSettingButton()){
			sButtonEnable();
		} else {
			sButtonDisable();
		}
	}
}
