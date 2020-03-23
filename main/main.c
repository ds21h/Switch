#include "switch_config.h"
#include "FreeRTOS/FreeRTOS.h"
#include "FreeRTOS/timers.h"
#include <esp_wifi.h>
#include <esp_event_loop.h>
#include <nvs_flash.h>
#include "nvs.h"
#include <sys/param.h>
#include "esp_system.h"
#include "esp_spi_flash.h"
#include <esp_http_server.h>
#include "server.h"
#include "switch.h"
#include "setting.h"
#include "tcpip_adapter.h"
#include "main_async.h"

TimerHandle_t mHartBeat;
int mStartCounter;
bool mConnected;
bool mConnectionFailed;
httpd_handle_t mServer = NULL;

void sConnectionSuccess(){
	nvs_handle lHandle;
	esp_err_t lResult;
	int8 lFailCount;

	printf("Init setting\n");
	lResult = nvs_open("fail", NVS_READWRITE, &lHandle);
	if (lResult == ESP_OK){
		printf("NVS opened\n");
		lResult = nvs_get_i8(lHandle, "count", &lFailCount);
		printf("Count read, Error code : %d\n", lResult);
		if (lResult == ESP_OK){
			if (lFailCount != 0){
				lFailCount = 0;
				lResult = nvs_set_i8(lHandle, "count", lFailCount);
				printf("Count reset, Error code : %d\n", lResult);
				lResult = nvs_commit(lHandle);
				printf("Commit, Error code : %d\n", lResult);
			}
		}
		nvs_close(lHandle);
		printf("NVS closed\n");
	}
}

int8 sConnectionFailed(){
	nvs_handle lHandle;
	esp_err_t lResult;
	int8 lFailCount;
	bool lReset;

	printf("Set failed count\n");
	lReset = false;
	lResult = nvs_open("fail", NVS_READWRITE, &lHandle);
	if (lResult == ESP_OK){
		printf("NVS opened\n");
		lResult = nvs_get_i8(lHandle, "count", &lFailCount);
		printf("Count read, Error code : %d\n", lResult);
		if (lResult != ESP_OK){
			lFailCount = 0;
		}
		lFailCount++;
		if (lFailCount > 4){
			lReset = true;
		}
		lResult = nvs_set_i8(lHandle, "count", lFailCount);
		printf("Count write, Error code : %d\n", lResult);
		lResult = nvs_commit(lHandle);
		printf("Commit, Error code : %d\n", lResult);
		nvs_close(lHandle);
		printf("NVS closed\n");
	} else {
		lFailCount = 0;
	}
	if (lReset){
		xSettingSetSsId("");
		xSettingWrite();
		lFailCount = -1;
	}
	return lFailCount;
}

