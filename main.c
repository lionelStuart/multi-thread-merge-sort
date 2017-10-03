/*
 * main.c
 *
 *  Created on: 2017年10月3日
 *      Author: lisi
 */

#include"main.h"
#include"workThread.h"
#include"sort.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>



int dat[N];

int* getData(){
	return dat;
}

void fill_data()
{
  int i = 0;
  for (i = 0; i < N; i++) {
  dat[i] = i;
  }
}

void shuffle()
{
  int i = 0;
  int r = 0;
  int temp = 0;
  for (i = 0; i < N; i++) {
  r = rand() % (i+1);
  temp = dat[i];
  dat[i] = dat[r];
  dat[r] = temp;
  }
}
clock_t get_time(){
	clock_t start=clock();
	return start;
}
double count_time(clock_t start,clock_t end){
	return ((double)(end-start))/CLOCKS_PER_SEC;
}



int main()
{
  printf("main starts\n");
  fill_data();
  shuffle();
  /*
  print_data(dat,N);
  quick_sort(dat+0,500);
  printf("after sort_1\n");
  print_data(dat+0,500);
  quick_sort(dat+500,500);
  printf("after sort_2\n");
  print_data(dat+500,500);
  merge_sort(dat+0,dat+500,500);
  printf("\n===after sort===\n");
  print_data(dat,N);
	*/
 printf("\n===start sort===\n");
 clock_t start=get_time();
 init_work_threads(dat,N);
 sort_start();
 clock_t end=get_time();
 printf("sort cost total time:%10.5f second",count_time(start,end));
 printf("\n===sort end===\n");
 //print_data(dat,N);






  FILE *fp = fopen("data.bin", "wb");

  fwrite(dat, sizeof(int), N, fp);

  fclose(fp);
  printf("main ends\n");
  getchar();
  return 0;
}



