#include <chrono>
#include <cstdint>
#include <iostream>
#include <unistd.h>
#include <pthread.h>

#define THREADS_AMOUNT 5

// Mutex for STDOUT resource
static pthread_mutex_t stdout_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutexattr_t stdout_mutex_attr;


// Structure for function procedure representing its arguments
class thread_args
{
public:
	uint32_t thread_id;
	uint32_t millis;
    uint32_t retval;

    thread_args(void) : thread_id{0}, millis{0}, retval{0} {  };
    thread_args(uint32_t thread_id, uint32_t millis)
        : thread_id{thread_id}, millis{millis}, retval{0} {  }
};

int procedure(uint32_t thread_id, uint32_t millis)
{
    using namespace std::chrono;

	// Wait STDOUT mutex to unlock to write a message
	uint32_t error = pthread_mutex_lock(&stdout_mutex);
    if (error == EDEADLK)
        return -1;
	
    std::cout << "Thread " << thread_id << " started\n";
	
    error = pthread_mutex_unlock(&stdout_mutex);
    if (error == EPERM)
        return -1;
	
	// Function pseudo-code
	auto start = steady_clock::now();
	usleep(millis);
	auto end = steady_clock::now();

	// Wait STDOUT mutex to unlock to write a message
	error = pthread_mutex_lock(&stdout_mutex);
    if (error == EDEADLK)
        return -1;

	std::cout << "Thread " << thread_id << " finished after " << millis << " (actual time " << duration_cast<milliseconds>(end - start).count() << ") milliseconds\n";
    
    error = pthread_mutex_unlock(&stdout_mutex);
    if (error == EPERM)
        return -1;

    return 0;
}

// Interface function for calling in separate threads
void *thread_procedure(void *args)
{
    thread_args *args_struct = reinterpret_cast<thread_args*>(args);
    args_struct->retval = procedure(args_struct->thread_id, args_struct->millis);
    pthread_exit(&args_struct->retval);
}

int main(int argc, char** argv)
{
    // Struct of arrays for new threads
    class threads_array
    {
    public:
        pthread_t	    threads_id[THREADS_AMOUNT];
        uint32_t	    threads_exit_code[THREADS_AMOUNT];
        thread_args	    threads_arguments[THREADS_AMOUNT];

        threads_array(void) = default;
    };

    pthread_attr_t threads_attr;

    // Initializing mutex
    if ((errno = pthread_mutexattr_init(&stdout_mutex_attr)) != 0)
    {
        perror("[-] Error initializing mutex context ");
        return -1;
    }

    if ((errno = pthread_mutex_init(&stdout_mutex, &stdout_mutex_attr)) != 0)
    {
        perror("[-] Error initializing mutex ");
        return -1;
    }

    if ((errno = pthread_attr_init(&threads_attr)) != 0)
    {
        perror("[-] Error initializing threads context ");
        return -1;
    }

	do
	{
		// Get user input
		std::cout << "Enter " << THREADS_AMOUNT << " numbers representing time in milliseconds: ";
		uint32_t threads_delay[THREADS_AMOUNT];
		for (size_t thread_id = 0; thread_id < THREADS_AMOUNT; ++thread_id)
			std::cin >> threads_delay[thread_id];

		// Allocate new threads
        threads_array threads;
        for (uint32_t thread_id = 0; thread_id < THREADS_AMOUNT; ++thread_id)
        {
            threads.threads_arguments[thread_id] = thread_args{ thread_id + 1, threads_delay[thread_id] };
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
            int *pthread_memory;
            pthread_join(threads.threads_id[thread_id], reinterpret_cast<void**>(&pthread_memory));

            std::cout << "Thread " << thread_id + 1 << " (0x" << std::hex << threads.threads_id[thread_id] << std::dec << ") done with code " << *pthread_memory << '\n';
        }

		std::cout << "Enter 'x' to exit or 'r' to retry (default 'x'): ";
		while (std::getchar() != '\n');

	} while (std::getchar() == 'r');
    pthread_mutex_destroy(&stdout_mutex);
    pthread_mutexattr_destroy(&stdout_mutex_attr);
    pthread_attr_destroy(&threads_attr);

	return 0;
}
