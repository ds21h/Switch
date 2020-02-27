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

char mSsId[32];
char mPassword[64];
union{
	struct{
		uint8 sPrefix[2];
		uint8 sMac[6];
	} uMac;
	uint64 uMacStor;
} mMac;
char mName[32];
char mDescr[64];
uint8 mSwitchModel;
uint8 mLogLevel;
bool mButton;
uint32 mAutoOff;
uint32 mServerIP;
uint32 mServerPort;

const char* xSettingSsId(){
	return mSsId;
}

void xSettingSetSsId(char * pSsId){
	if (strlen(pSsId) < sizeof(mSsId)){
		strcpy(mSsId, pSsId);
	}
}

const char* xSettingPassword(){
	return mPassword;
}

void xSettingSetPassword(char * pPassword){
	if (strlen(pPassword) < sizeof(mPassword)){
		strcpy(mPassword, pPassword);
	}
}

void xSettingMacDisp(char *pMac){
	sprintf(pMac, "%02x:%02x:%02x:%02x:%02x:%02x", mMac.uMac.sMac[0], mMac.uMac.sMac[1], mMac.uMac.sMac[2], mMac.uMac.sMac[3], mMac.uMac.sMac[4], mMac.uMac.sMac[5]);
}

void xSettingSetMac(uint8 * pMac){
	memcpy(mMac.uMac.sMac, pMac, sizeof(mMac.uMac.sMac));
}

const char* xSettingName(){
	return mName;
}

void xSettingSetName(char * pName){
	if (strlen(pName) < sizeof(mName)){
		strcpy(mName, pName);
	}
}

const char* xSettingDescription(){
	return mDescr;
}

void xSettingSetDescription(char * pDescr){
	if (strlen(pDescr) < sizeof(mDescr)){
		strcpy(mDescr, pDescr);
	}
}

uint8 xSettingSwitchModel(){
	return mSwitchModel;
}

void xSettingSetSwitchModel(uint8 pModel){
	if (pModel == 1 || pModel == 2){
		mSwitchModel = pModel;
	}
}

uint8 xSettingLogLevel(){
	return mLogLevel;
}

void xSettingSetLogLevel(uint8 pLevel){
	if (pLevel <= 2){
		mLogLevel = pLevel;
	}
}

bool xSettingButton(){
	return mButton;
}

void xSettingSetButton(bool pButton){
	mButton = pButton;
}

uint32 xSettingAutoOff(){
	return mAutoOff;
}

void xSettingSetAutoOff(uint32 pAutoOff){
	mAutoOff = pAutoOff;
}

void xSettingServerIpDisp(char *pIp){
	sprintf(pIp, "%d.%d.%d.%d", (mServerIP >> 24) & 0xFF, (mServerIP >> 16) & 0xFF, (mServerIP >>  8) & 0xFF, (mServerIP) & 0xFF);
}

void xSettingSetServerIp(uint8 * pIP){
	memcpy(mServerIP, pIP, sizeof(mServerIP));
}

uint32 xSettingServerPort(){
	return mServerPort;
}

void xSettingSetServerPort(uint32 pPort){
	mServerPort = pPort;
}

void xSettingWrite(){

}

void xSettingInit(){
	nvs_handle lHandle;
	esp_err_t lResult;

	lResult = nvs_open("switch", NVS_READONLY, &lHandle);
	if (lResult == ESP_OK){
		lResult = nvs_get_str(lHandle, "ssid", mSsId, (size_t *)sizeof(mSsId));
		if (lResult != ESP_OK){
			memset(mSsId, 0, sizeof(mSsId));
		}
		lResult = nvs_get_str(lHandle, "passwrd", mPassword, (size_t *)sizeof(mPassword));
		if (lResult != ESP_OK){
			memset(mPassword, 0, sizeof(mPassword));
		}
		lResult = nvs_get_u64(lHandle, "mac", &mMac.uMacStor);
		if (lResult != ESP_OK){
			mMac.uMacStor = 0;
		}
		lResult = nvs_get_str(lHandle, "name", mName, (size_t *)sizeof(mName));
		if (lResult != ESP_OK){
			memset(mName, 0, sizeof(mName));
		}
		lResult = nvs_get_str(lHandle, "descr", mDescr, (size_t *)sizeof(mDescr));
		if (lResult != ESP_OK){
			memset(mDescr, 0, sizeof(mDescr));
		}
		lResult = nvs_get_u8(lHandle, "model", &mSwitchModel);
		if (lResult != ESP_OK){
			mSwitchModel = 1;
		}
		lResult = nvs_get_u8(lHandle, "loglevel", &mLogLevel);
		if (lResult != ESP_OK){
			mLogLevel = 1;
		}
		lResult = nvs_get_u8(lHandle, "button", (uint8 *)&mButton);
		if (lResult != ESP_OK){
			mButton = true;
		}
		lResult = nvs_get_u32(lHandle, "autooff", &mAutoOff);
		if (lResult != ESP_OK){
			mAutoOff = 43200;
		}
		lResult = nvs_get_u32(lHandle, "serverip", &mServerIP);
		lResult = nvs_get_u32(lHandle, "serverport", &mServerPort);
		nvs_close(lHandle);
	}

	void xSettingReset(){
		memset(mSsId, 0, sizeof(mSsId));
		memset(mPassword, 0, sizeof(mPassword));
		memset(&mMac, 0, sizeof(mMac));
		memset(mName, 0, sizeof(mName));
		memset(mDescr, 0, sizeof(mDescr));
		mSwitchModel = 1;
		mLogLevel = 1;
		mButton = true;
		mAutoOff = 43200;
		mServerIP = 0;
		mServerPort = 0;
	}
}
