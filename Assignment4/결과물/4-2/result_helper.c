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

int main(void)
{
	for(int i=0;i<50;i++)
    {
		int ret1 = system("sync");
		int ret2 = system("echo 3 | sudo tee /proc/sys/vm/drop_caches");
        int ret3 = system("./test2");
        int ret4 = system("make");
        int ret5 = system("./drecompile");
        int ret6 = system("sync");
		int ret7 = system("echo 3 | sudo tee /proc/sys/vm/drop_caches");
        int ret8 = system("./test2");
        int ret9 = system("make dynamic");
        int ret10 = system("./drecompile");
    }
	return 0;
}