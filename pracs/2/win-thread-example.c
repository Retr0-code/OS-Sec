#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <Windows.h>

#define THREADS_AMOUNT 5

static HANDLE stdout_mutex;

typedef struct
{
	uint32_t thread_id;
	uint32_t millis;
} thread_args;

void procedure(uint32_t thread_id, uint32_t millis)
{
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
	
	clock_t time_start = clock();
	Sleep(millis);
	clock_t time_end = clock();

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

void __stdcall thread_procedure(thread_args* args)
{
	procedure(args->thread_id, args->millis);
}

int main(int argc, char** argv)
{
	clock_t time_start = clock();

	typedef struct
	{
		HANDLE		threads_handle[THREADS_AMOUNT];
		DWORD		threads_id[THREADS_AMOUNT];
		HANDLE		threads_exit_code[THREADS_AMOUNT];
		thread_args	thread_arguments[THREADS_AMOUNT];
	} threads_array;

	printf("Enter %lu numbers representing time in milliseconds: ", THREADS_AMOUNT);
	uint32_t threads_delay[THREADS_AMOUNT];
	for (size_t thread_id = 0; thread_id < THREADS_AMOUNT; ++thread_id)
		scanf_s("%ul", &threads_delay[thread_id]);

	stdout_mutex = CreateMutex(NULL, FALSE, NULL);

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

	for (size_t thread_id = 0; thread_id < THREADS_AMOUNT; ++thread_id)
	{
		DWORD exit_code = 0;
		GetExitCodeThread(threads.threads_handle[thread_id], &exit_code);
		printf("Thread %lu (%lu) done with code %u\n", thread_id + 1, threads.threads_id[thread_id], exit_code);
		CloseHandle(threads.threads_handle[thread_id]);
	}
	CloseHandle(stdout_mutex);


	clock_t time_end = clock();
	return 0;
}
