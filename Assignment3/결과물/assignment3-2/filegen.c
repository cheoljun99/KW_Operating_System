#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <math.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <pwd.h>
#define MAX_PROCESS 10000

int main()
{

	umask(0000);// 초기설정된 umask값을 변경
	int mkdir_err=mkdir("temp", S_IRWXU | S_IRWXG | S_IRWXO);//cache폴더 생성
	if (mkdir_err == -1)
	{
		printf("mkdir error\n");
		return 0;
	}
	int i;
	for (i = 0; i < MAX_PROCESS; i++)
	{
		char buf[10];
		sprintf(buf, "%d", i);
		char dire[100] = "temp/";
		strcat(dire, buf);
		//printf("%s\n", dire);
		FILE* f_write = fopen(dire,"w");
		fprintf(f_write, "%d", 1 + rand() % 9);
		fclose(f_write);

	}

	
}