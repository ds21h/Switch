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
LogInit = 0,
LogSet,
LogGetSwitch,
LogGetSetting,
LogGetSwitchMult,
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

void xLogEntry(enum LogItem pAction, uint32 pIp);
void xLogInit();

#endif /* MAIN_LOGGER_H_ */