static void hStationHandler(void* arg, esp_event_base_t pEventBase, int32_t pEventID, void* pEventData){
	esp_err_t lResult;
	wifi_event_sta_disconnected_t * lDisconnect;
	ip_event_got_ip_t * lGotIP;

	if (pEventBase == IP_EVENT){
		if (pEventID == IP_EVENT_STA_GOT_IP){
			lGotIP = (ip_event_got_ip_t *)pEventData;
			printf("Station got IP '%s'\n", ip4addr_ntoa(&lGotIP->ip_info.ip));
			mConnected = true;
			sConnectionSuccess();

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
			if (mConnectionFailed){
				lResult = esp_wifi_stop();
				ESP_ERROR_CHECK(lResult);
				printf("Wifi stopped\n");
			} else {
				printf("Retry Connect\n");
				lResult = esp_wifi_connect();
				ESP_ERROR_CHECK(lResult);
			}
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
	uint8 lMac[6];

	lResult = esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &hAPHandler, NULL);
	ESP_ERROR_CHECK(lResult);
	lConfig = (wifi_init_config_t)WIFI_INIT_CONFIG_DEFAULT();
	lResult = esp_wifi_init(&lConfig);
	ESP_ERROR_CHECK(lResult);
	lResult = esp_wifi_set_storage(WIFI_STORAGE_RAM);
	ESP_ERROR_CHECK(lResult);
	memset(&lWifiConfig, 0, sizeof(lWifiConfig));
	esp_wifi_get_mac(ESP_IF_WIFI_AP, lMac);
	sprintf((char *)lWifiConfig.ap.ssid, "EspSw_%02x%02x%02x%02x%02x%02x", MAC2STR(lMac));
	sprintf((char *)lWifiConfig.ap.password, "%s", "EspSwSetup");
	lWifiConfig.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
	lWifiConfig.ap.max_connection = 1;
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
	uint8 lMac[6];

	lResult = esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &hStationHandler, NULL);
	ESP_ERROR_CHECK(lResult);
	lResult = esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &hStationHandler, NULL);
	ESP_ERROR_CHECK(lResult);
	lConfig = (wifi_init_config_t)WIFI_INIT_CONFIG_DEFAULT();
	lResult = esp_wifi_init(&lConfig);
	ESP_ERROR_CHECK(lResult);

	printf("Test MAC\n");
	memcpy(lMac, xSettingMac(), sizeof(lMac));
	printf("MAC of Setting: %02x%02x%02x%02x%02x%02x\n", MAC2STR(lMac));
	if (xSettingMacPresent()){
		lResult = esp_wifi_set_mac(ESP_IF_WIFI_STA, xSettingMac());
		printf("Set MAC. Result %d\n", lResult);
	}
	esp_wifi_get_mac(ESP_IF_WIFI_STA, lMac);
	printf("MAC of station: %02x%02x%02x%02x%02x%02x\n", MAC2STR(lMac));

	lResult = esp_wifi_set_storage(WIFI_STORAGE_RAM);
	ESP_ERROR_CHECK(lResult);
	memset(&lWifiConfig, 0, sizeof(lWifiConfig));
	memcpy(lWifiConfig.sta.ssid, xSettingSsId(), strlen(xSettingSsId()));
	memcpy(lWifiConfig.sta.password, xSettingPassword(), strlen(xSettingPassword()));
	printf("Setting WiFi Station on SSID: %s,\nPassword: %s\n", lWifiConfig.sta.ssid, lWifiConfig.sta.password);
	lResult = esp_wifi_set_mode(WIFI_MODE_STA);
	ESP_ERROR_CHECK(lResult);
	lResult = esp_wifi_set_config(ESP_IF_WIFI_STA, &lWifiConfig);
	ESP_ERROR_CHECK(lResult);

	printf("Test MAC\n");
	memcpy(lMac, xSettingMac(), sizeof(lMac));
	printf("MAC of Setting: %02x%02x%02x%02x%02x%02x\n", MAC2STR(lMac));
	if (xSettingMacPresent()){
		lResult = esp_wifi_set_mac(ESP_IF_WIFI_STA, xSettingMac());
		printf("Set MAC. Result %d\n", lResult);
	}
	esp_wifi_get_mac(ESP_IF_WIFI_STA, lMac);
	printf("MAC of station: %02x%02x%02x%02x%02x%02x\n", MAC2STR(lMac));

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
	int8 lFailCount;

	mStartCounter++;
	if (mConnected) {
		lTest = (mStartCounter / 10) * 10;
		if (mStartCounter == lTest) {
			printf("Counting....%d\n", mStartCounter);
		}
	} else {
		if (mStartCounter == STARTPAUSE + 30){
			mConnectionFailed = true;
			lFailCount = sConnectionFailed();
			if (lFailCount < 0){
				printf("Connection failed too often. Reset!\n");
			} else {
				printf("Connection failed. Counter: %d\n", lFailCount);
			}
		}
		printf("Counting....%d\n", mStartCounter);
	}

	if (mStartCounter == STARTPAUSE){
		printf("SDK version:%s\n", esp_get_idf_version());
	    printf("Flash chip %dMB\n", spi_flash_get_chip_size() / (1024 * 1024));
	    printf("Ticks per second: %d\n", pdMS_TO_TICKS(1000));
	    xAsyncInit();
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
    xSettingInit();
	mHartBeat = xTimerCreate("HartBeat", pdMS_TO_TICKS(1000), pdTRUE, (void *)0, tcbHeartBeat);
	mStartCounter = 0;
	mConnected = false;
	mConnectionFailed = false;
	xTimerStart(mHartBeat, 100);
}
