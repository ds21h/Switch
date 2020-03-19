/*
 * setting.h
 *
 *  Created on: 21 feb. 2020
 *      Author: Jan
 */

#ifndef MAIN_SETTING_H_
#define MAIN_SETTING_H_

const char* xSettingSsId();
void xSettingSetSsId(char * pSsId);
const char* xSettingPassword();
void xSettingSetPassword(char * pPassword);
void xSettingMacDisp(char *pMac);
const uint8* xSettingMac();
bool xSettingMacPresent();
void xSettingSetMac(uint8 * pMac);
const char* xSettingName();
void xSettingSetName(char * pName);
const char* xSettingDescription();
void xSettingSetDescription(char * pDescr);
uint8 xSettingSwitchModel();
void xSettingSetSwitchModel(uint8 pModel);
uint8 xSettingLogLevel();
void xSettingSetLogLevel(uint8 pLevel);
bool xSettingButton();
void xSettingSetButton(bool pButton);
uint32 xSettingAutoOff();
void xSettingSetAutoOff(uint32 pAutoOff);
void xSettingServerIpDisp(char *pIp);
void xSettingSetServerIp(uint8 * pIP);
uint32 xSettingServerPort();
void xSettingSetServerPort(uint32 pPort);

void xSettingInit();
void xSettingReset();
void xSettingWrite();

#endif /* MAIN_SETTING_H_ */
