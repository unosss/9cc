#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>



int expc(){
	char type[] = "r";
	char result[1024];
	char command[] = "cat result.txt";
	FILE *fp = popen("cat result.txt", "r");
        if(fp == NULL){
                perror("popen failed.");
                pclose(fp);
                return -1;
        }
        if(fgets(result,sizeof(result),fp) == NULL){
                perror("fgets failed.");
                pclose(fp);
                return -1;
        }
        pclose(fp);
	int res = atoi(result);
	printf("expected: %d\n", res);
        return res;
}
