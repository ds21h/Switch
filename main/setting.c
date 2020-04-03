/*
 * setting.c
 *
 *  Created on: 21 feb. 2020
 *      Author: Jan
 */
#include "switch_config.h"
#include "nvs.h"
#include "esp_system.h"
#include <string.h>
#include "setting.h"
#include "FreeRTOS/FreeRTOS.h"
#include "FreeRTOS/timers.h"

#define SETTINGVERSION		1

struct {
	uint16 sVersion;
	char sSsId[32];
	char sPassword[64];
	uint8 sMac[6];
	char sName[32];
	char sDescr[64];
	uint8 sSwitchModel;
	uint8 sLogLevel;
	bool sButton;
	uint32 sAutoOff;
	uint32 sServerIP;
	uint32 sServerPort;
} mSetting;
bool mSettingChanged;

const char* xSettingSsId(){
	return mSetting.sSsId;
}

void xSettingSetSsId(char * pSsId){
	if (strcmp(mSetting.sSsId, pSsId) != 0){
		if (strlen(pSsId) < sizeof(mSetting.sSsId)){
			strcpy(mSetting.sSsId, pSsId);
			mSettingChanged = true;
		}
	}
}

const char* xSettingPassword(){
	return mSetting.sPassword;
}

void xSettingSetPassword(char * pPassword){
	if (strcmp(mSetting.sPassword, pPassword) != 0){
		if (strlen(pPassword) < sizeof(mSetting.sPassword)){
			strcpy(mSetting.sPassword, pPassword);
			mSettingChanged = true;
		}
	}
}

void xSettingMacDisp(char *pMac){
	sprintf(pMac, "%02x:%02x:%02x:%02x:%02x:%02x", mSetting.sMac[0], mSetting.sMac[1], mSetting.sMac[2], mSetting.sMac[3], mSetting.sMac[4], mSetting.sMac[5]);
}

const uint8* xSettingMac(){
	return mSetting.sMac;
}

bool xSettingMacPresent(){
	int lCount;

	for (lCount = 0; lCount < 6; lCount++){
		if (mSetting.sMac[lCount] != 0){
			return true;
		}
	}
	return false;
}

void xSettingSetMac(uint8 * pMac){
	if (memcmp(mSetting.sMac, pMac, sizeof(mSetting.sMac)) != 0){
		memcpy(mSetting.sMac, pMac, sizeof(mSetting.sMac));
		mSettingChanged = true;
	}
}

const char* xSettingName(){
	return mSetting.sName;
}

void xSettingSetName(char * pName){
	if (strcmp(mSetting.sName, pName) != 0){
		if (strlen(pName) < sizeof(mSetting.sName)){
			strcpy(mSetting.sName, pName);
			mSettingChanged = true;
		}
	}
}

const char* xSettingDescription(){
	return mSetting.sDescr;
}

void xSettingSetDescription(char * pDescr){
	if (strcmp(mSetting.sDescr, pDescr) != 0){
		if (strlen(pDescr) < sizeof(mSetting.sDescr)){
			strcpy(mSetting.sDescr, pDescr);
			mSettingChanged = true;
		}
	}
}

uint8 xSettingSwitchModel(){
	return mSetting.sSwitchModel;
}

void xSettingSetSwitchModel(uint8 pModel){
	if (pModel == 1 || pModel == 2){
		if (mSetting.sSwitchModel != pModel){
			mSetting.sSwitchModel = pModel;
			mSettingChanged = true;
		}
	}
}

uint8 xSettingLogLevel(){
	return mSetting.sLogLevel;
}

void xSettingSetLogLevel(uint8 pLevel){
	if (pLevel <= 2){
		if (mSetting.sLogLevel != pLevel){
			mSetting.sLogLevel = pLevel;
			mSettingChanged = true;
		}
	}
}

bool xSettingButton(){
	return mSetting.sButton;
}

void xSettingSetButton(bool pButton){
	if (mSetting.sButton != pButton){
		mSetting.sButton = pButton;
		mSettingChanged = true;
	}
}

uint32 xSettingAutoOff(){
	return mSetting.sAutoOff;
}

void xSettingSetAutoOff(uint32 pAutoOff){
	if (mSetting.sAutoOff != pAutoOff){
		mSetting.sAutoOff = pAutoOff;
		mSettingChanged = true;
	}
}

void xSettingServerIpDisp(char *pIp){
	uint8 lIP[4];

	memcpy (lIP, &mSetting.sServerIP, sizeof(lIP));
	sprintf(pIp, "%d.%d.%d.%d", lIP[0], lIP[1], lIP[2], lIP[3]);
}

void xSettingSetServerIp(uint8 * pIP){
	if (memcmp(&mSetting.sServerIP, pIP, sizeof(mSetting.sServerIP)) != 0){
		memcpy(&mSetting.sServerIP, pIP, sizeof(mSetting.sServerIP));
		mSettingChanged = true;
	}
}

uint32 xSettingServerPort(){
	return mSetting.sServerPort;
}

void xSettingSetServerPort(uint32 pPort){
	if (mSetting.sServerPort != pPort){
		mSetting.sServerPort = pPort;
		mSettingChanged = true;
	}
}

void xSettingWrite(){
	nvs_handle lHandle;
	esp_err_t lResult;

	if (mSettingChanged){
		/* As the nvs_set_blob can take some time insert some delays to prevent watchdog reset!  */
		vTaskDelay(1);
		lResult = nvs_open("switch", NVS_READWRITE, &lHandle);
		if (lResult == ESP_OK){
			vTaskDelay(1);
			lResult = nvs_set_blob(lHandle, "setting", &mSetting, sizeof(mSetting));
			printf("Write Setting. Result %d\n", lResult);
			vTaskDelay(1);
			lResult = nvs_commit(lHandle);
			printf("Commit write. Result %d\n", lResult);
			nvs_close(lHandle);
			printf("Close. Result %d\n", lResult);
		}
		mSettingChanged = false;
	}
}

void xSettingReset(){
	memset(&mSetting, 0, sizeof(mSetting));
	mSetting.sVersion = SETTINGVERSION;
	mSetting.sSwitchModel = 1;
	mSetting.sLogLevel = 0;
	mSetting.sButton = true;
	mSetting.sAutoOff = 43200;
	mSettingChanged = true;
}

void xSettingInit(){
	nvs_handle lHandle;
	esp_err_t lResult;
	size_t lLength;

	printf("Init setting\n");
	mSettingChanged = false;
	lResult = nvs_open("switch", NVS_READONLY, &lHandle);
	if (lResult == ESP_OK){
		printf("NVS opened\n");
		lLength = sizeof(mSetting);
		lResult = nvs_get_blob(lHandle, "setting", &mSetting, &lLength);
		printf("Setting read, Error code : %d\n", lResult);
		if (lResult != ESP_OK){
			xSettingReset();
		}
		printf("NVS read, closing\n");
		nvs_close(lHandle);
		printf("NVS closed\n");
		if (mSetting.sVersion != SETTINGVERSION){
			xSettingReset();
		}
	} else {
		printf("NVS open failed. Error %d\n", lResult);
		xSettingReset();
	}
}
