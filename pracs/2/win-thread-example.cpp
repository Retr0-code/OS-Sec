#include <chrono>
#include <cstdint>
#include <iostream>
#include <windows.h>

#define THREADS_AMOUNT 5

// Mutex for STDOUT resource
static HANDLE stdout_mutex;

// Structure for function procedure representing its arguments
class thread_args
{
public:
	uint32_t thread_id;
	uint32_t millis;

    thread_args(void) : thread_id{0}, millis{0} {  };
    thread_args(uint32_t thread_id, uint32_t millis) : thread_id{thread_id}, millis{millis} {  }
};

void procedure(uint32_t thread_id, uint32_t millis)
{
    using namespace std::chrono;

	// Wait STDOUT mutex to unlock to write a message
	DWORD wait = WaitForSingleObject(stdout_mutex, INFINITE);
	switch (wait)
	{
	case WAIT_OBJECT_0:
		std::cout << "Thread " << thread_id << " started\n";
		ReleaseMutex(stdout_mutex);
		break;

	case WAIT_ABANDONED:
		ExitThread(ERROR_INVALID_HANDLE);
	}
	
	// Function pseudo-code
	auto start = steady_clock::now();
	Sleep(millis);
	auto end = steady_clock::now();

	// Wait STDOUT mutex to unlock to write a message
	wait = WaitForSingleObject(stdout_mutex, INFINITE);
	switch (wait)
	{
	case WAIT_OBJECT_0:
		std::cout << "Thread " << thread_id << " finished after " << millis << " (actual time " << duration_cast<milliseconds>(end - start).count() << ") milliseconds\n";
		ReleaseMutex(stdout_mutex);
		break;

	case WAIT_ABANDONED:
		ExitThread(ERROR_INVALID_HANDLE);
	}

	ExitThread(ERROR_SUCCESS);
}

// Interface function for calling in separate threads
unsigned long __stdcall thread_procedure(LPVOID args)
{
	procedure(reinterpret_cast<thread_args*>(args)->thread_id, reinterpret_cast<thread_args*>(args)->millis);
    return ERROR_SUCCESS;
}

int main(int argc, char** argv)
{
    // Struct of arrays for new threads
    class threads_array
    {
    public:
        HANDLE		threads_handle[THREADS_AMOUNT];
        DWORD		threads_id[THREADS_AMOUNT];
        HANDLE		threads_exit_code[THREADS_AMOUNT];
        thread_args	thread_arguments[THREADS_AMOUNT];

        threads_array(void) = default;
    };

	do
	{
		// Get user input
		std::cout << "Enter " << THREADS_AMOUNT << " numbers representing time in milliseconds: ";
		uint32_t threads_delay[THREADS_AMOUNT];
		for (size_t thread_id = 0; thread_id < THREADS_AMOUNT; ++thread_id)
			std::cin >> threads_delay[thread_id];

		stdout_mutex = CreateMutex(NULL, FALSE, NULL);

		// Allocate new threads
		threads_array threads;
		for (uint32_t thread_id = 0; thread_id < THREADS_AMOUNT; ++thread_id)
		{
			threads.thread_arguments[thread_id] = thread_args{ thread_id + 1, threads_delay[thread_id] };
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
			if (threads.threads_handle[thread_id] == NULL)
				exit_code = -1;
			else
			{
				GetExitCodeThread(threads.threads_handle[thread_id], &exit_code);
				CloseHandle(threads.threads_handle[thread_id]);
			}

			std::cout << "Thread " << thread_id + 1 << " (" << threads.threads_id[thread_id] << ") done with code " << exit_code << '\n';
		}
		if (stdout_mutex != NULL)
			CloseHandle(stdout_mutex);

		std::cout << "Enter 'x' to exit or 'r' to retry (default 'x'): ";
		while (std::getchar() != '\n');

	} while (std::getchar() == 'r');

	return 0;
}
