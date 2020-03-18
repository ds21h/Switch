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
#include "tcpip_adapter.h"

TimerHandle_t mHartBeat;
int mStartCounter;
bool mConnected;
httpd_handle_t mServer = NULL;

static void hStationHandler(void* arg, esp_event_base_t pEventBase, int32_t pEventID, void* pEventData){
	esp_err_t lResult;
	wifi_event_sta_disconnected_t * lDisconnect;
	ip_event_got_ip_t * lGotIP;
	ip4_addr_t * lIpAddress;

	if (pEventBase == IP_EVENT){
		if (pEventID == IP_EVENT_STA_GOT_IP){
			lGotIP = (ip_event_got_ip_t *)pEventData;
			lIpAddress = &lGotIP->ip_info.ip;
			printf("Station got IP '%s'\n", ip4addr_ntoa(lIpAddress));
			mConnected = true;

			/* Start the web server */
			if (mServer == NULL) {
				mServer = xStartServer();
			}
		}
	} else {
		switch (pEventID){
		case WIFI_EVENT_STA_START:
			printf("Station Start\n");
			lResult = esp_wifi_set_protocol(ESP_IF_WIFI_STA, WIFI_PROTOCAL_11B | WIFI_PROTOCAL_11G | WIFI_PROTOCAL_11N);
			ESP_ERROR_CHECK(lResult);
			lResult = esp_wifi_connect();
			ESP_ERROR_CHECK(lResult);
			break;
		case WIFI_EVENT_STA_CONNECTED:
			printf("Station Connected\n");
			break;
		case WIFI_EVENT_STA_DISCONNECTED:
			lDisconnect = (wifi_event_sta_disconnected_t *)pEventData;
			printf("Station disconnected\n");
			printf("Disconnect reason : %d\n", lDisconnect->reason);
			lResult = esp_wifi_connect();
			ESP_ERROR_CHECK(lResult);
			mConnected = false;

			/* Stop the web server */
			if (mServer != NULL) {
				xStopServer(mServer);
				mServer = NULL;
			}
			break;
		}
	}
}

static void hAPHandler(void* arg, esp_event_base_t pEventBase, int32_t pEventID, void* pEventData){
	wifi_event_ap_staconnected_t* lConnect;
	wifi_event_ap_stadisconnected_t* lDisconnect;

	switch (pEventID){
	case WIFI_EVENT_AP_START:
		printf("AP Start\n");
		break;
	case WIFI_EVENT_AP_STACONNECTED:
        lConnect = (wifi_event_ap_staconnected_t*) pEventData;
		printf("station " MACSTR " join, AID=%d\n", MAC2STR(lConnect->mac), lConnect->aid);
		break;
	case WIFI_EVENT_AP_STADISCONNECTED:
		lDisconnect = (wifi_event_ap_stadisconnected_t*)pEventData;
        printf("station "MACSTR" leave, AID=%d\n", MAC2STR(lDisconnect->mac), lDisconnect->aid);
		break;
	}
}

void sInitialiseAP(){
	wifi_init_config_t lConfig;
	wifi_config_t lWifiConfig;
	esp_err_t lResult;

	lResult = esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &hAPHandler, NULL);
	ESP_ERROR_CHECK(lResult);
	lConfig = (wifi_init_config_t)WIFI_INIT_CONFIG_DEFAULT();
	lResult = esp_wifi_init(&lConfig);
	ESP_ERROR_CHECK(lResult);
	lResult = esp_wifi_set_storage(WIFI_STORAGE_RAM);
	ESP_ERROR_CHECK(lResult);
	lWifiConfig = (wifi_config_t){
        .ap = {
            .ssid = "EspSw001",
            .ssid_len = 8,
            .password = "EspSwSetup",
            .max_connection = 1,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
	};
	printf("Setting WiFi AP on SSID %s...\n", lWifiConfig.ap.ssid);
	lResult = esp_wifi_set_mode(WIFI_MODE_AP);
	ESP_ERROR_CHECK(lResult);
	lResult = esp_wifi_set_config(ESP_IF_WIFI_AP, &lWifiConfig);
	ESP_ERROR_CHECK(lResult);
	lResult = esp_wifi_start();
	ESP_ERROR_CHECK(lResult);
	mConnected = true;

	/* Start the web server */
	if (mServer == NULL) {
		mServer = xStartServer();
	}
}

void sInitialiseStation(){
	wifi_init_config_t lConfig;
	wifi_config_t lWifiConfig;
	esp_err_t lResult;

	lResult = esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &hStationHandler, NULL);
	ESP_ERROR_CHECK(lResult);
	lResult = esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &hStationHandler, NULL);
	ESP_ERROR_CHECK(lResult);
	lConfig = (wifi_init_config_t)WIFI_INIT_CONFIG_DEFAULT();
	lResult = esp_wifi_init(&lConfig);
	ESP_ERROR_CHECK(lResult);
	lResult = esp_wifi_set_storage(WIFI_STORAGE_RAM);
	ESP_ERROR_CHECK(lResult);
	lWifiConfig = (wifi_config_t){ .sta = { .ssid = "JEHok-2",
			.password = "VoulezVousCoucherAvecMoi", }, };
	printf("Setting WiFi Station on SSID %s...\n", lWifiConfig.sta.ssid);
	lResult = esp_wifi_set_mode(WIFI_MODE_STA);
	ESP_ERROR_CHECK(lResult);
	lResult = esp_wifi_set_config(ESP_IF_WIFI_STA, &lWifiConfig);
	ESP_ERROR_CHECK(lResult);
	lResult = esp_wifi_start();
	ESP_ERROR_CHECK(lResult);
}

void sInitialiseWifi(){
	esp_err_t lResult;

	tcpip_adapter_init();
	lResult = esp_event_loop_create_default();
	ESP_ERROR_CHECK(lResult);
	if (strcmp(xSettingSsId(), "") == 0){
		sInitialiseAP();
	} else {
		sInitialiseStation();
	}
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
	    sInitialiseWifi();
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
	xSwitchInit();
	mHartBeat = xTimerCreate("HartBeat", pdMS_TO_TICKS(1000), pdTRUE, (void *)0, tcbHeartBeat);
	mStartCounter = 0;
	mConnected = false;
	xTimerStart(mHartBeat, 100);
}
