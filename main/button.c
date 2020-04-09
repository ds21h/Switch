#include "switch_config.h"
#include "FreeRTOS/FreeRTOS.h"
#include "FreeRTOS/timers.h"
#include "driver/gpio.h"
#include "switch.h"
#include "logger.h"
#include "setting.h"

int mButtonPin = -1;
bool mInputBlock = true;
bool mButtonEnabled = false;

TaskHandle_t mProcessTask;

void hButtonISR(void *pParameters){
	BaseType_t xHigherPriorityTaskWoken;

	xHigherPriorityTaskWoken = pdFALSE;
	if (!mInputBlock){
		mInputBlock = true;

		xTaskNotifyFromISR(mProcessTask, 0, eNoAction, &xHigherPriorityTaskWoken);
	}
	if(xHigherPriorityTaskWoken){
		portYIELD_FROM_ISR();
	}
}

void hButtonProcess(void *pParameters){
	bool lSwitchStatus;
    enum LogItem lLogAction;

	while (1){
		xTaskNotifyWait(0xffffffff, 0xffffffff, NULL, portMAX_DELAY);
		vTaskDelay(pdMS_TO_TICKS(100));
		if (gpio_get_level(mButtonPin) == 0){
			lSwitchStatus = xSwitchFlip();
			if (lSwitchStatus){
				lLogAction = LogPutSwitchOn;
			} else {
				lLogAction = LogPutSwitchOff;
			}
			xLogEntry(lLogAction, 0);

			vTaskDelay(pdMS_TO_TICKS(400));
		}
		mInputBlock = false;
	}
}

void sButtonDisable(){
    gpio_config_t lConf;
    uint32 lPinMask;

    if (mButtonPin < 0){
    	if (xSettingSwitchModel() == 1){
    		mButtonPin = 0;
    	} else {
    		mButtonPin = 2;
    	}
    }
    lConf.intr_type = GPIO_INTR_DISABLE;
    lConf.mode = GPIO_MODE_DISABLE;
    lPinMask = 1ULL << mButtonPin;
    lConf.pin_bit_mask = lPinMask;
    lConf.pull_down_en = 0;
    lConf.pull_up_en = 0;
    gpio_config(&lConf);

    if (mProcessTask != NULL){
    	vTaskDelete(mProcessTask);
    	mProcessTask = NULL;
    }
    gpio_uninstall_isr_service();
    gpio_isr_handler_remove(mButtonPin);

    mInputBlock = true;
    mButtonEnabled = false;
}

void sButtonEnable(){
    gpio_config_t lConf;
    uint32 lPinMask;

    if (mButtonPin < 0){
    	if (xSettingSwitchModel() == 1){
    		mButtonPin = 0;
    	} else {
    		mButtonPin = 2;
    	}
    }
    lConf.intr_type = GPIO_INTR_NEGEDGE;
    lConf.mode = GPIO_MODE_INPUT;
    lPinMask = 1ULL << mButtonPin;
    lConf.pin_bit_mask = lPinMask;
    lConf.pull_down_en = 0;
    lConf.pull_up_en = 1;
    gpio_config(&lConf);

    mProcessTask = NULL;
	xTaskCreate(hButtonProcess, "Button", configMINIMAL_STACK_SIZE, NULL, 6, &mProcessTask);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(mButtonPin, hButtonISR, NULL);

    mInputBlock = false;
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
