#include <stdio.h>
#include <stdint.h> 
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/user.h>
#include <sys/mman.h>
#include <time.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

uint8_t* Operation;
uint8_t* compiled_code;

int segment_id;
int fd;
int pagesize = 0;
void sharedmem_init(); // 공유 메모리에 접근
void sharedmem_exit();
void drecompile_init(); // memory mapping 시작 
void drecompile_exit(); 
void* drecompile(uint8_t *func); //최적화하는 부분

int main(void)
{
	pagesize = getpagesize();
	int (*func)(int a);
	int i;
	struct timespec begin, end; //시간을 측정하기위한 변수
	time_t sec;
	long nsec;
	sharedmem_init();
	drecompile_init();
	func = (int (*)(int a))drecompile(Operation);
	clock_gettime(CLOCK_MONOTONIC, &begin);
	func(1);//compiled code excute
	clock_gettime(CLOCK_MONOTONIC, &end);
	sec = end.tv_sec - begin.tv_sec;
	nsec = end.tv_nsec - begin.tv_nsec;
	if (nsec < 0) nsec += 1000000000;
	printf("total excution time: %3ld.%09ld sec\n", sec, nsec);
	drecompile_exit();
	sharedmem_exit();
	return 0;
}

void sharedmem_init()
{
	segment_id = shmget(1234, pagesize, 0);  // get shared memory
	Operation = (uint8_t*)shmat(segment_id, NULL, 0);  // attach
}

void sharedmem_exit()
{
	shmdt(Operation);  // detach
	shmctl(segment_id, IPC_RMID, NULL);  // remove shared memory
}

void drecompile_init(uint8_t* func)
{
	char temp[pagesize];
	fd = open("Operation.txt", O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IXUSR);
	/*아래는 bus error 방지 과정*/
	for(int n = 0; n < PAGE_SIZE; n++) temp[n] = 'M';
	write(fd, temp, pagesize);
	lseek(fd, 0, 0);
	/***************************/
	compiled_code = mmap(0, pagesize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);  // memory mapping 
	assert(compiled_code!=MAP_FAILED); //error detection
	memset(compiled_code,0,pagesize); //메모리 매핑한 공간 초기화
	msync(compiled_code,pagesize,MS_SYNC);//메모리 매핑 후 변경된 사항을 파일에 반영 (동기화) MS_SYNC 정책
}


void drecompile_exit()
{
	munmap(compiled_code, pagesize);  // unmap
}

void* drecompile(uint8_t* func)
{
	int dynamic_check=0;
#ifdef dynamic
	dynamic_check=1;
	int i = 0;
	int j = 0;
	// for all function instruction
	uint8_t dl=0;
	int dl_register=0;

	while (func[i] != 0xc3)
	{
		if(func[i]==0xb2) //finding dl register
		{
			dl=func[i+1];
			dl_register=i;

		}
		if (func[i] != 0x83 && func[i] != 0x6b && func[i] != 0xf6)
		{
			// operation, source register
			// op is not target of optimization
			compiled_code[j++] = func[i++];
		}
		else 
		{
			uint8_t add_count=0;
			uint8_t sub_count=0;
			uint8_t imul_count=1;
			uint8_t div_count=1;
			int k = 0;
			if (func[i] == 0x83)
			{
				if (func[i + 1] == 0xc0)//add
				{
					k = i + 3;
					add_count+=func[i+2];
					while (func[k] == func[i] && func[k+1] == func[i+1])
					{
						add_count+=func[k+2];
						k = k + 3;
					}
					compiled_code[j++]=func[i];
					compiled_code[j++]=func[i+1];
					compiled_code[j++]=add_count;
					i=k;
					continue;
				}
				else if (func[i + 1] == 0xe8)//sub
				{
					k = i + 3;
					sub_count += func[i+2];
					while (func[k] == func[i] && func[k+1] == func[i+1])
					{
						sub_count += func[k+2];
						k = k + 3;
					}

					compiled_code[j++]=func[i];
					compiled_code[j++]=func[i+1];
					compiled_code[j++]=sub_count;
					i=k;
					continue;
				}
				else//exception handling
				{
					compiled_code[j++] = func[i++];
					continue;
				}
			}
			else if (func[i] == 0x6b)
			{
				if (func[i + 1] == 0xc0)//imul
				{
					k = i + 3;
					imul_count*=func[i+2];
					while (func[k] == func[i] && func[k+1] == func[i+1])
					{
						imul_count*=func[k+2];
						k = k + 3;
					}
					compiled_code[j++]=func[i];
					compiled_code[j++]=func[i+1];
					compiled_code[j++]=imul_count;
					i=k;
					continue;
				}
				else//exception handling
				{
					compiled_code[j++] = func[i++];
					continue;
				}

			}
			else if (func[i] == 0xf6)
			{
				if (func[i + 1] == 0xf2)//div
				{
					k = i + 2;
					div_count*=dl;
					while (func[k] == func[i] && func[k+1] == func[i+1])
					{
						div_count*=dl;
						k = k + 2;
					}
					compiled_code[j++]=func[i];
					compiled_code[j++]=func[i+1];
					compiled_code[dl_register+1]=div_count;
					i=k;
					continue;
				}
				else//exception handling
				{
					compiled_code[j++] = func[i++];
					continue;
				}
			}
		}
	}
	// end of func
	compiled_code[j] = func[i];//end
	msync(compiled_code,pagesize,MS_SYNC);
#endif
	if(dynamic_check==0)// case of no dynamic
	{
		//copiled_code 기존 그대로 매핑한 영역에 저장
		int b=0;
		while (func[b] != 0xc3)
		{
			compiled_code[b] = func[b];
			b++;
		}
		compiled_code[b] = func[b];
		msync(compiled_code,pagesize,MS_SYNC);//메모리 매핑 후 변경된 사항을 파일에 반영 (동기화) MS_SYNC 정책
		
	}
	if(mprotect(compiled_code, pagesize, PROT_READ | PROT_EXEC))// set protection RX
	{
	printf("set protection RX error using mprotect\n");
	}
	return compiled_code;
}

