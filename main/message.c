/*
 * message.c
 *
 *  Created on: 21 feb. 2020
 *      Author: Jan
 */
#include <ctype.h>
#include <string.h>
#include "switch_config.h"
#include "message.h"
#include "cJSON.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include <esp_http_server.h>
#include "setting.h"
#include "switch.h"
#include "logger.h"
#include "main_async.h"
#include "main_time.h"

#define ERROR_PARSE		"JSON error"
#define ERROR_NO_ACTION	"No action specified"
#define ERROR_VALUE		"Incorrect value specified"

void xMessSwitchStatus(struct MessSwitch * pSwitch){
	cJSON *lReply;
	cJSON *lResult;
	cJSON *lVersion;
	cJSON *lIdfVersion;
	cJSON *lDate;
	cJSON *lName;
	cJSON *lDescr;
	cJSON *lModel;
	cJSON *lStatus;

	memset(pSwitch, 0, sizeof(struct MessSwitch));

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
	cJSON_PrintPreallocated(lReply, pSwitch->sBuffer, sizeof(pSwitch->sBuffer), false);
	cJSON_Delete(lReply);
}

void xMessSwitchSetting(struct MessSetting * pSetting){
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

	memset(pSetting, 0, sizeof(struct MessSetting));

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
	cJSON_PrintPreallocated(lReply, pSetting->sBuffer, sizeof(pSetting->sBuffer), false);
	cJSON_Delete(lReply);
}

void xMessSwitchLog(int32 pStart, struct MessLog * pLog){
	cJSON *lReply;
	cJSON *lResult;
	cJSON *lNumber;
	cJSON *lCurrent;
	cJSON *lTime;
	cJSON *lLog;
	cJSON *lLogEntry;
	cJSON *lEntrySeq;
	cJSON *lEntryAction;
	cJSON *lEntryTime;
	cJSON *lEntryIP;
	int lTimeInt;
	char lTimeS[20];
	uint32 lIpInt;
	int lSeq;
	int lCount;

	memset(pLog, 0, sizeof(struct MessLog));

	lReply = cJSON_CreateObject();
	lResult = cJSON_CreateString("OK");
	cJSON_AddItemToObject(lReply, "result", lResult);
	lNumber = cJSON_CreateNumber(xLogNumber());
	cJSON_AddItemToObject(lReply, "number", lNumber);
	lCurrent = cJSON_CreateNumber(xLogCurrent());
	cJSON_AddItemToObject(lReply, "current", lCurrent);
	lTimeInt = xTimeNow();
	xTimeString(lTimeInt, lTimeS, sizeof(lTimeS));
	lTime = cJSON_CreateString(lTimeS);
	cJSON_AddItemToObject(lReply, "time", lTime);
	lLog = cJSON_CreateArray();
	cJSON_AddItemToObject(lReply, "log", lLog);

	if (pStart < 0){
		lSeq = xLogCurrent() - 1;
	} else {
		if (pStart < xLogNumber()){
			lSeq = pStart;
		} else {
			lSeq = xLogNumber() - 1;
		}
	}
	for (lCount = 0; lCount < 10; lCount++){
		lLogEntry = cJSON_CreateObject();
		cJSON_AddItemToArray(lLog, lLogEntry);
		lEntrySeq = cJSON_CreateNumber(lSeq);
		cJSON_AddItemToObject(lLogEntry, "entry", lEntrySeq);
		lEntryAction = cJSON_CreateNumber(xLogAction(lSeq));
		cJSON_AddItemToObject(lLogEntry, "action", lEntryAction);
		lTimeInt = xLogTime(lSeq);
		xTimeString(lTimeInt, lTimeS, sizeof(lTimeS));
		lEntryTime = cJSON_CreateString(lTimeS);
		cJSON_AddItemToObject(lLogEntry, "time", lEntryTime);
		lIpInt = xLogIP(lSeq);
		lEntryIP = cJSON_CreateString(ip4addr_ntoa((struct ip4_addr *)&lIpInt));
		cJSON_AddItemToObject(lLogEntry, "ip", lEntryIP);

		lSeq--;
		if (lSeq < 0){
			lSeq = xLogNumber() - 1;
		}
	}
	cJSON_PrintPreallocated(lReply, pLog->sBuffer, sizeof(pLog->sBuffer), false);
	cJSON_Delete(lReply);
}

