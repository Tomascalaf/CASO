#include <mach/mach.h>
#include <mach/mach_error.h>
#include <stdio.h>
#include <unistd.h>
#include <mach/mach_traps.h>
#include <stdlib.h>

void usage(){
	printf("Usage: ./pid_mem <pid> <adreça>\n");
}

// Parametres:
//	pid
//	adreça
int main(int argc, char *argv[]) {
        if (argc < 3) {
		usage();
		exit(1);
	}

	int pid = atoi(argv[1]);
	vm_address_t addr = atoi(argv[2]);
	kern_return_t kret;
	mach_port_t task;
	unsigned int datacount;
	vm_offset_t  *data;
	char test1[vm_page_size];

	kret = task_for_pid(mach_task_self(), pid, &task);
	if(kret != KERN_SUCCESS){
		printf("task_for_pid() error: %s\n", mach_error_string(kret));
	}

	kret = vm_read(task, addr, vm_page_size, data, &datacount);
	if(kret != KERN_SUCCESS){
		printf("vm_read() error: %s\n", mach_error_string(kret));
	}else{
		memcpy(test1, (const void *)data, datacount);
		printf("%s\n", test1);
	}
}
