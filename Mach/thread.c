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

char localdata[65536];


// useu _thread_set_tsd_base(localdata) a la funcio del nou thread
// per donar-li espai per les variables thread_local

// useu aquesta funció pel nou thread
//   - podeu posar-li un bucle infinit per veure que consumeix cpu
//   - podeu posar-li un write o un printf per veure que pot fer crides de UNIX
void thread_func(){
	_thread_set_tsd_base(localdata);
	printf("Hola");
	while(1);
}

int main() {
	int res;
	mach_port_t task = mach_task_self();
	mach_port_t newthread;

	res = _os_cpu_number();
	printf ("os_cpu %d\n", res);

        OS_GS_RELATIVE void * pb = _os_tsd_get_base();
	printf ("tsd_base %p\n", pb);

	_STRUCT_X86_THREAD_STATE64 thread_state;
	mach_msg_type_number_t size = x86_THREAD_STATE64_COUNT;

	// creeu el thread
	res = thread_create(task, &newthread);
	if(res != KERN_SUCCESS){
		printf("Error al crear el thread %d\n", res);
	}
	// agafeu el context del thread
	res = thread_get_state(newthread, 4, &thread_state, &size);
        if(res != KERN_SUCCESS){
                printf("Error al obtenir l'estat del thread %d\n", res);
        }
	// exemple de com imprimir alguns registres del context
        printf ("rax 0x%llx\n", thread_state.__rax);
        printf ("rsp 0x%llx\n", thread_state.__rsp);
        printf ("cs 0x%llx\n", thread_state.__cs);
        printf ("fs 0x%llx\n", thread_state.__fs);
        printf ("gs 0x%llx\n", thread_state.__gs);


	// demaneu memoria per a la pila del thread
	unsigned long *memory = malloc(4096);
        printf ("Stack mem [0x%lx - 0x%lx)\n", (unsigned long) memory,
             (unsigned long) memory + 4096);

	// assigneu el registre __rsp apuntant correctament a la pila
	thread_state.__rsp = memory;
        // assigneu el registre __rip apuntant correctament a la funcio del thread
	thread_state.__rip = &thread_func;
	// assigneu el nou context al thread
	res = thread_set_state(newthread, 4, &thread_state, size);
	if(res != KERN_SUCCESS){
                printf("Error al establir l'estat del thread %d\n", res);
        }
	// feu el thread_resume
	res = thread_resume(newthread);
	if(res != KERN_SUCCESS){
                printf("Error al resumir el thread %d\n", res);
	}
	sleep(10);
}
