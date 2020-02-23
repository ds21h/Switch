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

esp_err_t hGetSwitch(httpd_req_t *pReq) {
	size_t lLength;
    char*  lBuffer;

    lBuffer = (char *)malloc(BUFFER_LENGTH);
	memset(lBuffer, 0, BUFFER_LENGTH);

	lLength = httpd_req_get_url_query_len(pReq) + 1;
	if (lLength > 1) {
		httpd_resp_set_status(pReq, HTTPD_400);
		strcpy(lBuffer, HTTPD_400);
	} else {
		xMessSwitchStatus(lBuffer);
	}
	httpd_resp_send(pReq, lBuffer, strlen(lBuffer));
	free(lBuffer);
	return ESP_OK;
}

httpd_uri_t hGetSwitchCtrl = { .uri = "/switch", .method = HTTP_GET, .handler =
		hGetSwitch, .user_ctx = NULL };

httpd_handle_t xStartServer(void) {
	httpd_handle_t lServer = NULL;
	httpd_config_t lConfig;

	lConfig = (httpd_config_t)HTTPD_DEFAULT_CONFIG();
	// Start the httpd server
	printf("Starting server on port: '%d'\n", lConfig.server_port);
	if (httpd_start(&lServer, &lConfig) == ESP_OK) {
		// Set URI handlers
		httpd_register_uri_handler(lServer, &hGetSwitchCtrl);
		return lServer;
	}

	printf("Error starting server!\n");
	return NULL;
}

void xStopServer(httpd_handle_t pServer) {
	// Stop the httpd server
	httpd_stop(pServer);
}

