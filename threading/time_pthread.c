#include <pthread.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "bench.h"

void* pthread_func(void* argument)
{
	pthread_exit(NULL);
	return NULL;
}

void time_pthread(){
	pthread_t thr;

	uint val;
	ull high;
	ull low;
	unsigned long diff;
	ull start;
	ull end;

	/* Timing section */
	RDTSC(start);
	pthread_create(&thr, NULL, pthread_func, NULL);	
	pthread_join(thr, (void**)NULL);
	RDTSC(end);
	/* End section */

	diff = end - start;
	printf("Start cycles: %llu\n", start);
	printf("End cycles  : %llu\n", end);
	printf("Difference  : %llu\n", (end - start));

        int file = open("output.txt", O_APPEND | O_RDWR | O_CREAT, 0644);
        if(file < 0) printf("BAD FILE!\n");
        char numbuffer[512];
        snprintf(numbuffer, 512, "%lu\n", diff);
        write(file, numbuffer, strlen(numbuffer));
        close(file);
}

int main(int argc, char** argv)
{
	time_pthread();
	return 0;

}
