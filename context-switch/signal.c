#include <signal.h>
#include <sys/mman.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include "bench.h"

ull timer_start;
ull timer_end;
pid_t parent;
pid_t child;

#define SIGNAL SIGUSR2

struct process_sync
{
	pthread_mutex_t lock;
	pthread_cond_t cond;
	char child_ready;
	char value;
};

void main_handler(int sig)
{
	unsigned int val;
	timer_end = read_tscp(&val);
	if(sig != SIGNAL)
	{
		/* Not the right signal!! */
		printf("An invalid signal was received.\n");
		return;
	}

	printf("Relay complete.\n");
	printf("Starting cycles: %llu\n", timer_start);
	printf("Ending cycles  : %llu\n", timer_end);
	printf("Cycles to comp : %llu\n", (timer_end - timer_start));

	/* End process */
	exit(0);
}

void child_handler(int sig)
{
	/* Relay the signal */
	kill(parent, sig);
	/* Immediatly end process! */
	exit(0);
}

int main(int argc, char** argv)
{
	struct process_sync* share = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
			MAP_SHARED | MAP_ANONYMOUS | MAP_POPULATE, -1, 0);
	pthread_mutex_init(&share->lock, NULL);
	pthread_cond_init(&share->cond, NULL);
	share->child_ready = 0;
	if(share == (void*)-1)
	{
		perror("");
		return -1;
	}
	parent = getpid();
	child = fork();

	if(child == 0)
	{
		/* Acquire the guard */
		pthread_mutex_lock(&share->lock);
		printf("Initilizing child...\t\t\t\t");

		/* In the child process */
		struct sigaction child_handle;
		if(sigaction(SIGNAL, NULL, &child_handle))
                {
                        printf("[FAIL]\n");
                        return -1;
                }

                child_handle.sa_handler = child_handler;
                if(sigaction(SIGNAL, &child_handle, NULL))
                {
                        printf("[FAIL]\n");
                        return -1;
                }
		printf("[ OK ]\n");
		fflush(stdout);

		/* Set child ready */
		share->child_ready = 1;

		/* Release the lock */
		pthread_mutex_unlock(&share->lock);
		/* Wake the parent if they were sleeping */
		pthread_cond_signal(&share->cond);

		/* We are waiting now. */
		for(;;);
	} else if(child > 0) 
	{
		/* Acquire the guard */
                pthread_mutex_lock(&share->lock);
                printf("Initilizing parent...\t\t\t\t");

		/* Assign the parent signal handler */
		struct sigaction parent_handle;
		if(sigaction(SIGNAL, NULL, &parent_handle))
		{
			perror("");
			return -1;
		}

		parent_handle.sa_handler = main_handler;
		if(sigaction(SIGNAL, &parent_handle, NULL))
		{
			perror("");
			return -1;
		}

		printf("[ OK ]\n");

                printf("Checking for child ready...\t\t\t");
		while(!share->child_ready)
		{
			printf("[WAIT]\n");
			fflush(stdout);
			pthread_cond_wait(&share->cond, &share->lock);
                	printf("Checking for child ready...\t\t\t");
		}
		printf("[DONE]\n");
		printf("Starting experiment...");
		fflush(stdout);

		unsigned int val;
		timer_start = read_tscp(&val);
		kill(child, SIGNAL);

		/* Wait for signal to come back. */	
		for(;;);
	} else {
		printf("Fork failure.\n");
		perror("");
		return -1;
	}

	return 0;
}