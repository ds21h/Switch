/*
 * message.h
 *
 *  Created on: 21 feb. 2020
 *      Author: Jan
 */

#ifndef MAIN_MESSAGE_H_
#define MAIN_MESSAGE_H_

void xMessSwitchStatus(char * pBuffer, const int pLength);
uint16 xMessSetSwitch(char * pBuffer, const int pLength);
void xMessCreateError(char * pBuffer, const int pLength, const char * pText);

#endif /* MAIN_MESSAGE_H_ */
