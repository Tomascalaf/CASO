#include <mach/mach.h>
#include <mach/mach_error.h>
#include <mach/mig_errors.h>
#include <mach/thread_status.h>
#include <mach/processor_info.h>
#include <stdio.h>
#include <stdlib.h>


    /*unsigned int * */ processor_array_t processor_list = NULL;
    mach_msg_type_number_t processor_listCnt = 0;
	processor_basic_info_data_t pbi;

int main ()
{
   int res, i;
   mach_port_t host_privileged_port;
   mach_port_t hostp2; 
   mach_port_t rport;

   res = task_get_host_port (mach_task_self(), &rport);

   //res = host_get_host_port(HOST_PORT, &rport);

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

   printf ("Getting processors at array %p\n", processor_list);

   res = host_processors(host_privileged_port, 
                         &processor_list, &processor_listCnt);
   if (res != KERN_SUCCESS) {
      printf ("Error getting host_processors (0x%x), %s\n", res,
                mach_error_string(res));
      exit(1);
   }



   printf ("        processors at array %p\n", processor_list);
   printf ("processor_listCnt %d\n", processor_listCnt);
   mach_msg_type_number_t tmp = PROCESSOR_BASIC_INFO_COUNT;
   for (i=0; i < processor_listCnt; i++) {
      printf ("processor_list[%d] 0x%x\n", i, processor_list[i]);
      res = processor_info(processor_list[i], PROCESSOR_BASIC_INFO, &hostp2,
                            (processor_info_t) &pbi,&tmp);
      if (res != KERN_SUCCESS) printf("ERROR\n");
      else printf("SUCCESS\n");
      printf("CPU Type %d\n", pbi.cpu_type);
      printf("CPU Subtype %d\n", pbi.cpu_subtype);
      printf("Running %d\n", pbi.running);
      printf("Slot num %d\n", pbi.slot_num);
      printf("Is master %d\n", pbi.is_master);
   }
   printf ("CPU_TYPE_X86 %d\nCPUFAMILY_INTEL_PENRYN 0x%x\nCPU_SUBTYPE_486 %d\n",
            CPU_TYPE_X86, CPUFAMILY_INTEL_PENRYN, CPU_SUBTYPE_486);
   return 0;
}
