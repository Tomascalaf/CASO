#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>

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
        int fd = open("/tmp/escriure_disc",O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
        if(fd == -1){
                printf("Error en open\n");
                perror("Open");
                exit(1);
        }


        char *buff = "aaaaaaaaaaaaaaaa";
        res = gettimeofday(&tv0, NULL);
        if (res < 0) {
                perror ("gettimeofday");
        }
        int ret = 0;
        for(long long i = 0; i < (nBytes / 16); i++){
                ret = write(fd,buff,16);

        }
        res = gettimeofday(&tv1, NULL);
        if (res < 0) {
                perror ("gettimeofday");
        }
        ret = close(fd);
        if(ret == -1) printf("Error en close\n");
        // correct formula!!
        secs = (((double)tv1.tv_sec*1000000.0 + (double)tv1.tv_usec) - ((double)tv0.tv_sec*1000000.0 + (double)tv0.tv_usec))/1000000.0;

        printf("%f", secs);

        ret = remove("/tmp/escriure_disc");

        if(ret != 0) {
                printf("Error: unable to delete the file\n");
        }
}

