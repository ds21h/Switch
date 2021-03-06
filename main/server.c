/*
 * server.c
 *
 *  Created on: 22 feb. 2020
 *      Author: Jan
 */
#include "switch_config.h"
#include "message.h"
#include "esp_system.h"
#include "esp_http_server.h"
#include "esp_netif.h"
#include "lwip/sockets.h"
#include "logger.h"

#define ERROR_URL 		"Query too long"
#define ERROR_PAR 		"Invalid pars"
#define ERROR_LENGTH	"Request too long"

esp_err_t hGetSwitch(httpd_req_t *pReq);
esp_err_t hGetSetting(httpd_req_t *pReq);
esp_err_t hGetLog(httpd_req_t *pReq);
esp_err_t hGetRestart(httpd_req_t *pReq);
esp_err_t hGetUpgrade(httpd_req_t *pReq);
esp_err_t hPutSwitch(httpd_req_t *pReq);
esp_err_t hPutSetting(httpd_req_t *pReq);

httpd_handle_t mServer = NULL;

httpd_uri_t hGetSwitchCtrl = {
		.uri = "/Switch",
		.method = HTTP_GET,
		.handler = hGetSwitch,
		.user_ctx = NULL };

httpd_uri_t hGetSettingCtrl = {
		.uri = "/Switch/Setting",
		.method = HTTP_GET,
		.handler = hGetSetting,
		.user_ctx = NULL };

httpd_uri_t hGetLogCtrl = {
		.uri = "/Switch/Log",
		.method = HTTP_GET,
		.handler = hGetLog,
		.user_ctx = NULL };

httpd_uri_t hGetRestartCtrl = {
		.uri = "/Switch/Restart",
		.method = HTTP_GET,
		.handler = hGetRestart,
		.user_ctx = NULL };

httpd_uri_t hGetUpgradeCtrl = {
		.uri = "/Switch/Upgrade",
		.method = HTTP_GET,
		.handler = hGetUpgrade,
		.user_ctx = NULL };

httpd_uri_t hPutSwitchCtrl = {
		.uri = "/Switch",
		.method = HTTP_PUT,
		.handler = hPutSwitch,
		.user_ctx = NULL };

httpd_uri_t hPutSettingCtrl = {
		.uri = "/Switch/Setting",
		.method = HTTP_PUT,
		.handler = hPutSetting,
		.user_ctx = NULL };

uint32 sGetRemoteIP(httpd_req_t *pReq){
	struct sockaddr_in lSocket;
	int lSockFd;
	socklen_t lSockLen;
	esp_err_t lResult;
	uint32 lIP;

    lSockFd = httpd_req_to_sockfd(pReq);
    lSockLen = sizeof(lSocket);
    lResult = lwip_getpeername(lSockFd, (struct sockaddr *)&lSocket, &lSockLen);
    if (lResult == -1) {
        lIP = 0;
    } else {
        lIP = lSocket.sin_addr.s_addr;
    }
    return lIP;
}

esp_err_t hGetSwitch(httpd_req_t *pReq) {
	size_t lLength;
    struct MessSwitch lSwitch;
    enum LogItem lLogAction;
    uint32 lIP;

	memset(&lSwitch, 0, sizeof(lSwitch));

	lLength = httpd_req_get_url_query_len(pReq) + 1;
	if (lLength > 1) {
		lLogAction = LogGetSwitchError;
		xMessCreateError(lSwitch.sBuffer, sizeof(lSwitch.sBuffer), ERROR_URL);
	} else {
		lLogAction = LogGetSwitch;
		xMessSwitchStatus(&lSwitch);
	}
	lIP = sGetRemoteIP(pReq);
	xLogEntry(lLogAction, lIP);
	httpd_resp_set_type(pReq, TYPE_JSON);
	httpd_resp_send(pReq, lSwitch.sBuffer, strlen(lSwitch.sBuffer));

	return ESP_OK;
}

