#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

int main(int argc, char * argv []){
	int res;
   	void *p;
   	struct timeval tv0, tv1;
   	double secs, secs2;
   	int n = 2, i;
   	int howmuch = 10; //499850;

   	if (argc > 1) {
      		fprintf(stderr, "Us: %s\n", argv[0]);
   	   	exit(1);	
   	}

	res = gettimeofday(&tv0, NULL);
   	if (res < 0) { 
      		perror ("gettimeofday");
   	}
	for(int i = 0; i < 100000; i++){
		p = sbrk(0);
	}

	res = gettimeofday(&tv1, NULL);
   	if (res < 0) {
		perror ("gettimeofday");
   	}

  	// correct formula!!
   	secs = (((double)tv1.tv_sec*1000000.0 + (double)tv1.tv_usec) - ((double)tv0.tv_sec*1000000.0 + (double)tv0.tv_usec))/1000000.0;
   	
	printf("%f", secs);
}
