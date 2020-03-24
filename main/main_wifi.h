/*
 * main_wifi.h
 *
 *  Created on: 24 mrt. 2020
 *      Author: Jan
 */

#ifndef MAIN_MAIN_WIFI_H_
#define MAIN_MAIN_WIFI_H_

#include "switch_config.h"

bool xWifiConnected();
void xWifiInit();
void xWifiStart();
int8 xWifiFail();

#endif /* MAIN_MAIN_WIFI_H_ */
