/*
 * setting.c
 *
 *  Created on: 21 feb. 2020
 *      Author: Jan
 */
#include "switch_config.h"
#include "setting.h"
#include "nvs.h"
#include "esp_system.h"
#include <string.h>

nvs_handle lHandle;

char lSsId[32];
char sPassword[64];
uint8 sMac[6];
char sName[64];
char sDescription[128];
uint8 sLogLevel;
bool sButton;
uint32 sAutoOff;
uint8 sServerIP[4];
int sServerPort;

const char* xSettingName(){
	return "Test Name";
}

const char* xSettingDescription(){
	return "Test Description";
}

uint8 xSettingSwitchModel(){
	return 1;
}

void xSettingInit(){
	esp_err_t lResult;

	lResult = nvs_open("switch", NVS_READWRITE, &lHandle);
	printf("Result Open: %d\n", lResult);
	if (lResult == ESP_OK){
		lResult = nvs_get_str(lHandle, "ssid", lSsId, (size_t *)sizeof(lSsId));
		printf("Result Get SSID: %d\n", lResult);
		if (lResult != ESP_OK){
			memset(lSsId, 0, sizeof(lSsId));
		}
		nvs_close(lHandle);
	}
}
