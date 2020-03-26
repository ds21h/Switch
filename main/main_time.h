/*
 * main_time.h
 *
 *  Created on: 25 mrt. 2020
 *      Author: Jan
 */

#ifndef MAIN_MAIN_TIME_H_
#define MAIN_MAIN_TIME_H_

void xTimeInit();
int32 xTimeNow();
void xTimeString(int32 pTime, char * pResult, uint8 pLength);

#endif /* MAIN_MAIN_TIME_H_ */
