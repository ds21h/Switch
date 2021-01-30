#include "switch_config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "sys/param.h"
#include "esp_system.h"
#include "esp_ota_ops.h"
#include "esp_netif.h"
#include "esp_spi_flash.h"
#include "esp_http_server.h"
#include "switch.h"
#include "setting.h"
#include "main_async.h"
#include "main_wifi.h"
#include "main_time.h"
#include "logger.h"
#include "button.h"

TimerHandle_t mHartBeat;
int mStartCounter;
bool mCountSingle;

void tcbHeartBeat(TimerHandle_t pTimer) {
	static int lTest;
	int8 lFailCount;
	int32 lTime;
	char lTimeS[20];

	xSwitchTimeTick();

	mStartCounter++;
	if (mCountSingle){
		printf("Counting....%d\n", mStartCounter);
	} else {
		lTest = (mStartCounter / 10) * 10;
		if (mStartCounter == lTest) {
			lTime = xTimeNow();
			xTimeString(lTime, lTimeS, sizeof(lTimeS));
			printf("Counting....%d, Time %s\n", mStartCounter, lTimeS);
		}
	}

	if (!xWifiConnected() && mStartCounter == STARTPAUSE + 30) {
		lFailCount = xWifiFail();
		if (lFailCount < 0){
			printf("Connection failed too often. Reset!\n");
		} else {
			printf("Connection failed. Counter: %d\n", lFailCount);
		}
	}

	if (mStartCounter == STARTPAUSE){
		printf("SDK version:%s\n", esp_get_idf_version());
	    printf("Flash chip %dMB\n", spi_flash_get_chip_size() / (1024 * 1024));
	    printf("Ticks per second: %d\n", (int)pdMS_TO_TICKS(1000));
	    mCountSingle = false;
	    xAsyncInit();
	    xLogInit();
	    xButtonSet();
	    xWifiStart();
	    xTimeInit();
	}
}

void app_main() {
	esp_err_t lResult;

	lResult = nvs_flash_init();
	if (lResult == ESP_ERR_NVS_NO_FREE_PAGES) {
	    lResult = nvs_flash_erase();
	    ESP_ERROR_CHECK(lResult);
	    lResult = nvs_flash_init();
	}
	ESP_ERROR_CHECK(lResult);
    xSettingInit();
	xSwitchInit();
    xWifiInit();
	mHartBeat = xTimerCreate("HartBeat", pdMS_TO_TICKS(1000), pdTRUE, (void *)0, tcbHeartBeat);
	mStartCounter = 0;
	mCountSingle = true;
	xTimerStart(mHartBeat, 100);
}
