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
#include "switch.h"

void xMessSwitchStatus(char * pBuffer){
	cJSON *lReply;
	cJSON *lResult;
	cJSON *lVersion;
	cJSON *lIdfVersion;
	cJSON *lDate;
	cJSON *lName;
	cJSON *lDescr;
	cJSON *lModel;
	cJSON *lStatus;

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
	cJSON_PrintPreallocated(lReply, pBuffer, BUFFER_LENGTH, false);
	cJSON_Delete(lReply);
}
