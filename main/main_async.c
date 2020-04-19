/*
 * main_async.c
 *
 *  Created on: 22 mrt. 2020
 *      Author: Jan
 */
#include "switch_config.h"
#include "main_async.h"
#include "FreeRTOS/FreeRTOS.h"
#include "FreeRTOS/timers.h"
#include "FreeRTOS/queue.h"
#include "portmacro.h"
#include "setting.h"
#include "esp_system.h"
#include "esp_http_server.h"
#include "server.h"
#include "main_wifi.h"
#include "main_time.h"
#include "main_ota.h"

QueueHandle_t mAsyncQueue = NULL;

void hAsync(void *pParameters){
	struct QueueItem lItem;
	uint8 lDelay;

	while (1){
		xQueueReceive(mAsyncQueue, &lItem, portMAX_DELAY);
		switch (lItem.qAction){
		case ActionWriteSetting:
			xSettingWrite();
			break;
		case ActionRestart:
			printf("Restart requested\n");
			lDelay = 5;
			while (lDelay > 0){
				if(lDelay == 3){
					xStopServer();
					xTimeStop();
				}
				printf("Restart in %d seconds\n", lDelay);
				vTaskDelay(1000/portTICK_PERIOD_MS);
				lDelay--;
			}
			esp_restart();
			break;
		default:
			printf("Message fromAsyncTask\n");
			break;
		}
	}
}

void xAsyncProcess(struct QueueItem pItem){
	if (mAsyncQueue != NULL){
	    xQueueSend(mAsyncQueue, &pItem, 0);
	}
}

void xAsyncInit(){
	if (mAsyncQueue != NULL){
		vQueueDelete(mAsyncQueue);
	}
	mAsyncQueue = xQueueCreate( 2, sizeof(struct QueueItem));

	if (mAsyncQueue != NULL){
		xTaskCreate(hAsync, "Async", configMINIMAL_STACK_SIZE, NULL, 6, NULL);
	}
}
