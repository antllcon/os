#include "SysInfo.h"
#include <Windows.h>
#include <VersionHelpers.h>
#include <stdexcept>
#include <vector>

std::string SysInfo::GetOSVersion()
{
	if (IsWindows10OrGreater())
	{
		return "Windows 10 or Greater";
	}

	if (IsWindows8Point1OrGreater())
	{
		return "Windows 8.1";
	}

	if (IsWindows8OrGreater())
	{
		return "Windows 8";
	}

	if (IsWindows7SP1OrGreater())
	{
		return "Windows 7 SP1 or Greater";
	}

	if (IsWindows7OrGreater())
	{
		return "Windows 7";
	}

	if (IsWindowsVistaSP2OrGreater())
	{
		return "Windows Vista SP2 or Greater";
	}

	if (IsWindowsVistaSP1OrGreater())
	{
		return "Windows Vista SP1 or Greater";
	}

	if (IsWindowsVistaOrGreater())
	{
		return "Windows Vista";
	}

	if (IsWindowsXPSP3OrGreater())
	{
		return "Windows XP SP3 or Greater";
	}

	if (IsWindowsXPSP2OrGreater())
	{
		return "Windows XP SP2 or Greater";
	}

	if (IsWindowsXPSP1OrGreater())
	{
		return "Windows XP SP1 or Greater";
	}

	if (IsWindowsXPOrGreater())
	{
		return "Windows XP";
	}

	return "Windows older than XP";
}

std::string SysInfo::GetOSUsername()
{
	wchar_t buffer[256 + 1];
	DWORD size = std::size(buffer);
	if (GetUserNameW(buffer, &size))
	{
		std::wstring wstr(buffer);
		return std::string(wstr.begin(), wstr.end());
	}

	return "Unknown";
}

uint64_t SysInfo::GetTotalMemory()
{
	MEMORYSTATUSEX memInfo;
	memInfo.dwLength = sizeof(MEMORYSTATUSEX);
	if (GlobalMemoryStatusEx(&memInfo))
	{
		return memInfo.ullTotalPhys;
	}

	return 0;
}

uint64_t SysInfo::GetFreeMemory()
{
	MEMORYSTATUSEX memInfo;
	memInfo.dwLength = sizeof(MEMORYSTATUSEX);
	if (GlobalMemoryStatusEx(&memInfo))
	{
		return memInfo.ullAvailPhys;
	}

	return 0;
}

unsigned SysInfo::GetProcessorCount()
{
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);

	return sysInfo.dwNumberOfProcessors;
}