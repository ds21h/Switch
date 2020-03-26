/*
 * main_time.c
 *
 *  Created on: 25 mrt. 2020
 *      Author: Jan
 */
#include "switch_config.h"
#include "FreeRTOS/FreeRTOS.h"
#include "FreeRTOS/timers.h"
#include "esp_timer.h"
#include "lwip/apps/sntp.h"
#include "time.h"


void xTimeString(int32 pTime, char * pResult, uint8 pLength){
	struct tm lTime;
	time_t lTimeIn;

	if (pLength < 20){
		pResult = "";
	} else {
		lTimeIn = pTime;
		localtime_r(&lTimeIn, &lTime);
		strftime(pResult, pLength, "%Y-%m-%d %H:%M:%S", &lTime);
	}
}

int32 xTimeNow(){
	time_t lTime;

	time(&lTime);
	if (lTime < 0){
		lTime = 0;
	}
	return (int32)lTime;
}

void xTimeInit(){
	sntp_setservername(0, "0.nl.pool.ntp.org");
	sntp_setservername(1, "1.nl.pool.ntp.org");
	sntp_setservername(2, "2.nl.pool.ntp.org");
	sntp_init();
	setenv("TZ", "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00", 1);
	tzset();
}
