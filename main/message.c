/*
 * message.c
 *
 *  Created on: 21 feb. 2020
 *      Author: Jan
 */
#include "switch_config.h"
#include "message.h"
#include <string.h>
#include "cJSON.h"
#include "setting.h"
#include "esp_system.h"
#include <esp_http_server.h>
#include <ctype.h>
#include "switch.h"

void xMessSwitchStatus(char * pBuffer, const int pLength){
	cJSON *lReply;
	cJSON *lResult;
	cJSON *lVersion;
	cJSON *lIdfVersion;
	cJSON *lDate;
	cJSON *lName;
	cJSON *lDescr;
	cJSON *lModel;
	cJSON *lStatus;

	memset(pBuffer, 0, pLength);

	lReply = cJSON_CreateObject();
	lResult = cJSON_CreateString("OK");
	cJSON_AddItemToObject(lReply, "result", lResult);
	lVersion = cJSON_CreateString(VERSION);
	cJSON_AddItemToObject(lReply, "version", lVersion);
	lIdfVersion = cJSON_CreateString(esp_get_idf_version());
	cJSON_AddItemToObject(lReply, "idf-version", lIdfVersion);
	lDate = cJSON_CreateString(__DATE__);
	cJSON_AddItemToObject(lReply, "date", lDate);
	lName = cJSON_CreateString(xSettingName());
	cJSON_AddItemToObject(lReply, "name", lName);
	lDescr = cJSON_CreateString(xSettingDescription());
	cJSON_AddItemToObject(lReply, "descr", lDescr);
	lModel = cJSON_CreateNumber(xSettingSwitchModel());
	cJSON_AddItemToObject(lReply, "model", lModel);
	lStatus = cJSON_CreateString((xSwitchStatus()) ? "on" : "off");
	cJSON_AddItemToObject(lReply, "status", lStatus);
	cJSON_PrintPreallocated(lReply, pBuffer, pLength, false);
	cJSON_Delete(lReply);
}

void xMessSwitchSetting(char * pBuffer, const int pLength){
	cJSON *lReply;
	cJSON *lResult;
	cJSON *lSsId;
	cJSON *lPassword;
	cJSON *lMac;
	cJSON *lName;
	cJSON *lDescr;
	cJSON *lModel;
	cJSON *lLogLevel;
	cJSON *lButton;
	cJSON *lAutoOff;
	cJSON *lServerIP;
	cJSON *lServerPort;

	char lWorkStr[17];

	memset(pBuffer, 0, pLength);

	lReply = cJSON_CreateObject();
	lResult = cJSON_CreateString("OK");
	cJSON_AddItemToObject(lReply, "result", lResult);
	lSsId = cJSON_CreateString(xSettingSsId());
	cJSON_AddItemToObject(lReply, "ssid", lSsId);
	lPassword = cJSON_CreateString(xSettingPassword());
	cJSON_AddItemToObject(lReply, "password", lPassword);
	memset(lWorkStr, 0, sizeof(lWorkStr));
	xSettingMacDisp(lWorkStr);
	lMac = cJSON_CreateString(lWorkStr);
	cJSON_AddItemToObject(lReply, "mac", lMac);
	lName = cJSON_CreateString(xSettingName());
	cJSON_AddItemToObject(lReply, "name", lName);
	lDescr = cJSON_CreateString(xSettingDescription());
	cJSON_AddItemToObject(lReply, "descr", lDescr);
	lModel = cJSON_CreateNumber(xSettingSwitchModel());
	cJSON_AddItemToObject(lReply, "model", lModel);
	lLogLevel = cJSON_CreateNumber(xSettingLogLevel());
	cJSON_AddItemToObject(lReply, "loglevel", lLogLevel);
	lButton = cJSON_CreateString((xSettingButton()) ? "on" : "off");
	cJSON_AddItemToObject(lReply, "button", lButton);
	lAutoOff = cJSON_CreateNumber(xSettingAutoOff());
	cJSON_AddItemToObject(lReply, "auto-off", lAutoOff);
	memset(lWorkStr, 0, sizeof(lWorkStr));
	xSettingServerIpDisp(lWorkStr);
	lServerIP = cJSON_CreateString(lWorkStr);
	cJSON_AddItemToObject(lReply, "serverip", lServerIP);
	lServerPort = cJSON_CreateNumber(xSettingServerPort());
	cJSON_AddItemToObject(lReply, "serverport", lServerPort);
	cJSON_PrintPreallocated(lReply, pBuffer, pLength, false);
	cJSON_Delete(lReply);
}

