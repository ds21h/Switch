/*
 * logger.c
 *
 *  Created on: 26 mrt. 2020
 *      Author: Jan
 */
#include "switch_config.h"
#include "stdlib.h"
#include "string.h"
#include "esp_system.h"
#include "main_time.h"
#include "setting.h"
#include "logger.h"

#define LOG_VERSION			100
#define LOG_NUMBER_ENTRIES	250

static char* mLogText[] = {"None", "Log Init", "GET Switch", "GET Switch mult.", "GET Switch Err.", "GET Setting", "GET Setting Err.", "GET Log", "GET Log mult.", "GET Log Err.", "Restart", "Restart Err.", "Upgrade", "Upgrade Err.", "Switch On", "Switch Off", "Switch AutoOff", "PUT Switch Err.", "PUT Setting", "PUT Setting Err."};

struct log_entry{
	long sTime;
	uint8 sAction;
	uint32 sIp;
};

struct log{
	uint8 sVersion;
	uint8 sNumber;
	uint8 sCurrent;
	struct log_entry sEntry[LOG_NUMBER_ENTRIES];
} mLog;

uint8 mLogLevel = 0;

int xLogNumber(){
	return mLog.sNumber;
}

int xLogCurrent(){
	return mLog.sCurrent;
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
		strncpy(pBuffer, mLogText[mLog.sEntry[pEntry].sAction], pLength);
		pBuffer[pLength - 1] = 0;
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
	lPrev = mLog.sCurrent - 1;
	if (lPrev < 0){
		lPrev = mLog.sNumber - 1;
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
			mLog.sEntry[mLog.sCurrent].sAction = (uint8)pAction;
			mLog.sEntry[mLog.sCurrent].sTime = xTimeNow();
			mLog.sEntry[mLog.sCurrent].sIp = pIp;
			mLog.sCurrent++;
			if (mLog.sCurrent >= mLog.sNumber){
				mLog.sCurrent = 0;
			}
		}
	}
}

void sLogInit(){
    memset(&mLog, 0, sizeof(mLog));
    mLog.sVersion = LOG_VERSION;
    mLog.sNumber = LOG_NUMBER_ENTRIES;
    mLog.sCurrent = 0;
}

void xLogInit(){
	uint8 lLogLevel;
	enum LogItem lAction;

	lLogLevel = xSettingLogLevel();
	if (lLogLevel > 1){
		mLogLevel = 0;
	} else {
		mLogLevel = lLogLevel;
	}

	if (mLogLevel > 0){
		sLogInit();
		lAction = LogInit;
    	xLogEntry(lAction, 0);
	}
}
