#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>

// Rep com a parametre el numero de MB a llegir
int main(int argc, char * argv []){
        int res;
        void *p;
        struct timeval tv0, tv1;
        double secs, secs2;
        int n = 2, i;
        int howmuch = 10; //499850;

        if (argc != 2) {
                fprintf(stderr, "Us: %s MegaBystes\n", argv[0]);
                exit(1);
        }
        int nBytes = atoi(argv[1]) * 1024 * 1024;
        int fd = open("fitxerLlegir", O_RDONLY);
        if(fd == -1){
                printf("Error en open\n");
                perror("Open");
                exit(1);
        }


        char buff[16];
        res = gettimeofday(&tv0, NULL);
        if (res < 0) {
                perror ("gettimeofday");
        }
        int ret = 0;
	for(int i = 0; i < nBytes/16; i++){
		ret = read(fd, buff, 16);
	}
	if(ret < 0){
		perror("read");
	}
        res = gettimeofday(&tv1, NULL);
        if (res < 0) {
                perror ("gettimeofday");
        }
        ret = close(fd);
        if(ret == -1) perror("Error en close");
        // correct formula!!
        secs = (((double)tv1.tv_sec*1000000.0 + (double)tv1.tv_usec) - ((double)tv0.tv_sec*1000000.0 + (double)tv0.tv_usec))/1000000.0;

        printf("%f", secs);
}
