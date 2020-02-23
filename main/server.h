/*
 * server.h
 *
 *  Created on: 22 feb. 2020
 *      Author: Jan
 */

#ifndef MAIN_SERVER_H_
#define MAIN_SERVER_H_

httpd_handle_t xStartServer(void);
void xStopServer(httpd_handle_t pServer);

#endif /* MAIN_SERVER_H_ */
