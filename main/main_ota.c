/*
 * main_ota.c
 *
 *  Created on: 9 apr. 2020
 *      Author: Jan
 */
#include "switch_config.h"
#include "FreeRTOS/FreeRTOS.h"
#include "FreeRTOS/timers.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "esp_http_client.h"
#include "esp_netif.h"
#include "ota_exec.h"
#include "main_ota.h"
#include "setting.h"

void hOtaProcess(void *pVersion){
	int lDelay;
	esp_err_t lResult;

    lDelay = 5;
	while (lDelay > 0){
		printf("Upgrade in %d seconds\n", lDelay);
		vTaskDelay(1000/portTICK_PERIOD_MS);
		lDelay--;
	}
	vTaskDelay(100/portTICK_PERIOD_MS);
    lResult = xOtaExec((char *)pVersion);
    if (lResult == ESP_OK) {
        printf("Firmware Upgrade Success\n");
		vTaskDelay(100/portTICK_PERIOD_MS);
        esp_restart();
    } else {
        printf("Firmware Upgrade Failed\n");
    }
    free(pVersion);
    vTaskDelete(NULL);
    while (1){
    	vTaskDelay(100/portTICK_PERIOD_MS);
    }
}

void xOtaUpgrade(const char * pVersion){
	char * lVersion;

	lVersion = (char *)malloc(strlen(pVersion) + 1);
	strcpy(lVersion, pVersion);
	xTaskCreate(hOtaProcess, "OTA", 8192, lVersion, 6, NULL);
/*	while(1){
		vTaskDelay(100/portTICK_PERIOD_MS);
		if (pData->sStatus > Started){
			break;
		}
	}
	if (pData->sStatus == EndedOK){
        printf("Firmware Upgrade Success\n");
		vTaskDelay(100/portTICK_PERIOD_MS);
        esp_restart();
    } else {
        printf("Firmware Upgrade Failed\n");
    }
    free(pData); */
}
