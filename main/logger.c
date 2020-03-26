/*
 * logger.c
 *
 *  Created on: 26 mrt. 2020
 *      Author: Jan
 */
#include "switch_config.h"
#include "stdlib.h"
#include "string.h"
#include "main_time.h"
#include "setting.h"
#include "logger.h"

#define LOG_VERSION			100
#define LOG_NUMBER_ENTRIES	250

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
};

uint8 mLogLevel = 0;

struct log *mLog = NULL;

void xLogEntry(enum LogItem pAction, uint32 pIp){
	int8 lPrev;
	bool lMultiple;

	lMultiple = false;
	if (mLogLevel > 0){
		if (pAction == LogGetSwitch){
			lPrev = mLog->sCurrent - 1;
			if (lPrev < 0){
				lPrev = mLog->sNumber;
			}
			if (mLog->sEntry[lPrev].sAction == LogGetSwitch || mLog->sEntry[lPrev].sAction == LogGetSwitchMult){
				if (mLog->sEntry[lPrev].sIp == pIp){
					lMultiple = true;
					mLog->sEntry[mLog->sCurrent].sAction = LogGetSwitchMult;
					mLog->sEntry[mLog->sCurrent].sTime = xTimeNow();
				}
			}
		}
		if (!lMultiple){
			mLog->sEntry[mLog->sCurrent].sAction = (uint8)pAction;
			mLog->sEntry[mLog->sCurrent].sTime = xTimeNow();
			mLog->sEntry[mLog->sCurrent].sIp = pIp;
			mLog->sCurrent++;
			if (mLog->sCurrent >= mLog->sNumber){
				mLog->sCurrent = 0;
			}
		}
	}
}

void sLogInit(){
    memset(&mLog, 0, sizeof(mLog));
    mLog->sVersion = LOG_VERSION;
    mLog->sNumber = LOG_NUMBER_ENTRIES;
    mLog->sCurrent = 0;
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
		mLog = (struct log *)malloc(sizeof (struct log));
		if (mLog == NULL){
			mLogLevel = 0;
		} else {
			sLogInit();
			lAction = LogInit;
    		xLogEntry(lAction, mLogLevel);
		}
	}
}
