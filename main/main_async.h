/*
 * main_async.h
 *
 *  Created on: 22 mrt. 2020
 *      Author: Jan
 */

#ifndef MAIN_MAIN_ASYNC_H_
#define MAIN_MAIN_ASYNC_H_

enum AsyncAction{
	ActionWriteSetting,
	ActionRestart,
	ActionMessage
};

struct QueueItem{
	enum AsyncAction qAction;
	void * ActionData;
};

void xAsyncProcess(struct QueueItem pItem);
void xAsyncInit();

#endif /* MAIN_MAIN_ASYNC_H_ */
