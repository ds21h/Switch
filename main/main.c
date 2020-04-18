#include "switch_config.h"
#include "FreeRTOS/FreeRTOS.h"
#include "FreeRTOS/timers.h"
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
	const esp_partition_t *lRunningPart;

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
		lRunningPart = esp_ota_get_running_partition();
		if (lRunningPart == NULL){
			printf("No partition info available\n");
		} else {
			printf("Start partition: %s\n", lRunningPart->label);
		}
		printf("SDK version:%s\n", esp_get_idf_version());
	    printf("Flash chip %dMB\n", spi_flash_get_chip_size() / (1024 * 1024));
	    printf("Ticks per second: %d\n", pdMS_TO_TICKS(1000));
	    mCountSingle = false;
	    xAsyncInit();
	    xLogInit();
	    xButtonSet();
	    xWifiStart();
	    xTimeInit();
/*		ets_uart_printf("SDK version:%s\r\n", system_get_sdk_version());
		ets_uart_printf("Flash chip id: %x\r\n", spi_flash_get_id());
		switch (system_upgrade_userbin_check()){
		case UPGRADE_FW_BIN1:
			ets_uart_printf("Image %d\r\n", 1);
			break;
		case UPGRADE_FW_BIN2:
			ets_uart_printf("Image %d\r\n", 2);
			break;
		default:
			ets_uart_printf("Unknown image\r\n");
			break;
		}
		ets_uart_printf("Start configuration\r\n");
		system_os_post(0, EventStartSetup, 0); */
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
