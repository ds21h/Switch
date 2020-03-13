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
uint8 mMac[6];
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
	sprintf(pMac, "%02x:%02x:%02x:%02x:%02x:%02x", mMac[0], mMac[1], mMac[2], mMac[3], mMac[4], mMac[5]);
}

void xSettingSetMac(uint8 * pMac){
	memcpy(mMac, pMac, sizeof(mMac));
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
	uint8 lIP[4];

	memcpy (lIP, &mServerIP, sizeof(lIP));
	sprintf(pIp, "%d.%d.%d.%d", lIP[0], lIP[1], lIP[2], lIP[3]);
}

void xSettingSetServerIp(uint8 * pIP){
	memcpy(&mServerIP, pIP, sizeof(mServerIP));
}

uint32 xSettingServerPort(){
	return mServerPort;
}

void xSettingSetServerPort(uint32 pPort){
	mServerPort = pPort;
}

bool sTestStr(esp_err_t pResult, char * pValue1, char * pValue2){
	bool lResult;

	if (pResult == ESP_OK){
		if (strcmp(pValue1, pValue2) == 0){
			lResult = false;
		} else {
			lResult = true;
		}
	} else {
		if (pResult == ESP_ERR_NVS_NOT_FOUND){
			lResult = true;
		} else {
			lResult = false;
		}
	}
	return lResult;
}

bool sTestBlob(esp_err_t pResult, void * pValue1, void * pValue2, uint32 pLength){
	bool lResult;

	if (pResult == ESP_OK){
		if (memcmp(pValue1, pValue2, pLength) == 0){
			lResult = false;
		} else {
			lResult = true;
		}
	} else {
		if (pResult == ESP_ERR_NVS_NOT_FOUND){
			lResult = true;
		} else {
			lResult = false;
		}
	}
	return lResult;
}

bool sTestU8(esp_err_t pResult, uint8 pValue1, uint8 pValue2){
	bool lResult;

	if (pResult == ESP_OK){
		if (pValue1 == pValue2){
			lResult = false;
		} else {
			lResult = true;
		}
	} else {
		if (pResult == ESP_ERR_NVS_NOT_FOUND){
			lResult = true;
		} else {
			lResult = false;
		}
	}
	return lResult;
}

bool sTestU32(esp_err_t pResult, uint32 pValue1, uint32 pValue2){
	bool lResult;

	if (pResult == ESP_OK){
		if (pValue1 == pValue2){
			lResult = false;
		} else {
			lResult = true;
		}
	} else {
		if (pResult == ESP_ERR_NVS_NOT_FOUND){
			lResult = true;
		} else {
			lResult = false;
		}
	}
	return lResult;
}

