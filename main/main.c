#include "switch_config.h"
#include "FreeRTOS/FreeRTOS.h"
#include "FreeRTOS/timers.h"
#include <esp_wifi.h>
#include <esp_event_loop.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "esp_system.h"
#include "esp_spi_flash.h"
#include <esp_http_server.h>
#include "server.h"
#include "switch.h"
#include "setting.h"

TimerHandle_t mHartBeat;
int mStartCounter;
bool mConnected;
httpd_handle_t mServer = NULL;

static esp_err_t hEventHandler(void *pCtx, system_event_t *pEvent) {
	httpd_handle_t *lServer;
	system_event_info_t *lInfo;

	lServer = (httpd_handle_t*) pCtx;
	lInfo = &pEvent->event_info;

	switch (pEvent->event_id) {
	case SYSTEM_EVENT_STA_START:
		printf("Station Start\n");
		ESP_ERROR_CHECK(esp_wifi_connect());
		break;
	case SYSTEM_EVENT_STA_GOT_IP:
		printf("Station got IP '%s'\n", ip4addr_ntoa(&pEvent->event_info.got_ip.ip_info.ip));
		mConnected = true;

		/* Start the web server */
		if (*lServer == NULL) {
			*lServer = xStartServer();
		}
		break;
	case SYSTEM_EVENT_STA_DISCONNECTED:
		printf("Station disconnected\n");
		printf("Disconnect reason : %d\n", lInfo->disconnected.reason);
		if (lInfo->disconnected.reason == WIFI_REASON_BASIC_RATE_NOT_SUPPORT) {
			/*Switch to 802.11 bgn mode */
			esp_wifi_set_protocol(ESP_IF_WIFI_STA,
					WIFI_PROTOCAL_11B | WIFI_PROTOCAL_11G | WIFI_PROTOCAL_11N);
		}
		ESP_ERROR_CHECK(esp_wifi_connect());
		mConnected = false;

		/* Stop the web server */
		if (*lServer) {
			xStopServer(*lServer);
			*lServer = NULL;
		}
		break;
	default:
		break;
	}
	return ESP_OK;
}

static void sInitialise_wifi(void *pArg) {
	wifi_init_config_t lConfig;
	wifi_config_t lWifiConfig;

	tcpip_adapter_init();
	ESP_ERROR_CHECK(esp_event_loop_init(hEventHandler, pArg));
	lConfig = (wifi_init_config_t)WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&lConfig));
	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
	lWifiConfig = (wifi_config_t){ .sta = { .ssid = "LaBurgadeX",
			.password = "", }, };
//	lWifiConfig = (wifi_config_t){ .sta = { .ssid = "JEHok-2",
//			.password = "VoulezVousCoucherAvecMoi", }, };
	printf("Setting WiFi Station on SSID %s...\n", lWifiConfig.sta.ssid);
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &lWifiConfig));
	ESP_ERROR_CHECK(esp_wifi_start());
}

void tcbHeartBeat(TimerHandle_t pTimer) {
	static int lTest;

	mStartCounter++;
	if (mConnected) {
		lTest = (mStartCounter / 10) * 10;
		if (mStartCounter == lTest) {
			printf("Counting....%d\n", mStartCounter);
		}
	} else {
		printf("Counting....%d\n", mStartCounter);
	}

	if (mStartCounter == STARTPAUSE){
		printf("SDK version:%s\n", esp_get_idf_version());
	    printf("Flash chip %dMB\n", spi_flash_get_chip_size() / (1024 * 1024));
	    printf("Ticks per second: %d\n", pdMS_TO_TICKS(1000));
	    xSettingInit();
		sInitialise_wifi(&mServer);
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
	ESP_ERROR_CHECK(nvs_flash_init());
	xSwitchInit();
	mHartBeat = xTimerCreate("HartBeat", pdMS_TO_TICKS(1000), pdTRUE, (void *)0, tcbHeartBeat);
	mStartCounter = 0;
	mConnected = false;
	xTimerStart(mHartBeat, 100);
}
