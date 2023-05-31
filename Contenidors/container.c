#include <sys/types.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sched.h>
#include <linux/sched.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

   struct args {
      char * argument0;
      char ** argument1;
   };

// this function seems not to be declared in Ubuntu
extern int clone (int (*__fn) (void *__arg), void *__child_stack,
                  int __flags, void *__arg, ...) __THROW;

char stack_area[65536] __attribute__((aligned(16)));
void * stack = &stack_area[60*1024];

int func(void * a)
{
   struct args * args = (struct args *) a;
   int pid, status, res;

   // lets wait till parent sets us the proper uid/gid maps
   sleep (3);

   // let parent print values, and set user ids properly

   printf ("child: parent pid %d, child %d\n", getppid(), getpid());

   printf ("child: changing root to %s\n", args->argument0);

   // Afegiu aqui el codi per fer un chroot al directori representat per arg[0]
   res = chroot(args->argument0);
   if(res == -1){
   	perror("chroot");
	exit(1);
   }else{
   	printf("Changed root to %s\n", args->argument0);
   }
   // Fi del codi del chroot

   printf ("child: changing curdir to /\n");

   // Afegiu aqui el codi per fer un chdir al mateix directori

   res = chdir("/");
   if(res == -1){
   	perror("chdir");
	exit(1);
   }

   // Fi del codi del chdir

   printf ("child: executing init process %s\n", args->argument1[0]);
   // Afegiu aqui el codi per executar el programa (binari) representat per arg[1], useu execvp(...)

   res = execvp(args->argument1[0], args->argument1);
   if(res == -1){
   	perror("execvp");
	exit(1);
   }

   // Fi del codi del execlp

   return 0;
}

int main(int argc, char * argv [])
{
   int pid, status, res;
   struct stat st;
   struct args args;

   if (argc<3) {
      printf ("Us: %s <chroot-directory-to-rootfs> <init-process> [argument]...\n", argv[0]);
      exit(1);
   }
   res = stat (argv[1], &st);
   if (res < 0) { 
      perror ("stat - chroot-dir");
      exit(1);
   }
   if (!(S_ISDIR(st.st_mode))) {
      fprintf(stderr, "error: file %s is not a directory\n", argv[1]);
      exit(1);
   }

   args.argument0 = argv[1];
   args.argument1 = &argv[2];

   pid = clone(func, stack, CLONE_NEWCGROUP |
             CLONE_NEWIPC | CLONE_NEWNET | CLONE_NEWNS | CLONE_NEWPID | 
             CLONE_NEWUSER | CLONE_NEWUTS | SIGCHLD, &args);
   if (pid < 0) perror ("clone");
   else {

      sleep(1);

      // Afegiu aqui el codi per escriure la cadena "0 0 65535\n" 
      // o alternativament "0 0 4294967295\n", mireu-ho en els processos
      // que estan corrent en el vostre sistema, als
      // fitxers /proc/<pid>/uid_map  i  /proc/<pid>/gid_map
      // de forma que el contenidor mantingui aquest rang d'usuaris com a valid

      char * cadena = "0 0 4294967295\n";

      char uid_map[24];
      sprintf(uid_map, "/proc/%d/uid_map", pid);
      char gid_map[24];
      sprintf(gid_map, "/proc/%d/gid_map", pid);
      int fd_uid = open(uid_map, O_RDWR);
      int fd_gid = open(gid_map, O_RDWR);
      res = write(fd_uid, cadena, 15);
      if (res == -1){
         perror("write uid_map");
	 exit(1);
      }

      res = write(fd_gid, cadena, 15);
      if (res == -1){
         perror("write gid_map");
         exit(1);
      }

      // Fi del codi per escriure l'uid_map/gid_map

      printf ("Parent: parent %d, child %d, waiting...\n", getpid(), pid);

      res = wait(&status);
      if (res < 0) {
         perror ("wait");
      }
   }
   printf ("Parent: finishing\n");
   return 0;
}
