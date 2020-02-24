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
