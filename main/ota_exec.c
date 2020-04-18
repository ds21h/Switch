/*
 * ota_exec.c
 *
 *  Created on: 12 apr. 2020
 *      Author: Jan
 */
// Copyright 2017-2018 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "switch_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ota_exec.h"
#include "xesp_ota_ops.h"
#include <esp_log.h>
#include "setting.h"

#define OTA_BUF_SIZE    256
static const char *TAG = "OtaExec";

static void sHttpCleanup(esp_http_client_handle_t pClient)
{
    esp_http_client_close(pClient);
    esp_http_client_cleanup(pClient);
}

esp_err_t hHttpEvent(esp_http_client_event_t *pEvt){
    switch(pEvt->event_id) {
        case HTTP_EVENT_ERROR:
            printf("HTTP_EVENT_ERROR\n");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            printf("HTTP_EVENT_ON_CONNECTED\n");
            break;
        case HTTP_EVENT_HEADER_SENT:
            printf("HTTP_EVENT_HEADER_SENT\n");
            break;
        case HTTP_EVENT_ON_HEADER:
            printf("HTTP_EVENT_ON_HEADER, key=%s, value=%s\n", pEvt->header_key, pEvt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            printf("HTTP_EVENT_ON_DATA, len=%d\n", pEvt->data_len);
            break;
        case HTTP_EVENT_ON_FINISH:
            printf("HTTP_EVENT_ON_FINISH\n");
            break;
        case HTTP_EVENT_DISCONNECTED:
            printf("HTTP_EVENT_DISCONNECTED\n");
            break;
    }
    return ESP_OK;
}

esp_err_t xOtaExec(const char * pVersion){
    esp_http_client_handle_t lClient;
    esp_err_t lResult;
    esp_err_t lResultWrite;
    esp_ota_handle_t lUpdateHandle = 0;
    const esp_partition_t *lUpdatePartition = NULL;
    char *lUpgradeBuffer;
    int lImageLength;
    int lDataRead;
//    int lImageNumber;
	esp_http_client_config_t lConfig;
	char lUrl[64];
	int lPos;

    ESP_LOGI(TAG, "Starting OTA...");
    lUpdatePartition = esp_ota_get_next_update_partition(NULL);
    if (lUpdatePartition == NULL) {
        ESP_LOGE(TAG, "Passive OTA partition not found");
//        sHttpCleanup(lClient);
        return ESP_FAIL;
/*    } else {
    	if (lUpdatePartition == "ota_0"){
    		lImageNumber = 1;
    	} else {
    		lImageNumber = 2;
    	} */
    }

	strcpy(lUrl, "http://");
	lPos = strlen(lUrl);
	xSettingServerIpDisp(lUrl + lPos);
	lPos = strlen(lUrl);
	sprintf(lUrl + lPos, ":%d/EspServer/Fota/EspIdf/Switch/%s", xSettingServerPort(), pVersion);
	printf("Url: %s\n", lUrl);
	memset(&lConfig, 0, sizeof(lConfig));
    lConfig.url = lUrl;
    lConfig.event_handler = hHttpEvent;

    lClient = esp_http_client_init(&lConfig);
    if (lClient == NULL) {
        ESP_LOGE(TAG, "Failed to initialise HTTP connection");
        return ESP_FAIL;
    }

    lResult = esp_http_client_open(lClient, 0);
    if (lResult != ESP_OK) {
        esp_http_client_cleanup(lClient);
        ESP_LOGE(TAG, "Failed to open HTTP connection: %d", lResult);
        return lResult;
    }
    esp_http_client_fetch_headers(lClient);
    ESP_LOGI(TAG, "Writing to partition subtype %d at offset 0x%x",
             lUpdatePartition->subtype, lUpdatePartition->address);

    lResult = esp_ota_begin(lUpdatePartition, OTA_SIZE_UNKNOWN, &lUpdateHandle);
    if (lResult != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_begin failed, error=%d", lResult);
        sHttpCleanup(lClient);
        return lResult;
    }
    ESP_LOGI(TAG, "esp_ota_begin succeeded");
    ESP_LOGI(TAG, "Please Wait. This may take time");

    lResult = ESP_OK;
    lUpgradeBuffer = (char *)malloc(OTA_BUF_SIZE);
    if (lUpgradeBuffer == NULL) {
        ESP_LOGE(TAG, "Couldn't allocate memory to upgrade data buffer");
        return ESP_ERR_NO_MEM;
    }
    lImageLength = 0;
    lResultWrite = ESP_OK;
    while (1) {
        lDataRead = esp_http_client_read(lClient, lUpgradeBuffer, OTA_BUF_SIZE);
        if (lDataRead == 0) {
            ESP_LOGI(TAG, "Connection closed,all data received");
            break;
        }
        if (lDataRead < 0) {
            ESP_LOGE(TAG, "Error: SSL data read error");
            break;
        }
        if (lDataRead > 0) {
            lResultWrite = esp_ota_write( lUpdateHandle, (const void *)lUpgradeBuffer, lDataRead);
            if (lResultWrite != ESP_OK) {
                break;
            }
            lImageLength += lDataRead;
            ESP_LOGD(TAG, "Written image length %d", lImageLength);
        }
    }
    free(lUpgradeBuffer);
    sHttpCleanup(lClient);
    ESP_LOGD(TAG, "Total binary data length written: %d", lImageLength);

    lResult = esp_ota_end(lUpdateHandle);
    if (lResultWrite != ESP_OK) {
        ESP_LOGE(TAG, "Error: esp_ota_write failed! lResult=0x%d", lResultWrite);
        return lResultWrite;
    } else {
    	if (lResult != ESP_OK) {
            ESP_LOGE(TAG, "Error: esp_ota_end failed! lResult=0x%d. Image is invalid", lResult);
            return lResult;
    	}
    }

    lResult = esp_ota_set_boot_partition(lUpdatePartition);
    if (lResult != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_set_boot_partition failed! lResult=0x%d", lResult);
        return lResult;
    }
    ESP_LOGI(TAG, "esp_ota_set_boot_partition succeeded");

    return ESP_OK;
}


