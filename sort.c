/*
 * sort.c
 *
 *  Created on: 2017年10月3日
 *      Author: lisi
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "main.h"
int dat_tmp[N];


static int cmp(const void* a,const void * b);

void print_data(const int *data_begin,int len)
{
	int i=0;
	for(i=0;i!=len;i++)
	{
		printf("dat[%5d]:%5d\t",i,data_begin[i]);
		if((1+i)%5==0)
		{
			printf("\n");
		}
	}
}

void quick_sort(int* data_begin,int len)
{
	 qsort(data_begin,len,sizeof(data_begin[0]),cmp);
}

void merge(int* data_begin_1,int* data_begin_2,int len)
{
	memset(dat_tmp,0x0,sizeof(int)*len*2);
	int ptr_1=0;
	int ptr_2=0;
	int ptr_3=0;
	while(ptr_1!=len&&ptr_2!=len)
	{
		if(cmp(data_begin_1+ptr_1,data_begin_2+ptr_2)>0)
		{
			*(dat_tmp+ptr_3)=*(data_begin_2+ptr_2);
			ptr_2++;
		}
		else
		{
			*(dat_tmp+ptr_3)=*(data_begin_1+ptr_1);
			ptr_1++;
		}
		ptr_3++;
	}
	if(ptr_1!=len)
	{
		while(ptr_1!=len)
		{
			*(dat_tmp+ptr_3)=*(data_begin_1+ptr_1);
			ptr_1++;
			ptr_3++;
		}
	}
	if(ptr_2!=len)
	{
		while(ptr_2!=len)
		{
			*(dat_tmp+ptr_3)=*(data_begin_2+ptr_2);
			ptr_2++;
			ptr_3++;
		}
	}
	ptr_3=0;
	while(ptr_3!=len)
	{
		*(data_begin_1+ptr_3)=*(dat_tmp+ptr_3);
		ptr_3++;
	}
	ptr_3=0;
	while(ptr_3!=len)
	{
		*(data_begin_2+ptr_3)=*(dat_tmp+len+ptr_3);
		ptr_3++;
	}
}

int cmp(const void* a,const void * b)
{
	int num_a=*((int*)a);
	int num_b=*((int*)b);
	if(num_a>num_b)
	{
		return 1;
	}
	else if(num_a==num_b)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}