uint16 xMessSetSwitch(char * pBuffer, const int pLength){
	cJSON *lRequest = NULL;
	cJSON *lStatus = NULL;
	uint16 lResult;

	lRequest = cJSON_Parse(pBuffer);
	if (lRequest == NULL){
		xMessCreateError(pBuffer, pLength, RESP400);
		lResult = 400;
	} else {
	    lStatus = cJSON_GetObjectItem(lRequest, "status");
	    if (cJSON_IsString(lStatus) && (lStatus->valuestring != NULL)){
	    	if (strcmp(lStatus->valuestring, "on") == 0){
	    		xSwitchOn();
	    		xMessSwitchStatus(pBuffer, pLength);
	    		lResult = 200;
	    	} else {
		    	if (strcmp(lStatus->valuestring, "off") == 0){
		    		xSwitchOff();
		    		xMessSwitchStatus(pBuffer, pLength);
		    		lResult = 200;
		    	} else {
		    		xMessCreateError(pBuffer, pLength, RESP400);
		    		lResult = 400;
		    	}
	    	}
	    } else {
    		xMessCreateError(pBuffer, pLength, RESP400);
    		lResult = 400;
	    }
	}
	cJSON_Delete(lRequest);
	return lResult;
}

static uint8 sHexToInt(char pHex) {
	uint8 lInt;

	if (pHex >= '0' && pHex <= '9') {
		lInt = pHex - '0';
	} else {
		if (pHex >= 'a' && pHex <= 'f') {
			lInt = pHex - 'a' + 10;
		} else {
			if (pHex >= 'A' && pHex <= 'F') {
				lInt = pHex - 'A' + 10;
			} else {
				lInt = 16;
			}
		}
	}
	return lInt;
}

bool sParseIp(const char *pIpString, uint8 pIp[4]) {
	const char *lPos;
	int lSeqNmbr;
	bool lInit;
	bool lResult;

	lPos = pIpString;
	lSeqNmbr = 0;
	lInit = true;
	lResult = true;
	while (*lPos != '\0') {
		if (lInit == true) {
			pIp[lSeqNmbr] = 0;
			lInit = false;
		}
		if (isdigit(*lPos)) {
			pIp[lSeqNmbr] *= 10;
			pIp[lSeqNmbr] += *lPos - '0';
		} else {
			if (*lPos == '.') {
				lSeqNmbr++;
				if (lSeqNmbr < 4) {
					lInit = true;
				} else {
					lResult = false;
					break;
				}
			} else {
				lResult = false;
				break;
			}
		}
		lPos++;
	}
	return lResult;
}

