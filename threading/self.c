#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#include "bench.h"

void* pthread_func(void* argument)
{
	ull high;
	ull low;

	ull start;
	ull end;

	unsigned long diff;
	unsigned long best = (unsigned long)-1;
	
	int x;
	for(x = 0;x < 10000;x++)
	{
		RDTSC(start);
		pthread_t t = pthread_self();
		RDTSC(end);
		diff = end - start;
		if(diff < best) best = diff;
		if(!t) perror("");
	}

	diff = best;
	int file = open("output.txt", O_APPEND | O_RDWR | O_CREAT, 0644);
        if(file < 0) printf("BAD FILE!\n");
        char numbuffer[512];
        snprintf(numbuffer, 512, "%lu\n", diff);
        write(file, numbuffer, strlen(numbuffer));
        close(file);

	return NULL;
}

int main(int argc, char** argv)
{
	pthread_t thr;

	if(pthread_create(&thr, NULL, pthread_func, NULL ))
		perror("");

	pthread_join(thr, (void**)NULL);
	return 0;

}
