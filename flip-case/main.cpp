#define EXIT_INFO 2

#include "FileDesc/FileDesc.hpp"
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <span>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>
#include <vector>

constexpr size_t BUFFER_SIZE = 64 * 1024;

void PrintUsage(const char* progName)
{
	std::cerr << "Usage: " << progName << " <input1> <input2> ..." << std::endl;
	std::cerr << "For each input file creates <input>.out with flipped ASCII case" << std::endl;
	std::cerr << "Existing output files will be overwritten" << std::endl;
}

std::vector<std::string> ParseInputFiles(const int argc, char* argv[])
{
	if (argc < 2)
	{
		throw std::runtime_error("No input files provided");
	}

	std::vector<std::string> inputFiles;
	for (int i = 1; i < argc; ++i)
	{
		inputFiles.emplace_back(argv[i]);
	}

	return inputFiles;
}

void FlipCaseBuffer(std::span<char> buffer)
{
	for (auto& ch : buffer)
	{
		const auto uch = ch;
		if (uch >= 'a' && uch <= 'z')
		{
			ch = static_cast<char>(uch - 'a' + 'A');
		}
		else if (uch >= 'A' && uch <= 'Z')
		{
			ch = static_cast<char>(uch - 'A' + 'a');
		}
	}
}

int ProcessFile(const std::string& inputFile)
{
	const auto pid = getpid();
	std::cout << "Process " << pid << " is processing " << inputFile << std::endl;

	try
	{
		FileDesc inputFd;
		inputFd.Open(inputFile.c_str(), O_RDONLY);

		const auto outputFile = inputFile + ".out";

		FileDesc outputFd;
		outputFd.Open(outputFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC | 0644);

		std::vector<char> buffer(BUFFER_SIZE);

		while (true)
		{
			const size_t bytesRead = inputFd.Read(buffer.data(), buffer.size());
			if (bytesRead == 0)
			{
				break;
			}

			FlipCaseBuffer({buffer.data(), bytesRead});
			outputFd.Write(buffer.data(), bytesRead);
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}

	std::cout << "Process " << pid << " has finished successfully" << std::endl;
	return EXIT_SUCCESS;
}

pid_t SpawnChild(const std::string& file)
{
	const pid_t pid = fork();

	if (pid < 0)
	{
		std::cerr << "fork() failed: " << strerror(errno) << std::endl;
		return EXIT_FAILURE;
	}

	if (pid == 0)
	{
		const int processResult = ProcessFile(file);
		_exit(processResult);
	}

	return pid;
}

int WaitChildren(const size_t childCount)
{
	int exitCode = EXIT_SUCCESS;
	size_t finishedProccesses = 0;

	while (finishedProccesses < childCount)
	{
		int status = 0;
		const pid_t terminatedPid = waitpid(-1, &status, 0);

		if (terminatedPid < 0)
		{
			if (errno == EINTR)
			{
				continue;
			}

			if (errno == ECHILD)
			{
				break;
			}

			std::cerr << "waitpid() error: " << strerror(errno) << std::endl;
			exitCode = EXIT_FAILURE;
			break;
		}

		std::cout << "Child process " << terminatedPid << " is over" << std::endl;

		if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
		{
			exitCode = EXIT_FAILURE;
		}

		finishedProccesses++;
	}

	return exitCode;
}

int main(const int argc, char* argv[])
{
	std::vector<std::string> inputFiles;

	try
	{
		inputFiles = ParseInputFiles(argc, argv);
	}
	catch (std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		PrintUsage(argv[0]);
		return EXIT_INFO;
	}

	const auto maxParallel = std::thread::hardware_concurrency();
	std::cout << maxParallel << std::endl;

	std::vector<pid_t> childProcesses;
	for (const auto& file : inputFiles)
	{
		while (childProcesses.size() >= maxParallel)
		{
			int status = 0;
			pid_t terminaredPid = waitpid(-1, &status, 0);
			if (terminaredPid > 0)
			{
				std::cout << "Child process " << terminaredPid << " is over" << std::endl;
				std::erase(childProcesses, terminaredPid);
			}
		}

		pid_t pid = SpawnChild(file);
		if (pid > 0)
		{
			childProcesses.emplace_back(pid);
		}
		else
		{
			return EXIT_FAILURE;
		}
	}

	return WaitChildren(childProcesses.size());
}
