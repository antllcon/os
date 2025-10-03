#include "ProcessScanner.hpp"
#include <vector>

namespace
{
// тут пиши все const и constexpr
//
// Тут пиши AssertIs... - все проверки с исключениями
}

std::vector<ProcessInfo> ScanProcesses()
{
	std::vector<ProcessInfo> processes;
	const fs::path procPath("/proc");

	if (!fs::exists(procPath) || !fs::is_directory(procPath))
	{
		std::cerr << "Error: /proc directory not found or not a directory" << std::endl;
		return processes;
	}

	for (const auto& entry : fs::directory_iterator(procPath))
	{
		if (entry.is_directory())
		{
			std::string dirName = entry.path().filename().string();
			if (std::all_of(dirName.begin(), dirName.end(), ::isdigit))
			{
				uint32_t pid = std::stoul(dirName);
				ProcessInfo info;
				info.pid = pid;

				info.processName = GetProcessName(pid);
				info.userName = GetUserName(pid);
				getMemoryInfo(pid, info.privateBytes, info.sharedMemory);
				info.threadCount = GetThreadCount(pid);
				info.commandLine = GetCommandLine(pid);

				processes.emplace_back(std::move(info));
			}
		}
	}

	return processes;
}
