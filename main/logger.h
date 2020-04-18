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
LogGetSwitch,
LogGetSwitchMult,
LogGetSwitchError,
LogGetSetting,
LogGetSettingError,
LogGetLog,
LogGetLogMult,
LogGetLogError,
LogGetRestart,
LogGetRestartError,
LogGetUpgrade,
LogGetUpgradeError,
LogPutSwitchOn,
LogPutSwitchOff,
LogPutSwitchAutoOff,
LogPutSwitchError,
LogPutSetting,
LogPutSettingError
};

int xLogNumber();
int xLogCurrent();
enum LogItem xLogAction(int pEntry);
void xLogActionStr(int pEntry, char * pBuffer, int pLength);
long xLogTime(int pEntry);
uint32 xLogIP(int pEntry);
void xLogEntry(enum LogItem pAction, uint32 pIp);
void xLogInit();

#endif /* MAIN_LOGGER_H_ */
