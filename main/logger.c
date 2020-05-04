/*
 * logger.c
 *
 *  Created on: 26 mrt. 2020
 *      Author: Jan
 */
#include "switch_config.h"
#include "FreeRTOS/FreeRTOS.h"
#include "FreeRTOS/timers.h"
#include "stdlib.h"
#include "string.h"
#include "esp_system.h"
#include "nvs.h"
#include "main_time.h"
#include "setting.h"
#include "logger.h"

#define LOG_VERSION			100
#define LOG_NUMBER_ENTRIES	250
#define LOG_ENTRIES_WRITE	50		/* Must be a divider of LOG_NUMBER_ENTRIES!! */

static char* mLogText[] = {"None", "Log Init", "Log Cont", "Set Level", "GET Switch", "GET Switch mult.", "GET Switch Err.", "GET Setting", "GET Setting Err.", "GET Log", "GET Log mult.", "GET Log Err.", "Restart", "Restart Err.", "Upgrade", "Upgrade Err.", "Switch On", "Switch Off", "Switch AutoOff", "PUT Switch Err.", "PUT Setting", "PUT Setting Err."};

struct log_entry{
	long sTime;
	uint8 sAction;
	uint32 sIp;
};

struct log_header{
	uint8 sVersion;
	uint8 sNumber;
	uint8 sCurrent;
};

struct log{
	struct log_header sHeader;
	struct log_entry sEntry[LOG_NUMBER_ENTRIES];
} mLog;


uint8 mLogLevel = 0;

void sWriteLog();

int xLogNumber(){
	return mLog.sHeader.sNumber;
}

int xLogCurrent(){
	return mLog.sHeader.sCurrent;
}

enum LogItem xLogAction(int pEntry){
	if (pEntry >= 0 && pEntry < LOG_NUMBER_ENTRIES){
		return mLog.sEntry[pEntry].sAction;
	} else {
		return 0;
	}
}

void xLogActionStr(int pEntry, char * pBuffer, int pLength){
	memset(pBuffer, 0, pLength);
	if (pEntry >= 0 && pEntry < LOG_NUMBER_ENTRIES){
		if (mLog.sEntry[pEntry].sAction < sizeof(mLogText)/sizeof(mLogText[0])){
			strncpy(pBuffer, mLogText[mLog.sEntry[pEntry].sAction], pLength);
			pBuffer[pLength - 1] = 0;
		} else {
			sprintf(pBuffer, "%d", mLog.sEntry[pEntry].sAction);
		}
	} else {
		pBuffer[0] = 0;
	}
}

long xLogTime(int pEntry){
	if (pEntry >= 0 && pEntry < LOG_NUMBER_ENTRIES){
		return mLog.sEntry[pEntry].sTime;
	} else {
		return 0;
	}
}

uint32 xLogIP(int pEntry){
	if (pEntry >= 0 && pEntry < LOG_NUMBER_ENTRIES){
		return mLog.sEntry[pEntry].sIp;
	} else {
		return 0;
	}
}

void xLogEntry(enum LogItem pAction, uint32 pIp){
	int8 lPrev;
	bool lMultiple;

	lMultiple = false;
	lPrev = mLog.sHeader.sCurrent - 1;
	if (lPrev < 0){
		lPrev = mLog.sHeader.sNumber - 1;
	}
	if (mLogLevel > 0){
		if (pAction == LogGetSwitch){
			if (mLog.sEntry[lPrev].sAction == LogGetSwitch || mLog.sEntry[lPrev].sAction == LogGetSwitchMult){
				if (mLog.sEntry[lPrev].sIp == pIp){
					lMultiple = true;
					mLog.sEntry[lPrev].sAction = LogGetSwitchMult;
					mLog.sEntry[lPrev].sTime = xTimeNow();
				}
			}
		}
		if (pAction == LogGetLog){
			if (mLog.sEntry[lPrev].sAction == LogGetLog || mLog.sEntry[lPrev].sAction == LogGetLogMult){
				if (mLog.sEntry[lPrev].sIp == pIp){
					lMultiple = true;
					mLog.sEntry[lPrev].sAction = LogGetLogMult;
					mLog.sEntry[lPrev].sTime = xTimeNow();
				}
			}
		}
		if (!lMultiple){
			mLog.sEntry[mLog.sHeader.sCurrent].sAction = (uint8)pAction;
			mLog.sEntry[mLog.sHeader.sCurrent].sTime = xTimeNow();
			mLog.sEntry[mLog.sHeader.sCurrent].sIp = pIp;
			mLog.sHeader.sCurrent++;
			if (mLog.sHeader.sCurrent >= mLog.sHeader.sNumber){
				mLog.sHeader.sCurrent = 0;
			}
		}
		if (mLogLevel > 1){
			sWriteLog();
		}
	}
}