void xSettingWrite(){
	nvs_handle lHandle;
	esp_err_t lResult;
	union {
		char sString[64];
		uint8 sMac[6];
		uint8 sU8;
		uint32 sU32;
	} lInput;
	size_t lLength;
	uint8 lCount;

	lResult = nvs_open("switch", NVS_READWRITE, &lHandle);
	if (lResult == ESP_OK){
		printf("NVS opened for write\n");
		lCount = 0;

		memset(&lInput, 0, sizeof(lInput));
		lLength = sizeof(lInput);
		lResult = nvs_get_str(lHandle, "ssid", lInput.sString, &lLength);
		printf("SSID read, Error code : %d\n", lResult);
		if (sTestStr(lResult, mSsId, lInput.sString)){
			lResult = nvs_set_str(lHandle, "ssid", mSsId);
			printf("Write SSID. Result %d\n", lResult);
			lCount++;
		}

		memset(&lInput, 0, sizeof(lInput));
		lLength = sizeof(lInput);
		lResult = nvs_get_str(lHandle, "passwrd", lInput.sString, &lLength);
		printf("Password read, Error code : %d\n", lResult);
		if (sTestStr(lResult, mPassword, lInput.sString)){
			lResult = nvs_set_str(lHandle, "passwrd", mPassword);
			printf("Write Password. Result %d\n", lResult);
			lCount++;
		}

		memset(&lInput, 0, sizeof(lInput));
		lLength = sizeof(lInput);
		lResult = nvs_get_blob(lHandle, "mac", lInput.sMac, &lLength);
		printf("MAC read, Error code : %d\n", lResult);
		if (sTestBlob(lResult, mMac, lInput.sMac, sizeof(mMac))){
			lResult = nvs_set_blob(lHandle, "mac", mMac, sizeof(mMac));
			printf("Write MAC. Result %d\n", lResult);
			lCount++;
		}

		memset(&lInput, 0, sizeof(lInput));
		lLength = sizeof(lInput);
		lResult = nvs_get_str(lHandle, "name", lInput.sString, &lLength);
		printf("Name read, Error code : %d\n", lResult);
		if (sTestStr(lResult, mName, lInput.sString)){
			lResult = nvs_set_str(lHandle, "name", mName);
			printf("Write Name. Result %d\n", lResult);
			lCount++;
		}

		memset(&lInput, 0, sizeof(lInput));
		lLength = sizeof(lInput);
		lResult = nvs_get_str(lHandle, "descr", lInput.sString, &lLength);
		printf("Description read, Error code : %d\n", lResult);
		if (sTestStr(lResult, mName, lInput.sString)){
			lResult = nvs_set_str(lHandle, "descr", mDescr);
			printf("Write Description. Result %d\n", lResult);
			lCount++;
		}

		memset(&lInput, 0, sizeof(lInput));
		lLength = sizeof(lInput);
		lResult = nvs_get_u8(lHandle, "model", &lInput.sU8);
		printf("Model read, Error code : %d\n", lResult);
		if (sTestU8(lResult, mSwitchModel, lInput.sU8)){
			lResult = nvs_set_u8(lHandle, "model", mSwitchModel);
			printf("Write Model. Result %d\n", lResult);
			lCount++;
		}

		memset(&lInput, 0, sizeof(lInput));
		lLength = sizeof(lInput);
		lResult = nvs_get_u8(lHandle, "loglevel", &lInput.sU8);
		printf("LogLevel read, Error code : %d\n", lResult);
		if (sTestU8(lResult, mLogLevel, lInput.sU8)){
			lResult = nvs_set_u8(lHandle, "loglevel", mLogLevel);
			printf("Write LogLevel. Result %d\n", lResult);
			lCount++;
		}

		memset(&lInput, 0, sizeof(lInput));
		lLength = sizeof(lInput);
		lResult = nvs_get_u8(lHandle, "button", &lInput.sU8);
		printf("Button read, Error code : %d\n", lResult);
		if (sTestU8(lResult, mButton, lInput.sU8)){
			lResult = nvs_set_u8(lHandle, "button", mButton);
			printf("Write Button. Result %d\n", lResult);
			lCount++;
		}

		memset(&lInput, 0, sizeof(lInput));
		lLength = sizeof(lInput);
		lResult = nvs_get_u32(lHandle, "autooff", &lInput.sU32);
		printf("AutoOff read, Error code : %d\n", lResult);
		if (sTestU32(lResult, mAutoOff, lInput.sU32)){
			lResult = nvs_set_u32(lHandle, "autooff", mAutoOff);
			printf("Write AutoOff. Result %d\n", lResult);
			lCount++;
		}

		memset(&lInput, 0, sizeof(lInput));
		lLength = sizeof(lInput);
		lResult = nvs_get_u32(lHandle, "serverip", &lInput.sU32);
		printf("ServerIP read, Error code : %d\n", lResult);
		if (sTestU32(lResult, mServerIP, lInput.sU32)){
			lResult = nvs_set_u32(lHandle, "serverip", mServerIP);
			printf("Write ServerIP. Result %d\n", lResult);
			lCount++;
		}

		memset(&lInput, 0, sizeof(lInput));
		lLength = sizeof(lInput);
		lResult = nvs_get_u32(lHandle, "serverport", &lInput.sU32);
		printf("ServerPort read, Error code : %d\n", lResult);
		if (sTestU32(lResult, mServerPort, lInput.sU32)){
			lResult = nvs_set_u32(lHandle, "serverport", mServerPort);
			printf("Write ServerPort. Result %d\n", lResult);
			lCount++;
		}

		if (lCount > 0){
			lResult = nvs_commit(lHandle);
			printf("Commit. Result %d\n", lResult);
		}

		nvs_close(lHandle);
		printf("Close. Result %d\n", lResult);
	}
}

