/*
 * server.c
 *
 *  Created on: 22 feb. 2020
 *      Author: Jan
 */
#include "switch_config.h"
#include "message.h"
#include "esp_system.h"
#include <esp_http_server.h>

httpd_handle_t mServer = NULL;

esp_err_t hGetSwitch(httpd_req_t *pReq) {
	const int cBufferLength = 256;
	size_t lLength;
    char*  lBuffer;

    lBuffer = (char *)malloc(cBufferLength);
	memset(lBuffer, 0, cBufferLength);

	lLength = httpd_req_get_url_query_len(pReq) + 1;
	if (lLength > 1) {
		httpd_resp_set_status(pReq, RESP400);
		xMessCreateError(lBuffer, cBufferLength, RESP400);
	} else {
		xMessSwitchStatus(lBuffer, cBufferLength);
	}
	httpd_resp_set_type(pReq, TYPE_JSON);
	httpd_resp_send(pReq, lBuffer, strlen(lBuffer));
	free(lBuffer);
	return ESP_OK;
}

httpd_uri_t hGetSwitchCtrl = { .uri = "/switch", .method = HTTP_GET, .handler =
		hGetSwitch, .user_ctx = NULL };

esp_err_t hGetSetting(httpd_req_t *pReq) {
	const int cBufferLength = 512;
	size_t lLength;
    char*  lBuffer;

    lBuffer = (char *)malloc(cBufferLength);
	memset(lBuffer, 0, cBufferLength);

	lLength = httpd_req_get_url_query_len(pReq) + 1;
	if (lLength > 1) {
		httpd_resp_set_status(pReq, RESP400);
		xMessCreateError(lBuffer, cBufferLength, RESP400);
	} else {
		xMessSwitchSetting(lBuffer, cBufferLength);
	}
	httpd_resp_set_type(pReq, TYPE_JSON);
	httpd_resp_send(pReq, lBuffer, strlen(lBuffer));
	free(lBuffer);
	return ESP_OK;
}

httpd_uri_t hGetSettingCtrl = { .uri = "/switch/setting", .method = HTTP_GET, .handler =
		hGetSetting, .user_ctx = NULL };

esp_err_t hGetRestart(httpd_req_t *pReq) {
	const int cBufferLength = 128;
	size_t lLength;
    char*  lBuffer;

    lBuffer = (char *)malloc(cBufferLength);
	memset(lBuffer, 0, cBufferLength);

	lLength = httpd_req_get_url_query_len(pReq) + 1;
	if (lLength > 1) {
		httpd_resp_set_status(pReq, RESP400);
		xMessCreateError(lBuffer, cBufferLength, RESP400);
	} else {
		xMessRestart(lBuffer, cBufferLength);
	}
	httpd_resp_set_type(pReq, TYPE_JSON);
	httpd_resp_send(pReq, lBuffer, strlen(lBuffer));
	free(lBuffer);
	return ESP_OK;
}

httpd_uri_t hGetRestartCtrl = { .uri = "/switch/restart", .method = HTTP_GET, .handler =
		hGetRestart, .user_ctx = NULL };

esp_err_t hPutSwitch(httpd_req_t *pReq) {
	const int cBufferLength = 256;
	size_t lLength;
    char*  lBuffer;
    int lBytesRead;
    esp_err_t lResult;
    uint16 lSwitchResult;

    lBuffer = (char *)malloc(cBufferLength);
	memset(lBuffer, 0, cBufferLength);

	lLength = httpd_req_get_url_query_len(pReq) + 1;
	if (lLength > 1) {
		httpd_resp_set_status(pReq, RESP400);
		xMessCreateError(lBuffer, cBufferLength, RESP400);
		lResult = ESP_FAIL;
	} else {
		if (pReq->content_len < cBufferLength){
			lBytesRead = httpd_req_recv(pReq, lBuffer, cBufferLength);
			if (lBytesRead <= 0){
		        if (lBytesRead == HTTPD_SOCK_ERR_TIMEOUT) {
		    		httpd_resp_set_status(pReq, RESP408);
		    		xMessCreateError(lBuffer, cBufferLength, RESP408);
		    		lResult = ESP_FAIL;
		        } else {
		    		httpd_resp_set_status(pReq, RESP500);
		    		xMessCreateError(lBuffer, cBufferLength, RESP500);
		    		lResult = ESP_FAIL;
		        }
			} else {
				lSwitchResult = xMessSetSwitch(lBuffer, cBufferLength);
				switch (lSwitchResult){
				case 200:
		    		httpd_resp_set_status(pReq, RESP200);
		    		lResult = ESP_OK;
					break;
				case 400:
		    		httpd_resp_set_status(pReq, RESP400);
		    		lResult = ESP_FAIL;
					break;
				case 408:
		    		httpd_resp_set_status(pReq, RESP408);
		    		lResult = ESP_FAIL;
					break;
				case 413:
		    		httpd_resp_set_status(pReq, RESP413);
		    		lResult = ESP_FAIL;
					break;
				case 500:
		    		httpd_resp_set_status(pReq, RESP500);
		    		lResult = ESP_FAIL;
					break;
				default:
		    		httpd_resp_set_status(pReq, RESP499);
		    		lResult = ESP_FAIL;
					break;
				}
			}
		} else {
			httpd_resp_set_status(pReq, RESP413);
    		xMessCreateError(lBuffer, cBufferLength, RESP413);
			lResult = ESP_FAIL;
		}
	}
	httpd_resp_set_type(pReq, TYPE_JSON);
	httpd_resp_send(pReq, lBuffer, strlen(lBuffer));
	free(lBuffer);
	return lResult;
}

