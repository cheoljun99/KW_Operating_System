#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/unistd.h>

#define __NR_ftrace 336

int main()
{
	syscall(__NR_ftrace,1); 
	
	return 0;

}
