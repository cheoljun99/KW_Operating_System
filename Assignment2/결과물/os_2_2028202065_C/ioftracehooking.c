#include "ftracehooking.h"

extern int open_count; //EXPORT_SYMBOL 에 대한 extern
extern int read_count; //EXPORT_SYMBOL 에 대한 extern
extern int close_count; //EXPORT_SYMBOL 에 대한 extern
extern int lseek_count; //EXPORT_SYMBOL 에 대한 extern
extern int write_count; //EXPORT_SYMBOL 에 대한 extern
extern int read_byte; //EXPORT_SYMBOL 에 대한 extern
extern int write_byte; //EXPORT_SYMBOL 에 대한 extern
extern char kernel_buffer[1000]; //EXPORT_SYMBOL 에 대한 extern
extern pid_t given_pid; //EXPORT_SYMBOL 에 대한 extern

pid_t getPID(void) // 유저모드에서 getpid와 같은 역할을 함
{
    pid_t pid = 0;
    struct task_struct *task=current;//이 부분으로 인해 해당 프로세스에서의 pid 가져올 수 있음
    pid = task->pid;
    return pid;
}

typedef asmlinkage long (*sys_call_ptr_t)(const struct pt_regs *);
static sys_call_ptr_t *sys_call_table;

sys_call_ptr_t origin_open;
sys_call_ptr_t origin_read;
sys_call_ptr_t origin_write;
sys_call_ptr_t origin_close;
sys_call_ptr_t origin_lseek;
char *system_call_table = "sys_call_table";

/* 이것은 ftracehooking.c에 이미 정의했기 때문에 또 하면 killed 에러남
void make_rw(void *addr){
	unsigned int level;
	pte_t *pte = lookup_address((u64)addr, &level);

	if(pte->pte &~ _PAGE_RW)
		pte->pte |= _PAGE_RW;
}
*/
/*
void make_ro(void *addr){
	unsigned int level;
	pte_t *pte = lookup_address((u64)addr, &level);

	pte->pte = pte->pte &~ _PAGE_RW;
}
*/

static asmlinkage long ftrace_open(const struct pt_regs *regs)
{

	if(given_pid==getPID())
	{
		//유저 공간에 위치한 문자열 메모리 공간을 커널에서 이용하기 위하여
		//커널 공간에 버퍼를 잡고 문자열을 복사하는 과정.
		open_count++;//프로세스의 pid와 같을 때만 카운트
		strncpy_from_user(&kernel_buffer[0], (const char __user*)regs->di, sizeof(kernel_buffer) - 1);
		kernel_buffer[sizeof(kernel_buffer) - 1] = '\0';
		//printk("[+][chmod] filename :%s \n", kernel_buffer);
	}
	return origin_open(regs);
}
static asmlinkage long ftrace_read(const struct pt_regs *regs)
{
	//printk(KERN_INFO "OS Assignment2 ftrace_read Start\n");
	if(given_pid==getPID())
	{
		read_byte =read_byte+((int)regs->dx);//read 바이트를 카운트 하는 과정
		read_count++;//프로세스의 pid와 같을 때만 카운트
	}
	return origin_read(regs);
}
static asmlinkage long ftrace_write(const struct pt_regs *regs)
{
	//printk(KERN_INFO "OS Assignment2 ftrace_write Start\n");
	if(given_pid==getPID())
	{
		write_byte =write_byte+((int)regs->dx);// write 바이트를 카운트 하는 과정
		write_count++;//프로세스의 pid와 같을 때만 카운트
	}
	return origin_write(regs);
}
static asmlinkage long ftrace_lseek(const struct pt_regs *regs)
{
	//printk(KERN_INFO "OS Assignment2 ftrace_lseek Start\n");
	
	if(given_pid==getPID())
	{
		lseek_count++;//프로세스의 pid와 같을 때만 카운트
	}
	return origin_lseek(regs);
}
static asmlinkage long ftrace_close(const struct pt_regs *regs)
{
	//printk(KERN_INFO "OS Assignment2 ftrace_close Start\n");
	if(given_pid==getPID())
	{
		close_count++;//프로세스의 pid와 같을 때만 카운트
	}
	return origin_close(regs);
}

static int __init hooking_init(void)
{
	//모듈 적재함수
	sys_call_table = (sys_call_ptr_t *) kallsyms_lookup_name("sys_call_table");
	//make_rw(sys_call_table);//이것은 ftracehooking.c에 이미 정의했기 때문에 또 하면 killed 에러남
	origin_open=sys_call_table[__NR_open];
	origin_read=sys_call_table[__NR_read];
	origin_write=sys_call_table[__NR_write];
	origin_lseek=sys_call_table[__NR_lseek];
	origin_close=sys_call_table[__NR_close];
	sys_call_table[__NR_open]=ftrace_open;
	sys_call_table[__NR_read]=ftrace_read;
	sys_call_table[__NR_write]=ftrace_write;
	sys_call_table[__NR_lseek]=ftrace_lseek;
	sys_call_table[__NR_close]=ftrace_close;
	//printk(KERN_INFO "Operate insmod ioftracehooking.\n");//ioftrace.c 모듈 적재를 확인하기 위한 프린트문
	return 0;
}

static void __exit hooking_exit(void)
{
	//모듈 해제함수
	sys_call_table[__NR_open]=origin_open;
	sys_call_table[__NR_read]=origin_read;
	sys_call_table[__NR_write]=origin_write;
	sys_call_table[__NR_lseek]=origin_lseek;
	sys_call_table[__NR_close]=origin_close;
	//make_ro(sys_call_table);//이것은 ftracehooking.c에 이미 정의했기 때문에 또 하면 killed 에러남
	//printk(KERN_INFO "Operate rmmod ioftracehooking.\n"); //모듈 해제를 확인하기 위한 프린트문
}

module_init(hooking_init);
module_exit(hooking_exit);
MODULE_LICENSE("GPL");