uint16 xMessSetSetting(char * pBuffer, const int pLength){
	cJSON *lRequest = NULL;
	cJSON *lItem = NULL;
	uint16 lResult;
	uint8 lMac[6];
	char *lMacBuffer;
	bool lMacOk;
	int lCount;
	uint8 lInt;
	uint8 lByte;
	uint8 lIp[4];

	lRequest = cJSON_Parse(pBuffer);
	if (lRequest == NULL){
		xMessCreateError(pBuffer, pLength, RESP400);
		lResult = 400;
	} else {
	    lItem = cJSON_GetObjectItem(lRequest, "reset");
	    if (cJSON_IsString(lItem) && (lItem->valuestring != NULL)){
	    	if (strcmp(lItem->valuestring, "true") == 0){
	    		xSettingReset();
	    		xSettingWrite();
	    	}
    		xMessSwitchSetting(pBuffer, pLength);
    		lResult = 200;
	    } else {
		    lItem = cJSON_GetObjectItem(lRequest, "ssid");
		    if (cJSON_IsString(lItem) && (lItem->valuestring != NULL)){
		    	xSettingSetSsId(lItem->valuestring);
		    }
		    lItem = cJSON_GetObjectItem(lRequest, "password");
		    if (cJSON_IsString(lItem) && (lItem->valuestring != NULL)){
		    	xSettingSetPassword(lItem->valuestring);
		    }
		    lItem = cJSON_GetObjectItem(lRequest, "mac");
		    if (cJSON_IsString(lItem) && (lItem->valuestring != NULL)){
		    	if (strlen(lItem->valuestring) == 17){
					if (lItem->valuestring[2] == ':' && lItem->valuestring[5] == ':'
							&& lItem->valuestring[8] == ':' && lItem->valuestring[11] == ':'
							&& lItem->valuestring[14] == ':') {
						memset(lMac, 0, sizeof(lMac));
						lMacOk = true;
						lMacBuffer = lItem->valuestring;
						for (lCount = 0; lCount < 6; lCount++) {
							lInt = sHexToInt(lMacBuffer[0]);
							if (lInt > 15) {
								lMacOk = false;
								break;
							}
							lByte = lInt << 4;

							lInt = sHexToInt(lMacBuffer[1]);
							if (lInt > 15) {
								lMacOk = false;
								break;
							}
							lByte |= lInt;
							lMac[lCount] = lByte;

							lMacBuffer += 3;
						}
						if (lMacOk) {
							xSettingSetMac(lMac);
						}
					}
		    	}
		    }
		    lItem = cJSON_GetObjectItem(lRequest, "name");
		    if (cJSON_IsString(lItem) && (lItem->valuestring != NULL)){
		    	xSettingSetName(lItem->valuestring);
		    }
		    lItem = cJSON_GetObjectItem(lRequest, "descr");
		    if (cJSON_IsString(lItem) && (lItem->valuestring != NULL)){
		    	xSettingSetDescription(lItem->valuestring);
		    }
		    lItem = cJSON_GetObjectItem(lRequest, "model");
		    if (cJSON_IsNumber(lItem)){
		    	xSettingSetSwitchModel(lItem->valueint);
		    }
		    lItem = cJSON_GetObjectItem(lRequest, "loglevel");
		    if (cJSON_IsNumber(lItem)){
		    	xSettingSetSwitchModel(lItem->valueint);
		    }
		    lItem = cJSON_GetObjectItem(lRequest, "button");
		    if (cJSON_IsString(lItem) && (lItem->valuestring != NULL)){
		    	if (strcmp(lItem->valuestring, "on")){
		    		xSettingSetButton(true);
		    	} else {
			    	if (strcmp(lItem->valuestring, "off")){
			    		xSettingSetButton(false);
			    	}
		    	}
		    }
		    lItem = cJSON_GetObjectItem(lRequest, "auto-off");
		    if (cJSON_IsNumber(lItem)){
		    	if (lItem->valueint >= 0){
		    		xSettingSetAutoOff(lItem->valueint);
		    	}
		    }
		    lItem = cJSON_GetObjectItem(lRequest, "serverip");
		    if (cJSON_IsString(lItem) && (lItem->valuestring != NULL)){
				if (sParseIp(lItem->valuestring, lIp)){
					xSettingSetServerIp(lIp);
				}
		    }
		    lItem = cJSON_GetObjectItem(lRequest, "serverport");
		    if (cJSON_IsNumber(lItem)){
		    	if (lItem->valueint >= 0){
		    		xSettingSetServerPort(lItem->valueint);
		    	}
		    }

		    xSettingWrite();

    		xMessSwitchSetting(pBuffer, pLength);
    		lResult = 200;
	    }
	}
	cJSON_Delete(lRequest);
	return lResult;
}

void xMessCreateError(char * pBuffer, const int pLength, const char * pText){
	cJSON *lReply;
	cJSON *lResult;
	cJSON *lText;

	memset(pBuffer, 0, pLength);
	lReply = cJSON_CreateObject();
	lResult = cJSON_CreateString("NOK");
	cJSON_AddItemToObject(lReply, "result", lResult);
	lText = cJSON_CreateString(pText);
	cJSON_AddItemToObject(lReply, "text", lText);
	cJSON_PrintPreallocated(lReply, pBuffer, pLength, false);
	cJSON_Delete(lReply);
}
