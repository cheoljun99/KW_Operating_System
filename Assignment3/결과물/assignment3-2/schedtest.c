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
#include <sched.h>

#define MAX_PROCESS 10000

int main()
{


    /////////////////////////////////////////////////////////
    struct sched_param param;
    //////////////////////////////////////////////////////////
    
    ////////////////////////////////////////////////////////////////////
    pid_t pid;
    int received_value_child;
    ////////////////////////////////////////////////////////////////////////

	for (int policy = 0; policy < 3; policy++)
	{

		/////////////////////////////////////////////////////////
		struct timespec begin, end; //시간을 측정하기 위한 변수 
		time_t sec;
		long int nsec;
		double diff_time;
		////////////////////////////////////////////////////////

		/////////////////////////////////////////////////////////
		int inc;
		int priority=0;
		/////////////////////////////////////////////////////////

		for (int select_priority = 0; select_priority < 3; select_priority++)
		{

			///////////////////////////////////////////////////////////////////////////
			//결과에 영향을 주는 버퍼, 캐시 제거 및 sync 동기화
			int ret1 = system("rm -rf tmp*");
			int ret2 = system("sync");
			int ret3 = system("echo 3 | sudo tee /proc/sys/vm/drop_caches");
			//////////////////////////////////////////////////////////////////////////

			switch (policy)
			{
			case 0:
				printf("The standard round robin with time sharing policy\n");//The standard round robin with time sharing policy
				switch (select_priority)
				{
				case 0://Highest
					//priority자동으로 0으로 고정
					inc=-20;
					break;
				case 1://Default
					//priority자동으로 0으로 고정
					inc = 0;
					break;
				case 2://Lowest
					//priority자동으로 0으로 고정
					inc = 19;
					break;
				default:
					break;
				}
				break;
			case 1:
				printf("The first in, first out policy\n");//The first in, first out policy
				switch (select_priority)
				{
				case 0://Highest
					priority = 99;
					inc = -20;
					break;
				case 1://Default
					priority = 50;
					inc = 0;
					break;
				case 2://Lowest
					priority = 1;
					inc = 19;
					break;
				default:
					break;
				}
				break;
			case 2:
				printf("The round robin policy\n");//The round robin policy
				switch (select_priority)
				{
				case 0://Highest
					priority = 99;
					inc = -20;
					break;
				case 1://Default
					priority = 50;
					inc = 0;
					break;
				case 2://Lowest
					priority = 1;
					inc = 19;
					break;
				default:
					break;
				}
			}

			/////////////////////////////////////////////////////////////////////////////
			//아래는 크리티컬 연산
			param.sched_priority = priority;
			nice(inc);
			sched_setscheduler(0, policy, &param);//0=standard RR,1=FIFO,2=RR
			clock_gettime(CLOCK_MONOTONIC, &begin);
			for (int i = 0; i < MAX_PROCESS; i++)
			{
				if ((pid = fork()) < 0)
				{
					printf("fork error 났어요 \n");
					return 0;
				}
				else if (pid == 0)
				{
					//printf("%d\n", param.sched_priority);
					//printf("%d\n", inc);
					param.sched_priority = priority;
					nice(inc);
					sched_setscheduler(0, policy, &param);//0=standard RR,1=FIFO,2=RR
					char buf[10];
					sprintf(buf, "%d", i);
					char dire[100] = "temp/";
					strcat(dire, buf);
					//printf("%s\n", dire);
					FILE* f_read = fopen(dire, "r");
					int num = 0;
					fscanf(f_read, "%d\n", &num);
					//printf("child에서 num %d 을 받았슴당.\n", num);
					/*
					if (feof(f_read) != 0)
					{

					  printf("파일의 끝에 도달했습니다.\n");

					}
					*/
					fclose(f_read);
					exit(num);

				}
				else
				{
					wait(&received_value_child);
				}

			}
			clock_gettime(CLOCK_MONOTONIC, &end);
			sec = end.tv_sec - begin.tv_sec;
			nsec = end.tv_nsec - begin.tv_nsec;
			if (nsec < 0) nsec += 1000000000;
			diff_time = (double)sec + ((double)nsec / 1000000000);
			////////////////////////////////////////////////////////////////////////////////////////////////////////

			switch (select_priority)//결과출력
			{
			case 0://Highest
				printf("Highest: %lf\n", diff_time);// output the time taken
				break;
			case 1://Default
				printf("default: %lf\n", diff_time);// output the time taken
				break;
			case 2://Lowest
				printf("Lowest: %lf\n", diff_time);// output the time taken
				break;
			}
		}
	}

    //sched_yield(); /*다른 실시간 쓰레드, 프로세스가 수행될 수 있도록 CPU를 양도한다. 이것이 필요한 이유는 이 프로세스가 실시간 선점형 프로세스이기때문에 다른 일반 프로세스, 쓰레드들을 항상 선점하기 때문이다.(기아현상 방지)*/
    return 0;

}
