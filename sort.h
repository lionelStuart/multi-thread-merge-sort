/*
 * sort.h
 *
 *  Created on: 2017年10月3日
 *      Author: lisi
 */

#ifndef SORT_H_
#define SORT_H_

void print_data(const int *data_begin,int len);

void quick_sort(int* data_begin,int len);

//support only data_1.length==data_2.length==len
void merge(int* data_begin_1,int* data_begin_2,int len);

#endif /* SORT_H_ */
