#include <linux/module.h>
#include <linux/highmem.h>
#include <linux/kallsyms.h>
#include <linux/sched/mm.h>
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

static asmlinkage pid_t file_varea(const struct pt_regs* regs)
{
	//hooking 후 실행하는 file_varea함수
	struct task_struct* t;
	struct mm_struct* mm; //memory management struct
	struct vm_area_struct * mmap; // vm_area_struct
	char* file_path;
	char * buf;
	struct file* file;
	t = pid_task(find_vpid(regs->di), PIDTYPE_PID);
	mm = get_task_mm(t);
	buf = kmalloc(512, GFP_KERNEL);// using malloc in kernel
	printk("######## Loaded files of a process '%s([%d])' in VM ########\n", t->comm, t->pid);
	mmap = mm->mmap;
	while(mmap)
	{
		memset(buf, 0, 512);
		file = mmap->vm_file;// vma(Virtual Memory Area)에 해당하는 파일에 대한 포인터
		if(file)//vma에 해당하는 파일에 대한 포인터가 null이 아닌경우만 
		{
			file_path = d_path(&file->f_path, buf, 512);
			printk(KERN_INFO "mem[%lx~%lx] code[%lx~%lx] data[%lx~%lx] heap[%lx~%lx] %s \n", mmap->vm_start, mmap->vm_end, mm->start_code, mm->end_code, mm->start_data, mm->end_data, mm->start_brk, mm->brk, file_path);

		}
		mmap = mmap->vm_next;//리스트 형태로 구성됨을 이용
	}
	printk("#################################################################\n");
	kfree(buf);// using free in kernel
	return 0;
}


static int __init hooking_init(void)
{
	//hooking 모듈 적재 함수
	sys_call_table = (sys_call_ptr_t *) kallsyms_lookup_name(system_call_table);
	make_rw(sys_call_table);
	origin_ftrace = sys_call_table[__NR_ftrace];
	sys_call_table[__NR_ftrace] = (sys_call_ptr_t)file_varea;// 시스템 콜 테이블은 long 타입이기 때문에 타입 cast
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
