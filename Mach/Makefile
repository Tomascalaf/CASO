all: proc memory-management thread task pid_mem

proc: proc.c 
	gcc -D_GNU_SOURCE -O -o proc proc.c

memory-management: memory-management.c
	gcc -D_GNU_SOURCE -O -o memory-management memory-management.c

thread: thread.c
	gcc -D_GNU_SOURCE -O -o thread thread.c

task: task.c
	gcc -D_GNU_SOURCE -O -o task task.c

pid_mem: pid_mem.c
	gcc -D_GNU_SOURCE -O -o pid_mem pid_mem.c

clean:
	rm -f proc memory-management thread task pid_mem
