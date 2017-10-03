/*
 * workThread.cpp
 *
 *  Created on: 2017年10月3日
 *      Author: lisi
 */

#define _GNU_SOURCE
#include <sched.h>
#include <pthread.h>
#include <sys/sysinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "workThread.h"
#include "sort.h"

#define THREAD_NUM (15)
#define QSORT_THREAD_NUM (8)										//T0~T7
#define MERGE_1_NUM		(4)										//T8~T11
#define MERGE_2_NUM 		(2)										//T12~T13
#define MERGE_3_NUM 		(1)										//T14

static int get_CPU_core_num();
static int set_CPU(pthread_t thread_fd, uint cpu);
static int get_CPU(pthread_t thread_fd);
static void qsort_thread(void* pParam);
static void merge_thread(void* pParam);

typedef struct param_t merge_param;

struct param_t {
	int* data;
	int start;
	int end;
	int isThreadReady;
	int wait_fd_1;
	int wait_fd_2;
};

int threadsReady[THREAD_NUM];
merge_param PARAMS[THREAD_NUM];
pthread_t tid[THREAD_NUM];

int init_work_threads(int *data, int len) {
	if (len % 8 != 0 || data == NULL) {
		return -1;
	}
	memset(threadsReady, 0x0, sizeof(int) * THREAD_NUM);
	memset(PARAMS, 0x0, sizeof(merge_param) * THREAD_NUM);
	memset(tid, 0x0, sizeof(pthread_t) * THREAD_NUM);
	int i = 0;
	//T0~T7
	for (i = 0; i != QSORT_THREAD_NUM; i++) {
		PARAMS[i].data = data;
		PARAMS[i].start = (len / QSORT_THREAD_NUM) * i;
		PARAMS[i].end = (len / QSORT_THREAD_NUM) * (i + 1);
		PARAMS[i].isThreadReady = 0;
		PARAMS[i].wait_fd_1 = -1;
		PARAMS[i].wait_fd_2 = -1;
	}
	//T8～T11
	for (i = QSORT_THREAD_NUM; i != MERGE_1_NUM + QSORT_THREAD_NUM; i++) {
		PARAMS[i].data = data;
		PARAMS[i].start = (len / MERGE_1_NUM) * (i - QSORT_THREAD_NUM);
		PARAMS[i].end = (len / MERGE_1_NUM) * (i - QSORT_THREAD_NUM + 1);
		PARAMS[i].isThreadReady = 0;
		PARAMS[i].wait_fd_1 = (i - QSORT_THREAD_NUM) * 2;
		PARAMS[i].wait_fd_2 = (i - QSORT_THREAD_NUM) * 2 + 1;	//T0~T7
	}
	//T12，T13
	for (i = MERGE_1_NUM + QSORT_THREAD_NUM;
			i != MERGE_2_NUM + MERGE_1_NUM + QSORT_THREAD_NUM; i++) {
		PARAMS[i].data = data;
		PARAMS[i].start = (len / MERGE_2_NUM)
				* (i - MERGE_1_NUM - QSORT_THREAD_NUM);
		PARAMS[i].end = (len / MERGE_2_NUM)
				* (i - MERGE_1_NUM - QSORT_THREAD_NUM + 1);
		PARAMS[i].isThreadReady = 0;
		PARAMS[i].wait_fd_1 = QSORT_THREAD_NUM
				+ 2 * (i - MERGE_1_NUM - QSORT_THREAD_NUM);		//T8~T11
		PARAMS[i].wait_fd_2 = QSORT_THREAD_NUM
				+ 2 * (i - MERGE_1_NUM - QSORT_THREAD_NUM) + 1;
	}
	//T14
	i = MERGE_2_NUM + MERGE_1_NUM + QSORT_THREAD_NUM;
	PARAMS[i].data = data;
	PARAMS[i].start = (len / MERGE_3_NUM)
			* (i - MERGE_2_NUM - MERGE_1_NUM - QSORT_THREAD_NUM);
	PARAMS[i].end = (len / MERGE_3_NUM)
			* (i - MERGE_2_NUM - MERGE_1_NUM - QSORT_THREAD_NUM + 1);
	PARAMS[i].isThreadReady = 0;
	PARAMS[i].wait_fd_1 = QSORT_THREAD_NUM + MERGE_1_NUM;	//T12,T13
	PARAMS[i].wait_fd_2 = QSORT_THREAD_NUM + MERGE_1_NUM + 1;
	return 0;

}
void sort_start() {
	int i = 0;
	int cpu_num = get_CPU_core_num();
	for (i = 0; i != QSORT_THREAD_NUM; i++) {
		pthread_create(&tid[i], NULL, (void *) qsort_thread, &PARAMS[i]);
		set_CPU(tid[i], i % cpu_num);
	}
	for (i = QSORT_THREAD_NUM; i != THREAD_NUM; i++) {
		pthread_create(&tid[i], NULL, (void *) merge_thread, &PARAMS[i]);
		set_CPU(tid[i], i % cpu_num);
	}

	for (i = 0; i != THREAD_NUM; i++) {
		pthread_join(tid[i], NULL);
	}

}
void qsort_thread(void* pParam) {
	merge_param param = *(merge_param*) (pParam);
	int* data = param.data;
	int start = param.start;
	int end = param.end;
	quick_sort(data + start, end - start );
	//print_data(data,end-start);
	((merge_param*) pParam)->isThreadReady = 1;
}
void merge_thread(void* pParam) {
	merge_param param = *(merge_param*) (pParam);
	int * data = param.data;
	int wait_fd_1 = param.wait_fd_1;
	int wait_fd_2 = param.wait_fd_2;
	int *data_begin_1 = data + PARAMS[wait_fd_1].start;
	int *data_begin_2 = data + PARAMS[wait_fd_2].start;
	int len = PARAMS[wait_fd_1].end - PARAMS[wait_fd_1].start ;
	while (PARAMS[wait_fd_1].isThreadReady != 1
			|| PARAMS[wait_fd_2].isThreadReady != 1) {
		usleep(1000);
	}
	merge(data_begin_1, data_begin_2, len);
	((merge_param*) pParam)->isThreadReady = 1;
}

