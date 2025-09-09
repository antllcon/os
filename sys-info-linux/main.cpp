#include <bits/local_lim.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sys/statvfs.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <vector>

constexpr std::string OS_RELEASE_FILE = "/etc/os-release";
constexpr std::string MEMINFO_FILE = "/proc/meminfo";
constexpr std::string MOUNTS_FILE = "/proc/mounts";
constexpr std::string PRETTY_NAME_KEY = "PRETTY_NAME=";
constexpr std::string MEM_TOTAL_KEY = "MemTotal:";
constexpr std::string MEM_FREE_KEY = "MemFree:";
constexpr std::string SWAP_TOTAL_KEY = "SwapTotal:";
constexpr std::string SWAP_FREE_KEY = "SwapFree:";
constexpr std::string VIRTUAL_MEMORY_KEY = "VmallocTotal:";
constexpr int KILOBYTE = 1024;
constexpr long NEGATIVE_RAM = -1;

struct MemoryStats
{
	long total = NEGATIVE_RAM;
	long free = NEGATIVE_RAM;
};

struct DriveInfo
{
	std::string mountPoint;
	std::string fsType;
	unsigned long long freeBytes;
	unsigned long long totalBytes;
};

std::string GetOperationSystemName()
{
	std::ifstream file(OS_RELEASE_FILE);
	if (!file.is_open())
	{
		throw std::invalid_argument("Failed to open " + OS_RELEASE_FILE);
	}

	std::string line;
	while (std::getline(file, line))
	{
		if (line.starts_with(PRETTY_NAME_KEY))
		{
			const size_t start = line.find('"') + 1;
			const size_t end = line.find('"', start);

			if (start != std::string::npos && end != std::string::npos)
			{
				return line.substr(start, end - start);
			}
		}
	}

	throw std::runtime_error("Name not found");
}

std::string GetKernelInfo()
{
	utsname unameData;
	if (uname(&unameData) != 0)
	{
		throw std::runtime_error("Failed to get kernel info");
	}
	return unameData.release;
}

std::string GetArchitecture()
{
	utsname unameData;
	if (uname(&unameData) != 0)
	{
		return "Unknown";
	}
	return unameData.machine;
}

std::string GetHostname()
{
	char hostname[HOST_NAME_MAX];
	if (gethostname(hostname, HOST_NAME_MAX) == 0)
	{
		return std::string(hostname);
	}
	return "Unknown";
}

std::string GetUsername()
{
	char* username = getlogin();
	if (username != nullptr)
	{
		return std::string(username);
	}
	return "Unknown";
}

MemoryStats GetMemoryStats(const std::string& totalKey, const std::string& freeKey)
{
	std::ifstream file(MEMINFO_FILE);
	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open " + MEMINFO_FILE);
	}

	MemoryStats memoryStats;
	std::string line;
	while (std::getline(file, line))
	{
		if (line.starts_with(totalKey))
		{
			memoryStats.total = std::stol(line.substr(line.find(":") + 1)) / KILOBYTE;
		}
		else if (line.starts_with(freeKey))
		{
			memoryStats.free = std::stol(line.substr(line.find(":") + 1)) / KILOBYTE;
		}
	}

	if (memoryStats.total == NEGATIVE_RAM || memoryStats.free == NEGATIVE_RAM)
	{
		throw std::runtime_error("Failed to find info in " + MEMINFO_FILE);
	}

	return memoryStats;
}

long GetCountProcessors()
{
	const long countProccesors = get_nprocs();
	if (countProccesors < 0)
	{
		throw std::runtime_error("Failed to get number of processors");
	}
	return countProccesors;
}

void PrintLoadAverage()
{
	double loadAverage[3];
	if (getloadavg(loadAverage, 3) != 3)
	{
		throw std::runtime_error("Failed to get load average");
	}

	std::cout << "Load average: " << std::fixed << std::setprecision(2)
			  << loadAverage[0] << ", "
			  << loadAverage[1] << ", "
			  << loadAverage[2] << std::endl;
}

void PrintDrivesInfo()
{
	std::ifstream mountsFile(MOUNTS_FILE.data());
	if (!mountsFile.is_open())
	{
		throw std::runtime_error("Failed to open " + std::string(MOUNTS_FILE));
	}

	std::vector<DriveInfo> drives;
	std::string line;

	while (std::getline(mountsFile, line))
	{
		std::stringstream ss(line);
		std::string device, mountPoint, fsType;
		ss >> device >> mountPoint >> fsType;

		struct statvfs stat;
		if (statvfs(mountPoint.c_str(), &stat) == 0)
		{
			unsigned long long totalBytes = stat.f_blocks * stat.f_bsize;
			unsigned long long freeBytes = stat.f_bavail * stat.f_bsize;

			drives.push_back({mountPoint, fsType, freeBytes, totalBytes});
		}
	}

	std::cout << "Drives:" << std::endl;
	for (const auto& drive : drives)
	{
		std::cout << "  " << std::left << std::setw(12) << drive.mountPoint
				  << std::setw(10) << drive.fsType
				  << std::setw(10) << drive.freeBytes << " free / "
				  << drive.totalBytes << " total" << std::endl;
	}
}

int main()
{
	try
	{
		std::cout << "OS: " << GetOperationSystemName() << std::endl;
		std::cout << "Kernel: " << GetKernelInfo() << std::endl;
		std::cout << "Architecture: " << GetArchitecture() << std::endl;
		std::cout << "Hostname: " << GetHostname() << std::endl;
		std::cout << "User: " << GetUsername() << std::endl;

		const MemoryStats ram = GetMemoryStats(MEM_TOTAL_KEY, MEM_FREE_KEY);
		std::cout << "RAM: " << ram.free << "MB free / " << ram.total << "MB total" << std::endl;

		const MemoryStats swap = GetMemoryStats(SWAP_TOTAL_KEY, SWAP_FREE_KEY);
		std::cout << "Swap: " << swap.free << "MB free / " << swap.total << "MB total" << std::endl;

		std::cout << "Processors: " << GetCountProcessors() << std::endl;
		PrintLoadAverage();
		PrintDrivesInfo();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}