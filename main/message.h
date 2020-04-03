/*
 * message.h
 *
 *  Created on: 21 feb. 2020
 *      Author: Jan
 */

#ifndef MAIN_MESSAGE_H_
#define MAIN_MESSAGE_H_

struct MessSwitch{
	struct {
		uint8 sProcessInfo;
	} sResult;
	char sBuffer[256];
};

struct MessSetting{
	struct {
		uint8 sProcessInfo;
	} sResult;
	char sBuffer[380];
};

struct MessRestart{
	char sBuffer[50];
};

struct MessLog{
	char sBuffer[1024];
};


void xMessSwitchStatus(struct MessSwitch * pSwitch);
void xMessSwitchSetting(struct MessSetting * pSetting);
void xMessSwitchLog(int32 pStart, struct MessLog * pLog);
void xMessRestart(struct MessRestart * pRestart);
void xMessSetSwitch(struct MessSwitch * pSwitch);
void xMessSetSetting(struct MessSetting * pSetting);
void xMessCreateError(char * pBuffer, const int pLength, const char * pText);

#endif /* MAIN_MESSAGE_H_ */
