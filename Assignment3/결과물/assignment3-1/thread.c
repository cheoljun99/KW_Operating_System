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
#include <pthread.h>

#define MAX_PROCESS 64

//스레드 함수에 인자값으로 넣어줄 데이터를 구조체를 정의하여 사용
typedef struct passing_thread_data_struct
{
	//쓰레드 함수에서 사용할 변수들을 구성하는 구조체이며
	//쓰레드 함수에서 쓰인다.
    FILE* f_read_in_thread;

}passing_thread_data_struct;// 예명으로 사용한다.
void* thread_function(void* passing_thread_data)
{
	//스레드함수이며
	//인자는 void형 포인터 자료형 이는 구조체 변수의 void*형으로 타입 케스트를 통해
	//인자로 넘겨줄 것이다.

    passing_thread_data_struct* passing_thread_data_in_thread_function =(passing_thread_data_struct*)passing_thread_data;
    int num1=0,num2=0;
    fscanf(passing_thread_data_in_thread_function->f_read_in_thread, "%d\n", &num1);
    fscanf(passing_thread_data_in_thread_function->f_read_in_thread, "%d\n", &num2);
    //printf("스레드에서 pos %d를 받았습니다.\n",pos);
    //printf("스레드에서 num1 %d 읽습니다.\n", num1);
    /*
    if (feof(f_read) != 0)
    {
               
        printf("파일의 끝에 도달했습니다.\n");
            
    }
    */
    int sum=num1+num2;
    //printf("스레드에서 sum %d 보냅니다.\n", sum);
    return (void*)sum;//값 반환
}

int main()
{
    struct timespec begin, end; //시간을 측정하기 위한 변수 
    clock_gettime(CLOCK_MONOTONIC, &begin);
    time_t sec;
    long int nsec;
    double diff_time;
    int sum_result=0;
    pthread_t thread_id;//스레드 아이디를 생성한다.
    int err; //스레드 생성에 실패하면 에러를 저장하기 위해 생성
    void* thread_return; //스레드 함수의 리턴값을 저장할 변수
    passing_thread_data_struct passing_thread_data;

    FILE *f_read = fopen("temp.txt","r");
    passing_thread_data.f_read_in_thread = f_read;

    int i;
    for(i=0;i<MAX_PROCESS; i++)
    {
 	    err = pthread_create(&thread_id, NULL, thread_function, (void*)&passing_thread_data);//쓰레드 아이디 변수에 쓰레드 생성후 해당 아이디 담기
	    if (err != 0)
	    {
		    printf("pthread_create() error.\n");
		    continue;
	    }
        pthread_join(thread_id, &thread_return);
        int thread_return_type_cast=(int)thread_return;
        //printf("스레드에서 thread_return_type_cast %d를 받았습니다.\n",thread_return_type_cast);
         sum_result+=thread_return_type_cast;

    }
    printf("value of fork : %d\n",sum_result);
    clock_gettime(CLOCK_MONOTONIC, &end);
    sec  = end.tv_sec  - begin.tv_sec;
    nsec = end.tv_nsec - begin.tv_nsec;
    if (nsec < 0) nsec += 1000000000;
    diff_time=(double)sec+((double)nsec/1000000000);
    printf("%lf\n", diff_time);
    fclose(f_read);
    return 0;
}