void xMessRestart(struct MessRestart pRestart){
	cJSON *lReply;
	cJSON *lResult;
	cJSON *lText;
	struct QueueItem lQueueItem;

	memset(pRestart, 0, sizeof(struct MessRestart));

	lReply = cJSON_CreateObject();
	lResult = cJSON_CreateString("OK");
	cJSON_AddItemToObject(lReply, "result", lResult);
	lText = cJSON_CreateString("Restart requested");
	cJSON_AddItemToObject(lReply, "text", lText);
	cJSON_PrintPreallocated(lReply, pRestart.sBuffer, sizeof(pRestart.sBuffer), false);
	cJSON_Delete(lReply);

    lQueueItem.qAction = ActionRestart;
    xAsyncProcess(lQueueItem);
}

void xMessSetSwitch(struct MessSwitch * pSwitch){
	cJSON *lRequest = NULL;
	cJSON *lStatus = NULL;

	pSwitch->sResult.sProcessResult = true;
	lRequest = cJSON_Parse(pSwitch->sBuffer);
	if (lRequest == NULL){
		pSwitch->sResult.sProcessInfo = 9;
		xMessCreateError(pSwitch->sBuffer, sizeof(pSwitch->sBuffer), ERROR_PARSE);
	} else {
	    lStatus = cJSON_GetObjectItem(lRequest, "status");
	    if (cJSON_IsString(lStatus) && (lStatus->valuestring != NULL)){
	    	if (strcmp(lStatus->valuestring, "on") == 0){
	    		xSwitchOn();
	    		xMessSwitchStatus(pSwitch);
				pSwitch->sResult.sProcessInfo = 1;
	    	} else {
		    	if (strcmp(lStatus->valuestring, "off") == 0){
		    		xSwitchOff();
		    		xMessSwitchStatus(pSwitch);
					pSwitch->sResult.sProcessInfo = 0;
		    	} else {
		    		xMessCreateError(pSwitch->sBuffer, sizeof(pSwitch->sBuffer), ERROR_VALUE);
					pSwitch->sResult.sProcessInfo = 9;
		    	}
	    	}
	    } else {
			pSwitch->sResult.sProcessInfo = 9;
    		xMessCreateError(pSwitch->sBuffer, sizeof(pSwitch->sBuffer), ERROR_NO_ACTION);
	    }
	}
	cJSON_Delete(lRequest);
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
	const int8 *lPos;
	int lSeqNmbr;
	bool lInit;
	bool lResult;

	lPos = (int8 *)pIpString;
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
	if (lSeqNmbr != 3){
		lResult = false;
	}
	return lResult;
}

void xMessSetSetting(struct MessSetting * pSetting){
	cJSON *lRequest = NULL;
	cJSON *lItem = NULL;
	uint8 lMac[6];
	char *lMacBuffer;
	bool lMacOk;
	int lCount;
	uint8 lInt;
	uint8 lByte;
	uint8 lIp[4];
	struct QueueItem lQueueItem;

	lRequest = cJSON_Parse(pSetting->sBuffer);
	if (lRequest == NULL){
		pSetting->sResult.sProcessInfo = 9;
		xMessCreateError(pSetting->sBuffer, sizeof(pSetting->sBuffer), ERROR_PARSE);
	} else {
	    lItem = cJSON_GetObjectItem(lRequest, "reset");
	    if (cJSON_IsString(lItem) && (lItem->valuestring != NULL)){
	    	if (strcmp(lItem->valuestring, "true") == 0){
	    		xSettingReset();
	    		xSettingWrite();
	    	}
    		xMessSwitchSetting(pSetting);
    		pSetting->sResult.sProcessInfo = 0;
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
		    	xSettingSetLogLevel(lItem->valueint);
		    }
		    lItem = cJSON_GetObjectItem(lRequest, "button");
		    if (cJSON_IsString(lItem) && (lItem->valuestring != NULL)){
		    	if (strcmp(lItem->valuestring, "on") == 0){
		    		xSettingSetButton(true);
		    	} else {
			    	if (strcmp(lItem->valuestring, "off") == 0){
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

		    lQueueItem.qAction = ActionWriteSetting;
		    xAsyncProcess(lQueueItem);

    		xMessSwitchSetting(pSetting);
    		pSetting->sResult.sProcessInfo = 0;
	    }
	}
	cJSON_Delete(lRequest);
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
