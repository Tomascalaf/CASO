#include <mach/mach.h>
#include <mach/mach_error.h>
#include <mach/mig_errors.h>
#include <mach/thread_status.h>
#include <mach/processor_info.h>
#include <stdio.h>
#include <stdlib.h>

#define N_ITER (16)

    processor_array_t processor_lists[N_ITER];
    mach_msg_type_number_t processor_listCnt;

int main ()
{
   int res, i;
   int * p;
   mach_port_t host_privileged_port;
   mach_port_t rport; 

   res = task_get_host_port (mach_task_self(), &rport);
   if (res != KERN_SUCCESS) {
      printf ("Error getting regular host port (0x%x), %s\n", res, 
                mach_error_string(res));
      exit(1);
   }

   res = host_get_host_priv_port(rport, &host_privileged_port);
   if (res != KERN_SUCCESS) {
      printf ("Error getting priv host port (0x%x), %s\n", res, 
                mach_error_string(res));
      exit(1);
   }

   printf ("privileged port: host 0x%x\n", host_privileged_port);

   i = 0;
   while (i < N_ITER) {

      printf ("%d: Getting processors at array %p\n", i, processor_lists[i]);

      res = host_processors(host_privileged_port, 
                         &processor_lists[i], &processor_listCnt);
      if (res != KERN_SUCCESS) {
         printf ("Error getting host_processors (0x%x), %s\n", res,
                   mach_error_string(res));
         exit(1);
      }

      printf ("        processors at array %p (count %d)\n",
                processor_lists[i], processor_listCnt);

      p = (int *) processor_lists[i];
      printf ("        Writing to memory area... %p\n", p);
      *p = 73763;

      p = NULL;
      printf ("%d: Getting memory at location %p\n", i, p);
      res = vm_map(mach_task_self(), (vm_address_t *) &p, 8192, 0, TRUE,
              MACH_PORT_NULL, 0, FALSE, VM_PROT_READ,  VM_PROT_ALL, FALSE);
      if (res != KERN_SUCCESS) {
          printf ("Error getting memory with vm_map(0x%x), %s\n", res,
                    mach_error_string(res));
          exit(1);
      }
      vm_protect(mach_task_self(),(vm_address_t *) p, 8192, FALSE, VM_PROT_ALL);
      if (res != KERN_SUCCESS) {
          printf ("Error changing protection  with vm_protect, %d\n", res);
          exit(1);
      }
      printf ("        New vm_map memory at %p\n", p);
      printf ("        Writing to memory area... %p\n", p);
      *p = 53759;

    ++i;
   }
   return 0;
}
