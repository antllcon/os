#include "SysInfo.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <pwd.h>

namespace
{
const std::string OS_RELEASE_FILE = "/etc/os-release";
const std::string MEMINFO_FILE = "/proc/meminfo";
const std::string PRETTY_NAME_KEY = "PRETTY_NAME=";
const std::string MEM_TOTAL_KEY = "MemTotal:";
const std::string MEM_FREE_KEY = "MemFree:";
constexpr long NEGATIVE_RAM = -1;
constexpr int KILOBYTE = 1024;

void AssertIsFileExist(const std::ifstream& file)
{
	if (!file.is_open())
	{
		throw std::runtime_error("File does not exist");
	}
}

void AssertIsPositiveValue(const uint64_t value)
{
	if (value == NEGATIVE_RAM)
	{
		throw std::runtime_error("Value mast be positive");
	}
}
} // namespace

std::string SysInfo::GetOSVersion()
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

std::string SysInfo::GetOSUsername()
{
	passwd *pw = getpwuid(getuid());
	if (pw != nullptr) {
		return std::string(pw->pw_name);
	}

	return "Unknown";
}

uint64_t SysInfo::GetFreeMemory()
{
	std::ifstream file(MEMINFO_FILE);
	AssertIsFileExist(file);

	uint64_t value = NEGATIVE_RAM;
	std::string line;

	while (std::getline(file, line))
	{
		if (line.starts_with(MEM_FREE_KEY))
		{
			value = std::stol(line.substr(line.find(":") + 1)) / KILOBYTE;
			break;
		}
	}

	AssertIsPositiveValue(value);
	return value;
}

uint64_t SysInfo::GetTotalMemory()
{
	std::ifstream file(MEMINFO_FILE);
	AssertIsFileExist(file);

	uint64_t value = NEGATIVE_RAM;
	std::string line;

	while (std::getline(file, line))
	{
		if (line.starts_with(MEM_TOTAL_KEY))
		{
			value = std::stol(line.substr(line.find(":") + 1)) / KILOBYTE;
			break;
		}
	}

	AssertIsPositiveValue(value);
	return value;
}

unsigned SysInfo::GetProcessorCount()
{
	const int countProccesors = get_nprocs();
	if (countProccesors <= 0)
	{
		throw std::runtime_error("Failed to get number of processors");
	}

	return countProccesors;
}