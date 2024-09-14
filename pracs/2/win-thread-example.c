#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <Windows.h>

#define THREADS_AMOUNT 5

// Mutex for STDOUT resource
static HANDLE stdout_mutex;

// Structure for function procedure representing its arguments
typedef struct
{
	uint32_t thread_id;
	uint32_t millis;
} thread_args;

void procedure(uint32_t thread_id, uint32_t millis)
{
	// Wait STDOUT mutex to unlock to write a message
	DWORD wait = WaitForSingleObject(stdout_mutex, INFINITE);
	switch (wait)
	{
	case WAIT_OBJECT_0:
		printf("Thread %u started\n", thread_id);
		ReleaseMutex(stdout_mutex);
		break;

	case WAIT_ABANDONED:
		return;
	}
	
	// Function pseudo-code
	clock_t time_start = clock();
	Sleep(millis);
	clock_t time_end = clock();

	// Wait STDOUT mutex to unlock to write a message
	wait = WaitForSingleObject(stdout_mutex, INFINITE);
	switch (wait)
	{
	case WAIT_OBJECT_0:
		printf("Thread %u finished after %u (actual time %u) milliseconds\n", thread_id, millis, time_end - time_start);
		ReleaseMutex(stdout_mutex);
		break;

	case WAIT_ABANDONED:
		return;
	}

	ExitThread(ERROR_SUCCESS);
}

// Interface function for calling in separate threads
void __stdcall thread_procedure(thread_args* args)
{
	procedure(args->thread_id, args->millis);
}

int main(int argc, char** argv)
{
	clock_t time_start = clock();

	// Struct of arrays for new threads
	typedef struct
	{
		HANDLE		threads_handle[THREADS_AMOUNT];
		DWORD		threads_id[THREADS_AMOUNT];
		HANDLE		threads_exit_code[THREADS_AMOUNT];
		thread_args	thread_arguments[THREADS_AMOUNT];
	} threads_array;


	do
	{
		// Get user input
		printf("Enter %lu numbers representing time in milliseconds: ", THREADS_AMOUNT);
		uint32_t threads_delay[THREADS_AMOUNT];
		for (size_t thread_id = 0; thread_id < THREADS_AMOUNT; ++thread_id)
			scanf_s("%lu", &threads_delay[thread_id]);

		stdout_mutex = CreateMutex(NULL, FALSE, NULL);

		// Allocate new threads
		threads_array threads;
		for (uint32_t thread_id = 0; thread_id < THREADS_AMOUNT; ++thread_id)
		{
			threads.thread_arguments[thread_id] = (thread_args){ thread_id + 1, threads_delay[thread_id] };
			threads.threads_handle[thread_id] = CreateThread(
				NULL,
				0,
				thread_procedure,
				&threads.thread_arguments[thread_id],
				0,
				&threads.threads_id[thread_id]
			);
		}

		WaitForMultipleObjects(THREADS_AMOUNT, threads.threads_handle, TRUE, INFINITE);

		// Close resources handles
		for (size_t thread_id = 0; thread_id < THREADS_AMOUNT; ++thread_id)
		{
			DWORD exit_code = 0;
			GetExitCodeThread(threads.threads_handle[thread_id], &exit_code);
			printf("Thread %lu (%lu) done with code %u\n", thread_id + 1, threads.threads_id[thread_id], exit_code);
			CloseHandle(threads.threads_handle[thread_id]);
		}
		CloseHandle(stdout_mutex);

		puts("Enter 'x' to exit or 'r' to retry: ");
		while (getchar() != '\n');

	} while (getchar() == 'r');

	clock_t time_end = clock();
	printf("Program completed within %lu\n", time_end - time_start);

	return 0;
}
