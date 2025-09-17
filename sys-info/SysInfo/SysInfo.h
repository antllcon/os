#ifndef SYSINFO_H
#define SYSINFO_H

#include <cstdint>
#include <string>

class SysInfo
{
public:
	SysInfo() = default;
	~SysInfo() = default;

	static std::string GetOSVersion();
	static std::string GetOSUsername();
	static uint64_t GetFreeMemory();
	static uint64_t GetTotalMemory();
	static unsigned GetProcessorCount();
};

#endif // SYSINFO_H