#include <cstdlib>
#include <iostream>
#include <sys/wait.h>
#include <unistd.h>

int main()
{
	std::cout << "Parent process: " << getpid() << std::endl;

	std::cout << std::flush;
	pid_t pid = fork();

	if (pid < 0)
	{
		std::cerr << "Fork failed" << std::endl;
		return EXIT_FAILURE;
	}

	if (pid == 0)
	{
		std::cout << "Child process: " << getpid() << std::endl;
		_exit(127);
	}
	if (pid > 0)
	{
		std::cout << "Parent process: " << getpid() << std::endl;
		// std::cout << "Child process - PID: " << pid << std::endl;

		int status;
		pid_t userPid;

		while (true)
		{
			std::cout << "Write kid PID: ";
			std::cin >> userPid;

			const pid_t result = waitpid(userPid, &status, WNOHANG);

			if (result == -1)
			{
				std::cerr << "Invalid PID, waitpid() return error" << std::endl;
			}
			else if (result == 0)
			{
				std::cerr << "Process with PID " << userPid << " dont stop" << std::endl;
			}
			else
			{
				std::cout << "Success take kid PID" << std::endl;
				break;
			}
		}
	}

	return EXIT_SUCCESS;
}