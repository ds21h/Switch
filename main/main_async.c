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

QueueHandle_t mAsyncQueue = NULL;

void hAsync(void *pParameters){
	struct QueueItem lItem;

	while (1){
		xQueueReceive(mAsyncQueue, &lItem, portMAX_DELAY);
		switch (lItem.qAction){
		case ActionWriteSetting:
			xSettingWrite();
			break;
		case ActionRestart:
			printf("Restart requested\n");
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
	BaseType_t lResult;

	if (mAsyncQueue != NULL){
		printf("Async queue already exists. Delete!\n");
		vQueueDelete(mAsyncQueue);
	}
	mAsyncQueue = xQueueCreate( 2, sizeof(struct QueueItem));
	if (mAsyncQueue == NULL){
		printf("Async queue creation failed\n");
	} else {
		printf("Async queue created\n");
	}

	if (mAsyncQueue != NULL){
		lResult = xTaskCreate(hAsync, "Async", configMINIMAL_STACK_SIZE, NULL, 6, NULL);
		if (lResult == pdPASS){
			printf("Async task created\n");
		} else {
			printf("Async task failed. Result: %d\n", lResult);
		}
	}
}