void xLogSetLevel(uint8 pLevel){
	enum LogItem lAction;

	mLogLevel = 1;
	lAction = LogLevel;
	xLogEntry(lAction, pLevel);

	mLogLevel = pLevel;
}

void sLogInit(){
	enum LogItem lAction;

	memset(&mLog, 0, sizeof(mLog));
    mLog.sHeader.sVersion = LOG_VERSION;
    mLog.sHeader.sNumber = LOG_NUMBER_ENTRIES;
    mLog.sHeader.sCurrent = 0;
	lAction = LogInit;
	xLogEntry(lAction, 0);
}

void sWriteLog(){
	nvs_handle lHandle;
	esp_err_t lResult;
	size_t lLength;
	struct log_entry * lLog;
	uint8 lLogCount;
	char lName[5];

	/* As the nvs_set_blob can take some time insert some delays to prevent watchdog reset!  */
	vTaskDelay(1);
	lResult = nvs_open("switch", NVS_READWRITE, &lHandle);
	if (lResult == ESP_OK){
		vTaskDelay(1);
		lResult = nvs_set_blob(lHandle, "logheader", &mLog.sHeader, sizeof(struct log_header));
		if (lResult == ESP_OK){
			lLength = sizeof(struct log_entry) * LOG_ENTRIES_WRITE;
			for (lLogCount = 0; lLogCount < LOG_NUMBER_ENTRIES/LOG_ENTRIES_WRITE; lLogCount++){
				lLog = &mLog.sEntry[(lLogCount * LOG_ENTRIES_WRITE)];
				sprintf(lName, "log%d", lLogCount);
				vTaskDelay(1);
				lResult = nvs_set_blob(lHandle, lName, lLog, lLength);
				if (lResult != ESP_OK){
					break;
				}
			}
		}
		if (lResult == ESP_OK){
			vTaskDelay(1);
			lResult = nvs_commit(lHandle);
		} else {
			mLogLevel = 1;
		}
		nvs_close(lHandle);
	}
}

void sReadLog(){
	enum LogItem lAction;
	nvs_handle lHandle;
	esp_err_t lResult;
	size_t lLength;
	struct log_entry * lLog;
	uint8 lLogCount;
	char lName[5];

	lResult = nvs_open("switch", NVS_READONLY, &lHandle);
	if (lResult == ESP_OK){
		lLength = sizeof(struct log_header);
		lResult = nvs_get_blob(lHandle, "logheader", &mLog.sHeader, &lLength);
		if (lResult == ESP_OK){
			for (lLogCount = 0; lLogCount < LOG_NUMBER_ENTRIES/LOG_ENTRIES_WRITE; lLogCount++){
				lLength = sizeof(struct log_entry) * LOG_ENTRIES_WRITE;
				lLog = &mLog.sEntry[(lLogCount * LOG_ENTRIES_WRITE)];
				sprintf(lName, "log%d", lLogCount);
				lResult = nvs_get_blob(lHandle, lName, lLog, &lLength);
				if (lResult != ESP_OK){
					break;
				}
			}
			if (lResult != ESP_OK){
				sLogInit();
			}
		} else {
			sLogInit();
		}
		nvs_close(lHandle);
	} else {
		sLogInit();
	}
	lAction = LogCont;
	xLogEntry(lAction, 0);
}

void xLogInit(){
	switch (xSettingLogLevel()){
	case 1:
		mLogLevel = 1;
		sLogInit();
		break;
	case 2:
		mLogLevel = 2;
		sReadLog();
		break;
	default:
		mLogLevel = 1;
		sLogInit();
		mLogLevel = 0;
		break;
	}
}
