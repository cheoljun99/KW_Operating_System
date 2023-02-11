#include "ftracehooking.h"

#define __NR_ftrace 336

int open_count=0;// open 횟수 카운트 변수
int read_count=0;// read 횟수 카운트 변수
int close_count=0;// close 횟수 카운트 변수
int lseek_count=0;// lseek 횟수 카운트 변수
int write_count=0;// write 횟수 카운트 변수
int read_byte=0; // read 바이트 카운트 변수
int write_byte=0; // write 바이트 카운트 변수
char kernel_buffer[1000] = {0,}; // 커널 버퍼 변수
pid_t given_pid=0; // 프로세스 피아이디를 저장할 변수


EXPORT_SYMBOL(open_count); //ioftracehooking.c에서 증가 시킬 예정
EXPORT_SYMBOL(read_count); //ioftracehooking.c에서 증가 시킬 예정
EXPORT_SYMBOL(close_count); //ioftracehooking.c에서 증가 시킬 예정
EXPORT_SYMBOL(lseek_count); //ioftracehooking.c에서 증가 시킬 예정
EXPORT_SYMBOL(write_count); //ioftracehooking.c에서 증가 시킬 예정
EXPORT_SYMBOL(read_byte); //ioftracehooking.c에서 증가 시킬 예정
EXPORT_SYMBOL(write_byte); //ioftracehooking.c에서 증가 시킬 예정
EXPORT_SYMBOL(kernel_buffer); //ioftracehooking.c에서 증가 시킬 예정
EXPORT_SYMBOL(given_pid); //ioftracehooking.c에서 증가 시킬 예정

typedef asmlinkage long (*sys_call_ptr_t)(const struct pt_regs *);
static sys_call_ptr_t *sys_call_table;

sys_call_ptr_t origin_ftrace;
char *system_call_table = "sys_call_table";

void make_rw(void *addr){
	//시스템콜 테이블에 rw권한을 주는 함수
	unsigned int level;
	pte_t *pte = lookup_address((u64)addr, &level);

	if(pte->pte &~ _PAGE_RW)
		pte->pte |= _PAGE_RW;
}

void make_ro(void *addr){
	//시스템콜 테이블에 rw권한을 뺏는 함수
	unsigned int level;
	pte_t *pte = lookup_address((u64)addr, &level);

	pte->pte = pte->pte &~ _PAGE_RW;
}

char * getprocessname( pid_t input )
{
	//프로세스의 이름을 추출하는 함수
    struct task_struct *task;
    for_each_process( task )
	{
        if(task->pid==given_pid)
		{
			return(task->comm);
		}
    }
    return 0;
}
            

static asmlinkage int ftrace(const struct pt_regs *regs){	
	//hooking 후 실행하는 ftrace함수
	if(regs->di==0)
	{
		//ftrace종료
		printk(KERN_INFO "[2018202065] /%s [%s] statas [x] read - %d / written - %d\n",getprocessname(given_pid),kernel_buffer,read_byte,write_byte);
		printk(KERN_INFO "open[%d] close[%d] read[%d] write[%d] lseek[%d]\n", open_count,close_count,read_count,write_count,lseek_count);
		printk(KERN_INFO "OS Assignment 2 ftrace [%d] End\n", given_pid);
		return 0;
	}
	else
	{
		given_pid = regs->di;
		printk(KERN_INFO "OS Assignment 2 ftrace [%d] Start\n", given_pid);
		return 0;
	}
}

static int __init hooking_init(void)
{
	//hooking 모듈 적재 함수
	sys_call_table = (sys_call_ptr_t *) kallsyms_lookup_name(system_call_table);
	make_rw(sys_call_table);

	origin_ftrace = sys_call_table[__NR_ftrace];
	sys_call_table[__NR_ftrace] = (sys_call_ptr_t)ftrace;// 시스템 콜 테이블은 long 타입이기 때문에 타입 cast

	//printk(KERN_INFO "Operate insmod ftracehooking.\n"); //모듈 적재를 확인하기위한 프린트문
	return 0;
}

static void __exit hooking_exit(void){
	sys_call_table[__NR_ftrace]= origin_ftrace;
	make_ro(sys_call_table);
	//printk(KERN_INFO "Operate rmmod ftracehooking.\n"); //모듈 해제를 확인하기위한 프린트문
}

module_init(hooking_init);
module_exit(hooking_exit);
MODULE_LICENSE("GPL");