httpd_uri_t hPutSwitchCtrl = { .uri = "/switch", .method = HTTP_PUT, .handler =
		hPutSwitch, .user_ctx = NULL };

esp_err_t hPutSetting(httpd_req_t *pReq) {
	const int cBufferLength = 512;
	size_t lLength;
    char*  lBuffer;
    int lBytesRead;
    esp_err_t lResult;
    uint16 lSwitchResult;

    lBuffer = (char *)malloc(cBufferLength);
	memset(lBuffer, 0, cBufferLength);

	lLength = httpd_req_get_url_query_len(pReq) + 1;
	if (lLength > 1) {
		httpd_resp_set_status(pReq, RESP400);
		xMessCreateError(lBuffer, cBufferLength, RESP400);
		lResult = ESP_FAIL;
	} else {
		if (pReq->content_len < cBufferLength){
			lBytesRead = httpd_req_recv(pReq, lBuffer, cBufferLength);
			if (lBytesRead <= 0){
		        if (lBytesRead == HTTPD_SOCK_ERR_TIMEOUT) {
		    		httpd_resp_set_status(pReq, RESP408);
		    		xMessCreateError(lBuffer, cBufferLength, RESP408);
		    		lResult = ESP_FAIL;
		        } else {
		    		httpd_resp_set_status(pReq, RESP500);
		    		xMessCreateError(lBuffer, cBufferLength, RESP500);
		    		lResult = ESP_FAIL;
		        }
			} else {
				lSwitchResult = xMessSetSetting(lBuffer, cBufferLength);
				switch (lSwitchResult){
				case 200:
		    		httpd_resp_set_status(pReq, RESP200);
		    		lResult = ESP_OK;
					break;
				case 400:
		    		httpd_resp_set_status(pReq, RESP400);
		    		lResult = ESP_FAIL;
					break;
				case 408:
		    		httpd_resp_set_status(pReq, RESP408);
		    		lResult = ESP_FAIL;
					break;
				case 413:
		    		httpd_resp_set_status(pReq, RESP413);
		    		lResult = ESP_FAIL;
					break;
				case 500:
		    		httpd_resp_set_status(pReq, RESP500);
		    		lResult = ESP_FAIL;
					break;
				default:
		    		httpd_resp_set_status(pReq, RESP499);
		    		lResult = ESP_FAIL;
					break;
				}
			}
		} else {
			httpd_resp_set_status(pReq, RESP413);
    		xMessCreateError(lBuffer, cBufferLength, RESP413);
			lResult = ESP_FAIL;
		}
	}
	httpd_resp_set_type(pReq, TYPE_JSON);
	httpd_resp_send(pReq, lBuffer, strlen(lBuffer));
	free(lBuffer);
	return lResult;
}

httpd_uri_t hPutSettingCtrl = { .uri = "/switch/setting", .method = HTTP_PUT, .handler =
		hPutSetting, .user_ctx = NULL };

void xStartServer() {
	httpd_config_t lConfig;

	if (mServer == NULL){
		lConfig = (httpd_config_t)HTTPD_DEFAULT_CONFIG();
		printf("Starting server on port: '%d'\n", lConfig.server_port);
		if (httpd_start(&mServer, &lConfig) == ESP_OK) {
			// Set URI handlers
			httpd_register_uri_handler(mServer, &hGetSwitchCtrl);
			httpd_register_uri_handler(mServer, &hGetSettingCtrl);
			httpd_register_uri_handler(mServer, &hGetRestartCtrl);
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

