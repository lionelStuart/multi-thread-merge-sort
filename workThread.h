/*
 * workThread.h
 *
 *  Created on: 2017年10月3日
 *      Author: lisi
 */

#ifndef WORKTHREAD_H_
#define WORKTHREAD_H_

//#define _GNU_SOURCE



void * myfun(void *arg);

int init_work_threads(int *data, int len);

void sort_start();

#endif /* WORKTHREAD_H_ */