void xSettingReset(){
	memset(mSsId, 0, sizeof(mSsId));
	memset(mPassword, 0, sizeof(mPassword));
	memset(mMac, 0, sizeof(mMac));
	memset(mName, 0, sizeof(mName));
	memset(mDescr, 0, sizeof(mDescr));
	mSwitchModel = 1;
	mLogLevel = 1;
	mButton = true;
	mAutoOff = 43200;
	mServerIP = 0;
	mServerPort = 0;
}

void xSettingInit(){
	nvs_handle lHandle;
	esp_err_t lResult;
	size_t lLength;

	printf("Init setting\n");
	lResult = nvs_open("switch", NVS_READONLY, &lHandle);
	if (lResult == ESP_OK){
		printf("NVS opened\n");
		lLength = sizeof(mSsId);
		lResult = nvs_get_str(lHandle, "ssid", mSsId, &lLength);
		printf("SSID read, Error code : %d\n", lResult);
		if (lResult != ESP_OK){
			memset(mSsId, 0, sizeof(mSsId));
		}
		lLength = sizeof(mPassword);
		lResult = nvs_get_str(lHandle, "passwrd", mPassword, &lLength);
		printf("Password read, Error code : %d\n", lResult);
		if (lResult != ESP_OK){
			memset(mPassword, 0, sizeof(mPassword));
		}
		lLength = sizeof(mMac);
		lResult = nvs_get_blob(lHandle, "mac", mMac, &lLength);
		printf("MAC read, Error code : %d\n", lResult);
		if (lResult != ESP_OK){
			memset(mMac, 0, sizeof(mMac));
		}
		lLength = sizeof(mName);
		lResult = nvs_get_str(lHandle, "name", mName, &lLength);
		printf("Name read, Error code : %d\n", lResult);
		if (lResult != ESP_OK){
			memset(mName, 0, sizeof(mName));
		}
		lLength = sizeof(mDescr);
		lResult = nvs_get_str(lHandle, "descr", mDescr, &lLength);
		printf("Description read, Error code : %d\n", lResult);
		if (lResult != ESP_OK){
			memset(mDescr, 0, sizeof(mDescr));
		}
		lResult = nvs_get_u8(lHandle, "model", &mSwitchModel);
		printf("Model read, Error code : %d\n", lResult);
		if (lResult != ESP_OK){
			mSwitchModel = 1;
		}
		lResult = nvs_get_u8(lHandle, "loglevel", &mLogLevel);
		printf("LogLevel read, Error code : %d\n", lResult);
		if (lResult != ESP_OK){
			mLogLevel = 1;
		}
		lResult = nvs_get_u8(lHandle, "button", (uint8 *)&mButton);
		printf("LogLevel read, Error code : %d\n", lResult);
		if (lResult != ESP_OK){
			mButton = true;
		}
		lResult = nvs_get_u32(lHandle, "autooff", &mAutoOff);
		printf("AutoOff read, Error code : %d\n", lResult);
		if (lResult != ESP_OK){
			mAutoOff = 43200;
		}
		lResult = nvs_get_u32(lHandle, "serverip", &mServerIP);
		printf("ServerIP read, Error code : %d\n", lResult);
		if (lResult != ESP_OK){
			mServerIP = 0;
		}
		lResult = nvs_get_u32(lHandle, "serverport", &mServerPort);
		printf("ServerPort read, Error code : %d\n", lResult);
		if (lResult != ESP_OK){
			mServerPort = 0;
		}
		printf("NVS read, closing\n");
		nvs_close(lHandle);
		printf("NVS closed\n");
	} else {
		printf("NVS open failed. Error %d\n", lResult);
		xSettingReset();
	}
}