esp_err_t hGetSetting(httpd_req_t *pReq) {
	size_t lLength;
    struct MessSetting lSetting;
    enum LogItem lLogAction;
    uint32 lIP;

	memset(&lSetting, 0, sizeof(lSetting));

	lLength = httpd_req_get_url_query_len(pReq) + 1;
	if (lLength > 1) {
		lLogAction = LogGetSettingError;
		xMessCreateError(lSetting.sBuffer, sizeof(lSetting.sBuffer), ERROR_URL);
	} else {
		lLogAction = LogGetSetting;
		xMessSwitchSetting(&lSetting);
	}
	lIP = sGetRemoteIP(pReq);
	xLogEntry(lLogAction, lIP);
	httpd_resp_set_type(pReq, TYPE_JSON);
	httpd_resp_send(pReq, lSetting.sBuffer, strlen(lSetting.sBuffer));
	return ESP_OK;
}

int32 sDecodeNum(char * pBuffer, int pLength){
	int32 lResult;
	int32 lFactor;
	char * lPos;

	lFactor = 1;
	lResult = 0;
	for (lPos = pBuffer + pLength - 1; lPos >= pBuffer; lPos--){
		if (*lPos >= '0' && *lPos <= '9'){
			lResult += ((*lPos - '0') * lFactor);
			lFactor *= 10;
		} else {
			if (*lPos != 0){
				lResult = -1;
				break;
			}
		}
	}
	return lResult;
}

esp_err_t hGetLog(httpd_req_t *pReq) {
	size_t lLength;
	struct MessLog lLog;
    char* lWorkBuffer;
    enum LogItem lLogAction;
    uint32 lIP;
    esp_err_t lResult;
    int32 lStart;
    int32 lMax;

    httpd_resp_set_type(pReq, TYPE_JSON);

	memset(&lLog, 0, sizeof(lLog));

	lLength = httpd_req_get_url_query_len(pReq) + 1;
	if (lLength > 18) {
		lLogAction = LogGetLogError;
		xMessCreateError(lLog.sBuffer, sizeof(lLog.sBuffer), ERROR_URL);
	} else {
		lStart = -1;
		lMax = -1;
		lResult = httpd_req_get_url_query_str(pReq, lLog.sBuffer, sizeof(lLog.sBuffer));
		if (lResult == ESP_OK){
			lWorkBuffer = lLog.sBuffer + 100;
			lResult = httpd_query_key_value(lLog.sBuffer, "start", lWorkBuffer, 4);
			if (lResult == ESP_OK){
				lStart = sDecodeNum(lWorkBuffer, 4);
			}
			memset(lWorkBuffer, 0, 4);
			lResult = httpd_query_key_value(lLog.sBuffer, "max", lWorkBuffer, 4);
			if (lResult == ESP_OK){
				lMax = sDecodeNum(lWorkBuffer, 4);
			}
		}
		lLogAction = LogGetLog;
		xMessSwitchLogInit(lStart, lMax, &lLog);
		httpd_resp_send_chunk(pReq, lLog.sBuffer, strlen(lLog.sBuffer));
		while (!lLog.sLogInfo.sLast){
			xMessSwitchLogContent(&lLog);
			if (strlen(lLog.sBuffer) > 0){
				httpd_resp_send_chunk(pReq, lLog.sBuffer, strlen(lLog.sBuffer));
			}
		}
		xMessSwitchLogEnd(&lLog);
	}
	httpd_resp_send_chunk(pReq, lLog.sBuffer, strlen(lLog.sBuffer));
	httpd_resp_send_chunk(pReq, lLog.sBuffer, 0);

	lIP = sGetRemoteIP(pReq);
	xLogEntry(lLogAction, lIP);

	return ESP_OK;
}

esp_err_t hGetRestart(httpd_req_t *pReq) {
	struct MessRestart lRestart;
	size_t lLength;
    enum LogItem lLogAction;
    uint32 lIP;

	memset(&lRestart, 0, sizeof(lRestart));

	lLength = httpd_req_get_url_query_len(pReq) + 1;
	if (lLength > 1) {
		lLogAction = LogGetRestartError;
		xMessCreateError(lRestart.sBuffer, sizeof(lRestart.sBuffer), ERROR_URL);
	} else {
		lLogAction = LogGetRestart;
		xMessRestart(&lRestart);
	}
	lIP = sGetRemoteIP(pReq);
	xLogEntry(lLogAction, lIP);
	httpd_resp_set_type(pReq, TYPE_JSON);
	httpd_resp_send(pReq, lRestart.sBuffer, strlen(lRestart.sBuffer));
	return ESP_OK;
}

