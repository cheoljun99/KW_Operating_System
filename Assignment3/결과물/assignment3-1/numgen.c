#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#define MAX_PROCESS 64

int main()
{
    FILE *f_write = fopen("temp.txt","a+");
    int i=0;

    for(i=0; i<(MAX_PROCESS*2);i++)
    {
        fprintf(f_write, "%d\n",i+1);

    }

    fclose(f_write);

    return 0;
}


