#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>

#define THREADS_AMOUNT 5

static pthread_mutex_t stdout_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutexattr_t stdout_mutex_attr;

// Structure for function procedure representing its arguments
typedef struct
{
	uint32_t thread_id;
	uint32_t millis;
    uint32_t retval;
} thread_args;

int procedure(uint32_t thread_id, uint32_t millis)
{
	// Wait STDOUT mutex to unlock to write a message
    uint32_t error = pthread_mutex_lock(&stdout_mutex);
    if (error == EDEADLK)
    	return -1;
	
	printf("Thread %u started\n", thread_id);
    error = pthread_mutex_unlock(&stdout_mutex);
    if (error == EPERM)
	    return -1;


	// Function pseudo-code
	struct timespec time_start, time_end;
    clock_gettime(CLOCK_REALTIME, &time_start);
	usleep(millis * 1000);
    clock_gettime(CLOCK_REALTIME, &time_end);

	// Wait STDOUT mutex to unlock to write a message
	error = pthread_mutex_lock(&stdout_mutex);
    if (error == EDEADLK)
		return -1;

	printf("Thread %u finished after %u (actual time %lu) milliseconds\n", thread_id, millis, (time_end.tv_nsec - time_start.tv_nsec) / CLOCKS_PER_SEC);
	
    error = pthread_mutex_unlock(&stdout_mutex);
    if (error == EPERM)
		return -1;

	return 0;
}

// Interface function for calling in separate threads
void thread_procedure(thread_args* args)
{
	args->retval = procedure(args->thread_id, args->millis);
    pthread_exit(&args->retval);
}


int main()
{
    pthread_attr_t threads_attr;
	struct timespec time_start, time_end;
    clock_gettime(CLOCK_REALTIME, &time_start);

	// Struct of arrays for new threads
	typedef struct
	{
		pthread_t	    threads_id[THREADS_AMOUNT];
		uint32_t	    threads_exit_code[THREADS_AMOUNT];
		thread_args	    threads_arguments[THREADS_AMOUNT];
	} threads_array;

    // Initializing mutex
	if (pthread_mutexattr_init(&stdout_mutex_attr) != 0)
    {
        perror("[-] Error initializing mutex context ");
        return -1;
    }

    if (pthread_mutex_init(&stdout_mutex, &stdout_mutex_attr) != 0)
    {
        perror("[-] Error initializing mutex ");
        return -1;
    }

    if (pthread_attr_init(&threads_attr) != 0)
	{
        perror("[-] Error initializing threads context ");
        return -1;
    }

	do
	{
		// Get user input
		printf("Enter %u numbers representing time in milliseconds: ", THREADS_AMOUNT);
		uint32_t threads_delay[THREADS_AMOUNT];
		for (size_t thread_id = 0; thread_id < THREADS_AMOUNT; ++thread_id)
			scanf("%u", &threads_delay[thread_id]);

		// Allocate new threads
		threads_array threads;
		for (uint32_t thread_id = 0; thread_id < THREADS_AMOUNT; ++thread_id)
		{
			threads.threads_arguments[thread_id] = (thread_args){ thread_id + 1, threads_delay[thread_id] };
            pthread_create(
				&threads.threads_id[thread_id],
                &threads_attr,
                thread_procedure,
                &threads.threads_arguments[thread_id]
			);
		}

		// Close resources handles
		for (size_t thread_id = 0; thread_id < THREADS_AMOUNT; ++thread_id)
		{
            uint8_t *pthread_memory;
			pthread_join(threads.threads_id[thread_id], NULL);
			// pthread_join(threads.threads_id[thread_id], &pthread_memory);

			printf("Thread %lu (0x%016lx) done with code %s\n", thread_id + 1, threads.threads_id[thread_id], pthread_memory);
            free(pthread_memory);
		}

		puts("Enter 'x' to exit or 'r' to retry (default 'x'): ");
		while (getchar() != '\n');

	} while (getchar() == 'r');

    pthread_mutex_destroy(&stdout_mutex);
    pthread_mutexattr_destroy(&stdout_mutex_attr);
    pthread_attr_destroy(&threads_attr);

    clock_gettime(CLOCK_REALTIME, &time_end);
	printf("Program completed within %lu\n", (time_end.tv_nsec - time_start.tv_nsec) / CLOCKS_PER_SEC);

    return 0;
}