esp_err_t hGetUpgrade(httpd_req_t *pReq) {
	struct MessUpgrade lUpgrade;
	size_t lLength;
    enum LogItem lLogAction;
    uint32 lIP;
    esp_err_t lResult;
    char lWork[13];
    bool lForce = false;
    bool lError;

	memset(&lUpgrade, 0, sizeof(lUpgrade));

	lLength = httpd_req_get_url_query_len(pReq) + 1;
	if (lLength > 32) {
		lLogAction = LogGetUpgradeError;
		xMessCreateError(lUpgrade.sBuffer, sizeof(lUpgrade.sBuffer), ERROR_URL);
	} else {
		lError = false;
		lResult = httpd_req_get_url_query_str(pReq, lUpgrade.sBuffer, sizeof(lUpgrade.sBuffer));
		if (lResult == ESP_OK){
			lResult = httpd_query_key_value(lUpgrade.sBuffer, "force", lWork, sizeof(lWork));
			if (lResult == ESP_OK){
				if (strcmp(lWork, "true") == 0){
					lForce = true;
				} else {
					lForce = false;
					lError = true;
				}
			} else {
				lForce = false;
			}
			if (!lError){
				memset(lWork, 0, sizeof(lWork));
				lResult = httpd_query_key_value(lUpgrade.sBuffer, "version", lWork, sizeof(lWork));
				if (lResult != ESP_OK){
					lError = true;
				}
			}
		}
		if (lError){
			lLogAction = LogGetUpgradeError;
			xMessCreateError(lUpgrade.sBuffer, sizeof(lUpgrade.sBuffer), ERROR_PAR);
		} else {
			xMessUpgrade(lWork, lForce, &lUpgrade);
			if (lUpgrade.sResult.sProcessInfo == 0){
				lLogAction = LogGetUpgrade;
			} else {
				lLogAction = LogGetUpgradeError;
			}
		}
	}
	lIP = sGetRemoteIP(pReq);
	xLogEntry(lLogAction, lIP);
	httpd_resp_set_type(pReq, TYPE_JSON);
	httpd_resp_send(pReq, lUpgrade.sBuffer, strlen(lUpgrade.sBuffer));
	return ESP_OK;
}

esp_err_t hPutSwitch(httpd_req_t *pReq) {
	size_t lLength;
    struct MessSwitch lSwitch;
    int lBytesRead;
    esp_err_t lResult;
    enum LogItem lLogAction;
    uint32 lIP;

	memset(&lSwitch, 0, sizeof(lSwitch));

	lResult = ESP_OK;
	lLength = httpd_req_get_url_query_len(pReq) + 1;
	if (lLength > 1) {
		lLogAction = LogPutSwitchError;
		xMessCreateError(lSwitch.sBuffer, sizeof(lSwitch.sBuffer), ERROR_URL);
	} else {
		if (pReq->content_len < sizeof(lSwitch.sBuffer)){
			lBytesRead = httpd_req_recv(pReq, lSwitch.sBuffer, sizeof(lSwitch.sBuffer));
			if (lBytesRead < 0){
		        if (lBytesRead == HTTPD_SOCK_ERR_TIMEOUT) {
		    		lLogAction = LogPutSwitchError;
		    		httpd_resp_set_status(pReq, RESP408);
		    		xMessCreateError(lSwitch.sBuffer, sizeof(lSwitch.sBuffer), RESP408);
		    		lResult = ESP_FAIL;
		        } else {
		    		lLogAction = LogPutSwitchError;
		    		httpd_resp_set_status(pReq, RESP500);
		    		xMessCreateError(lSwitch.sBuffer, sizeof(lSwitch.sBuffer), RESP500);
		    		lResult = ESP_FAIL;
		        }
			} else {
				xMessSetSwitch(&lSwitch);
				switch (lSwitch.sResult.sProcessInfo){
				case 0:
		    		lLogAction = LogPutSwitchOff;
					break;
				case 1:
		    		lLogAction = LogPutSwitchOn;
					break;
				default:
		    		lLogAction = LogPutSwitchError;
					break;
				}
			}
		} else {
    		xMessCreateError(lSwitch.sBuffer, sizeof(lSwitch.sBuffer), ERROR_LENGTH);
    		lLogAction = LogPutSwitchError;
		}
	}
	lIP = sGetRemoteIP(pReq);
	xLogEntry(lLogAction, lIP);
	httpd_resp_set_type(pReq, TYPE_JSON);
	httpd_resp_send(pReq, lSwitch.sBuffer, strlen(lSwitch.sBuffer));
	return lResult;
}

