/*
 * logger.h
 *
 *  Created on: 26 mrt. 2020
 *      Author: Jan
 */

#ifndef MAIN_LOGGER_H_
#define MAIN_LOGGER_H_

#include "switch_config.h"

enum LogItem {
LogNone = 0,
LogInit,
LogSet,
LogGetSwitch,
LogGetSetting,
LogGetSwitchMult,
LogGetLog,
LogGetLogMult,
LogGetRestart,
LogGetError,
LogPutSwitchOn,
LogPutSwitchOff,
LogPutSwitchFlip,
LogSwitchAutoOff,
LogPutSwitchError,
LogPutButtonOn,
LogPutButtonOff,
LogPutButtonError,
LogPutSetting,
LogPutError,
LogUpgrade,
LogRestart
};

int xLogNumber();
int xLogCurrent();
int xLogAction(int pEntry);
long xLogTime(int pEntry);
uint32 xLogIP(int pEntry);
void xLogEntry(enum LogItem pAction, uint32 pIp);
void xLogInit();

#endif /* MAIN_LOGGER_H_ */
