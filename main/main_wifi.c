/*
 * main_wifi.c
 *
 *  Created on: 24 mrt. 2020
 *      Author: Jan
 */
#include "switch_config.h"
#include "FreeRTOS/FreeRTOS.h"
#include "FreeRTOS/timers.h"
#include "string.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_err.h"
#include "esp_event_loop.h"
#include "tcpip_adapter.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "server.h"
#include "setting.h"

enum{
	ConnectionClosed,
	ConnectionMaking,
	ConnectionConnected,
	ConnectionFailed,
	ConnectionClosing
} mConnectionStatus;

bool xWifiConnected(){
	return (mConnectionStatus == ConnectionConnected);
}

void sConnectionSuccess(){
	nvs_handle lHandle;
	esp_err_t lResult;
	int8 lFailCount;

	lResult = nvs_open("fail", NVS_READWRITE, &lHandle);
	if (lResult == ESP_OK){
		lResult = nvs_get_i8(lHandle, "count", &lFailCount);
		if (lResult == ESP_OK){
			if (lFailCount != 0){
				lFailCount = 0;
				lResult = nvs_set_i8(lHandle, "count", lFailCount);
				lResult = nvs_commit(lHandle);
			}
		}
		nvs_close(lHandle);
	}
}

int8 xWifiFail(){
	nvs_handle lHandle;
	esp_err_t lResult;
	int8 lFailCount;
	bool lReset;

	mConnectionStatus = ConnectionFailed;
	lReset = false;
	lResult = nvs_open("fail", NVS_READWRITE, &lHandle);
	if (lResult == ESP_OK){
		lResult = nvs_get_i8(lHandle, "count", &lFailCount);
		if (lResult != ESP_OK){
			lFailCount = 0;
		}
		lFailCount++;
		if (lFailCount > 4){
			lReset = true;
		}
		lResult = nvs_set_i8(lHandle, "count", lFailCount);
		lResult = nvs_commit(lHandle);
		nvs_close(lHandle);
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
			mConnectionStatus = ConnectionConnected;
			sConnectionSuccess();
			xStartServer();
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
			switch (mConnectionStatus){
			case ConnectionConnected:
				mConnectionStatus = ConnectionMaking;
				xStopServer();
				printf("Retry Connect\n");
				lResult = esp_wifi_connect();
				ESP_ERROR_CHECK(lResult);
				break;
			case ConnectionClosing:
				printf("Disconnect, no retry!\n");
				break;
			default:
				printf("Retry Connect\n");
				lResult = esp_wifi_connect();
				ESP_ERROR_CHECK(lResult);
				break;
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
	mConnectionStatus = ConnectionConnected;
	xStartServer();
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

	lResult = esp_wifi_set_storage(WIFI_STORAGE_RAM);
	ESP_ERROR_CHECK(lResult);
	memset(&lWifiConfig, 0, sizeof(lWifiConfig));
	memcpy(lWifiConfig.sta.ssid, xSettingSsId(), strlen(xSettingSsId()));
	memcpy(lWifiConfig.sta.password, xSettingPassword(), strlen(xSettingPassword()));
	printf("Setting WiFi Station on SSID: %s\n", lWifiConfig.sta.ssid);
	lResult = esp_wifi_set_mode(WIFI_MODE_STA);
	ESP_ERROR_CHECK(lResult);
	lResult = esp_wifi_set_config(ESP_IF_WIFI_STA, &lWifiConfig);
	ESP_ERROR_CHECK(lResult);

	memcpy(lMac, xSettingMac(), sizeof(lMac));
	if (xSettingMacPresent()){
		lResult = esp_wifi_set_mac(ESP_IF_WIFI_STA, xSettingMac());
	}

	lResult = esp_wifi_start();
	ESP_ERROR_CHECK(lResult);
}

void xWifiStart(){
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

void xWifiInit(){
	mConnectionStatus = ConnectionClosed;
}

