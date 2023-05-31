#include <mach/mach.h>
#include <mach/mach_error.h>
#include <mach/thread_status.h>
#include <mach/i386/thread_status.h>
#include <mach/machine/thread_status.h>
#include <stdio.h>
#include <unistd.h>
#include <mach/mach_traps.h>
#include <stdlib.h>

#include "tsd.h"


int main() {
        kern_return_t res;
        mach_port_t task = mach_task_self();
	ledger_array_t ledgers;
	mach_msg_type_number_t ledgersCnt = 0;
	boolean_t inherit_memory = 0;
	task_t newtask;
        res = _os_cpu_number();
        printf ("os_cpu %d\n", res);

        OS_GS_RELATIVE void * pb = _os_tsd_get_base();
        printf ("tsd_base %p\n", pb);

	res = task_create(task,ledgers,ledgersCnt,inherit_memory,&newtask);
	if(res != KERN_SUCCESS){
		printf("task_create error: %s\n", mach_error_string(res));
		exit(0);
	}

	res = vm_allocate(newtask, 1, 16384, 0);
	if(res != KERN_SUCCESS){
                printf("vm_allocate error: %s\n", mach_error_string(res));
                exit(0);
        }
	char data[4] = "Hola";
	res = vm_write(newtask, 1, data, 4);
	if(res != KERN_SUCCESS){
                printf("vm_write error: %s\n", mach_error_string(res));
                exit(0);
        }
}
