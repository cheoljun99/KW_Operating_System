#include <linux/module.h>
#include <linux/highmem.h>
#include <linux/kallsyms.h>
#include <linux/syscalls.h>
#include <asm/syscall_wrapper.h>
#include <linux/unistd.h>
#include <linux/string.h>
#include <linux/sched.h>
#include <linux/init_task.h>

#define __NR_ftrace 336

char kernel_buffer[1000] = {0,}; // 커널 버퍼 변수

typedef asmlinkage long (*sys_call_ptr_t)(const struct pt_regs*);
static sys_call_ptr_t *sys_call_table;

sys_call_ptr_t origin_origin_ftrace;
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

static asmlinkage pid_t process_trace(const struct pt_regs* regs)
{
	//hooking 후 실행하는 ftrace함수
	int error_check = 1;
	struct task_struct* task;
	struct task_struct* children_task;
	struct list_head* children_list;
	int children_count = 0;
	struct task_struct* sibling_task;
	struct list_head* sibling_list;
	struct task_struct* parent_task;
	int sibling_count = 0;
	pid_t given_pid = regs->di;// 프로세스 피아이디를 저장할 변수
	
	for_each_process(task)
	{
		if (task->pid == given_pid)
		{
			////////////////////
			error_check = 0;
			///////////////////

			///////////////////////////////////////////////////////////////////////////////
			printk("##### TASK INFORMAITON OF ''[%d] %s '' #####\n", task->pid, task->comm);
			///////////////////////////////////////////////////////////////////////////////

			///////////////////////////////////////////////////////////////////////////////
			if (task->state == 0)
				printk("- task state : Running or ready\n");
			else if (task->state == 1)
				printk("- task state : Wait with ignoring all signals\n");
			else if (task->state == 2)
				printk("- task state : Wait\n");
			else if (task->state == 4)
				printk("- task state : Stopped\n");
			else if (task->state == 32)
				printk("- task state : Zombie process\n");
			else if ((task->state == 16) || (task->state == 128))
				printk("- task state : Dead\n");
			else
				printk("- task state : etc.\n");
			////////////////////////////////////////////////////////////////////////////////////


			/////////////////////////////////////////////////////////////////////////////////////
			printk("- Process Group Leader : [%d] %s\n", task->group_leader->pid, task->group_leader->comm);
			/////////////////////////////////////////////////////////////////////////////////////////////////


			///////////////////////////////////////////////////////////////////////////////////
			printk("- Number of context switches : %ld\n", task->nvcsw + task->nivcsw);
			/////////////////////////////////////////////////////////////////////////////////////

			///////////////////////////////////////////////////////////////////////////////////
			printk("- Number of calling fork() : %d\n", task->fork_count);
			/////////////////////////////////////////////////////////////////////////////////////


			/////////////////////////////////////////////////////////////////////////////////////
			parent_task = task->real_parent;
			printk("- it's parnet process : [%d] %s\n", parent_task->pid, parent_task->comm);
			////////////////////////////////////////////////////////////////////////////////////////////////


			/////////////////////////////////////////////////////////////////////////////////////////////////
			printk("- it's sibling process(es) : \n");
			list_for_each(sibling_list, &parent_task->children)//부모의 자식은 task의 어떠한 형제이다.
			{
				sibling_task = list_entry(sibling_list, struct task_struct, sibling);
				if (sibling_task->pid != task->pid) {
					printk("    > [%d] %s\n", sibling_task->pid, sibling_task->comm);
					sibling_count++;
				}

				// task now points to one of task's sibling
			}
			if (sibling_count == 0) printk("    > It has no sibling.\n");
			printk("    > This process has %d sibling process(es) \n", sibling_count);
			///////////////////////////////////////////////////////////////////////////////////////////////

			/////////////////////////////////////////////////////////////////////////////////////////////////
			printk("- it's children process(es) : \n");
			list_for_each(children_list, &task->children) {
				children_count++;
				children_task = list_entry(children_list, struct task_struct, sibling);
				printk("    > [%d] %s\n", children_task->pid, children_task->comm);
				
				// task now points to one of task's children
			}
			if (children_count == 0) printk("    > It has no child.\n");
			printk("    > This process has %d child process(es) \n",children_count);
			////////////////////////////////////////////////////////////////////////////////////////////////

			///////////////////////////////////////////////////////////////////////////////
			printk("##### END OF INFORMATION #####\n");
			///////////////////////////////////////////////////////////////////////////////


			return(task->pid);
		}
	}

	if (error_check==1)
	{
		printk(KERN_INFO "ERROR CAUSE : Don't print impormation for this PID\n");
		return -1;
	}
	return-1;

}


static int __init hooking_init(void)
{
	//struct task_struct *findtask = &init_task;
	//hooking 모듈 적재 함수
	sys_call_table = (sys_call_ptr_t *) kallsyms_lookup_name(system_call_table);
	make_rw(sys_call_table);

	origin_origin_ftrace = sys_call_table[__NR_ftrace];
	sys_call_table[__NR_ftrace] = (sys_call_ptr_t)process_trace;// 시스템 콜 테이블은 long 타입이기 때문에 타입 cast
	/*
	do
	{
		//printk("%s[%d] ->", findtask->comm, findtask->pid);
		process_trace(findtask->pid);
		findtask = next_task(findtask);
	} while ((findtask->pid != init_task.pid));
	process_trace(findtask->pid);
	*/


	//printk(KERN_INFO "Operate insmod ftracehooking.\n"); //모듈 적재를 확인하기위한 프린트문
	return 0;
}

static void __exit hooking_exit(void){
	sys_call_table[__NR_ftrace]= origin_origin_ftrace;
	make_ro(sys_call_table);
	//printk(KERN_INFO "Operate rmmod ftracehooking.\n"); //모듈 해제를 확인하기위한 프린트문
}

module_init(hooking_init);
module_exit(hooking_exit);
MODULE_LICENSE("GPL");
