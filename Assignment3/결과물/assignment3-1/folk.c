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

#define MAX_PROCESS 64

int main()
{
    struct timespec begin, end; //시간을 측정하기 위한 변수
    time_t sec;
    long nsec;
    double diff_time;
    clock_gettime(CLOCK_MONOTONIC, &begin);
    int sum_result=0;
    FILE *f_read = fopen("temp.txt","r");
    pid_t bottom_pid;
    int i, sum;
    int received_value_child,received_value_parrent;

    pid_t top_pid = fork();
    if (top_pid < 0)
    {
        printf("fork error.\n");
        return 0;
    }
    else if (top_pid == 0)
    {
        for (i = 0; i < MAX_PROCESS; i++)
        {
            if ((bottom_pid = fork()) < 0)
            {
                printf("fork error.\n");
                return 0;
            }
            else if (bottom_pid == 0)
            {
                //(in child process) using child process (parrent->child->child)
                int num1 = 0, num2 = 0;
                fscanf(f_read, "%d\n", &num1);
                fscanf(f_read, "%d\n", &num2);
                //printf("child에서 num1 %d 보냅니다.\n", num1);
                /*
                if (feof(f_read) != 0)
                {
                   printf("파일의 끝에 도달했습니다.\n");

                }
                */
                sum = num1 + num2;
                //printf("child에서 sum %d 보냅니다.\n", sum);
                //fprintf(f_write, "%d\n",sum);
                exit(sum);

            }
            else
            {

                // (in child process) using parrent process (parrent->child->parrent)
                wait(&received_value_child);
                received_value_child = received_value_child >> 8;
                sum_result += received_value_child;


            }

        }
        //printf("value of fork in child : %d\n", sum_result);
        exit(sum_result);

    }
    else
    {
        // parrent
        wait(&received_value_parrent);
        received_value_parrent = received_value_parrent >> 8;

    }
    printf("value of fork : %d\n", received_value_parrent);
    clock_gettime(CLOCK_MONOTONIC, &end);
    sec = end.tv_sec  - begin.tv_sec;
    nsec = end.tv_nsec - begin.tv_nsec;
    if (nsec < 0) nsec += 1000000000;
    diff_time=(double)sec+((double)nsec/1000000000);
    printf("%lf\n", diff_time);
    fclose(f_read);
    return 0;

}