esp_err_t hPutSetting(httpd_req_t *pReq) {
	size_t lLength;
    struct MessSetting lSetting;
    int lBytesRead;
    esp_err_t lResult;
    enum LogItem lLogAction;
    uint32 lIP;

	memset(&lSetting, 0, sizeof(lSetting));

	lResult = ESP_OK;
	lLength = httpd_req_get_url_query_len(pReq) + 1;
	if (lLength > 1) {
		lLogAction = LogPutSettingError;
		xMessCreateError(lSetting.sBuffer, sizeof(lSetting.sBuffer), ERROR_URL);
	} else {
		if (pReq->content_len < sizeof(lSetting.sBuffer)){
			lBytesRead = httpd_req_recv(pReq, lSetting.sBuffer, sizeof(lSetting.sBuffer));
			if (lBytesRead < 0){
		        if (lBytesRead == HTTPD_SOCK_ERR_TIMEOUT) {
		    		lLogAction = LogPutSettingError;
		    		httpd_resp_set_status(pReq, RESP408);
		    		xMessCreateError(lSetting.sBuffer, sizeof(lSetting.sBuffer), RESP408);
		    		lResult = ESP_FAIL;
		        } else {
		    		lLogAction = LogPutSettingError;
		    		httpd_resp_set_status(pReq, RESP500);
		    		xMessCreateError(lSetting.sBuffer, sizeof(lSetting.sBuffer), RESP500);
		    		lResult = ESP_FAIL;
		        }
			} else {
				xMessSetSetting(&lSetting);
				if (lSetting.sResult.sProcessInfo == 0){
					lLogAction = LogPutSetting;
				} else {
					lLogAction = LogPutSettingError;
				}
			}
		} else {
			lLogAction = LogPutSettingError;
    		xMessCreateError(lSetting.sBuffer, sizeof(lSetting.sBuffer), ERROR_LENGTH);
		}
	}
	lIP = sGetRemoteIP(pReq);
	xLogEntry(lLogAction, lIP);
	httpd_resp_set_type(pReq, TYPE_JSON);
	httpd_resp_send(pReq, lSetting.sBuffer, strlen(lSetting.sBuffer));
	return lResult;
}

void xStartServer() {
	httpd_config_t lConfig;

	if (mServer == NULL){
		lConfig = (httpd_config_t)HTTPD_DEFAULT_CONFIG();
		printf("Starting server on port: '%d'\n", lConfig.server_port);
		if (httpd_start(&mServer, &lConfig) == ESP_OK) {
			// Set URI handlers
			httpd_register_uri_handler(mServer, &hGetSwitchCtrl);
			httpd_register_uri_handler(mServer, &hGetSettingCtrl);
			httpd_register_uri_handler(mServer, &hGetLogCtrl);
			httpd_register_uri_handler(mServer, &hGetRestartCtrl);
			httpd_register_uri_handler(mServer, &hGetUpgradeCtrl);
			httpd_register_uri_handler(mServer, &hPutSwitchCtrl);
			httpd_register_uri_handler(mServer, &hPutSettingCtrl);
		} else {
			printf("Error starting server!\n");
			mServer = NULL;
		}
	} else {
		printf("Server already started!\n");
	}
}

void xStopServer() {
	if (mServer != NULL){
		httpd_stop(mServer);
		mServer = NULL;
	}
}

