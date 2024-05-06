#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>

int act(){
	char command1[] = "./9cc testdir/input.txt > input.s";
	char command2[] = "cc -o input input.s";
	char command[] = "./input ; echo $?";
	char type[] = "r";
	FILE *fp1;
	FILE *fp2;
	FILE *fp;
	fp1 = popen(command1, type);
	pclose(fp1);
	fp2 = popen(command2, type);
	pclose(fp2);
	fp = popen(command, type);
	if(fp == NULL){
		perror("popen failed.");
		pclose(fp);
		return -1;
	}
	char result[1024];
	if(fgets(result,sizeof(result),fp) == NULL){
		perror("fgets failed.");
		pclose(fp);
		return -1;
	}
	pclose(fp);
	int res = atoi(result);
	printf("actual: %d\n", res);
	return res;
}	