int get_CPU_core_num() {
	return sysconf(_SC_NPROCESSORS_CONF);
}
int set_CPU(pthread_t thread_fd, uint cpu) {
	cpu_set_t mask;
	CPU_ZERO(&mask);
	if (cpu >= get_CPU_core_num()) {
		fprintf(stderr, "set thread affinity failed\n");
		return -1;
	}
	CPU_SET(cpu, &mask);
	if (pthread_setaffinity_np(thread_fd, sizeof(mask), &mask) < 0) {
		fprintf(stderr, "set thread affinity failed\n");
		return -1;
	}
	else{
		fprintf(stderr, "set thread :[%10ld] at cpu [%3d]\n",thread_fd,cpu);
	}
	return 0;
}
int get_CPU(pthread_t thread_fd) {
	cpu_set_t get;
	CPU_ZERO(&get);
	if (pthread_getaffinity_np(thread_fd, sizeof(get), &get) < 0) {
		fprintf(stderr, "get thread affinity failed\n");
	}
	int j = 0;
	for (j = 0; j < get_CPU_core_num(); j++) {
		if (CPU_ISSET(j, &get)) {
			return j;
		}
	}
	return -1;
}

void *myfun(void *arg) {
	cpu_set_t mask;
	cpu_set_t get;
	char buf[256];
	int i;
	int j;
	int num = get_CPU_core_num();

	for (i = 0; i < num; i++) {
		CPU_ZERO(&mask);
		CPU_SET(i, &mask);
		if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0) {
			fprintf(stderr, "set thread affinity failed\n");
		}
		CPU_ZERO(&get);
		if (pthread_getaffinity_np(pthread_self(), sizeof(get), &get) < 0) {
			fprintf(stderr, "get thread affinity failed\n");
		}
		for (j = 0; j < num; j++) {
			if (CPU_ISSET(j, &get)) {
				printf("thread %d is running in processor %d\n",
						(int) pthread_self(), j);
			}
		}
		j = 0;
		while (j++ < 100000000) {
			memset(buf, 0, sizeof(buf));
		}
	}
	get_CPU(pthread_self());
	pthread_exit(NULL);
}

