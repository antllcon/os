#include <Windows.h>
#include <VersionHelpers.h>
#include <cstdlib>
#include <exception>
#include <iomanip>
#include <iostream>
#include <psapi.h>
#include <stdexcept>
#include <vector>
#include <lmcons.h>

namespace Constants
{
constexpr size_t MEGABYTE = 1024 * 1024;
constexpr size_t GIGABYTE = 1024 * 1024 * 1024;
constexpr size_t PC_NAME_BUFFER = MAX_COMPUTERNAME_LENGTH + 1;
constexpr size_t OS_USERNAME_BUFFER = UNLEN + 1;

} // namespace Constants

inline static MEMORYSTATUSEX GetMemoryInfo()
{
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof(statex);

	if (!GlobalMemoryStatusEx(&statex))
	{
		throw std::runtime_error("Failed to get memory status");
	}

	return statex;
}

inline static uint64_t GetTotalMemory(const MEMORYSTATUSEX& memInfo)
{
	return memInfo.ullTotalPhys / Constants::MEGABYTE;
}

inline static uint64_t GetFreeMemory(const MEMORYSTATUSEX& memInfo)
{
	return memInfo.ullAvailPhys / Constants::MEGABYTE;
}

inline static uint64_t GetVirtualMemory(const MEMORYSTATUSEX& memInfo)
{
	return memInfo.ullAvailPageFile / Constants::MEGABYTE;
}

inline static DWORD GetMemoryLoad(const MEMORYSTATUSEX& memInfo)
{
	return memInfo.dwMemoryLoad;
}

inline static uint64_t GetPagefileTotal(const MEMORYSTATUSEX& memInfo)
{
	return memInfo.ullTotalPageFile / Constants::MEGABYTE;
}

inline static uint64_t GetPagefileFree(const MEMORYSTATUSEX& memInfo)
{
	return memInfo.ullAvailPageFile / Constants::MEGABYTE;
}

std::string GetOperationSystemName()
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

std::string GetPCName()
{
	DWORD bufferSize = Constants::PC_NAME_BUFFER;
	wchar_t nameBuffer[Constants::PC_NAME_BUFFER];

	if (GetComputerNameW(nameBuffer, &bufferSize))
	{
		std::wstring name(nameBuffer, bufferSize);
		return std::string(name.begin(), name.end());
	}

	return "";
}

std::string GetUsername()
{
	wchar_t buffer[Constants::OS_USERNAME_BUFFER];
	DWORD size = std::size(buffer);

	if (GetUserNameW(buffer, &size))
	{
		std::wstring wstr(buffer);
		return std::string(wstr.begin(), wstr.end());
	}

	return "Unknown";
}

std::string GetArchitecture()
{
	SYSTEM_INFO sysInfo;
	GetNativeSystemInfo(&sysInfo);

	switch (sysInfo.wProcessorArchitecture)
	{
	case PROCESSOR_ARCHITECTURE_AMD64:
		return "x64 (AMD64)";
	case PROCESSOR_ARCHITECTURE_INTEL:
		return "x86 (Intel)";
	case PROCESSOR_ARCHITECTURE_ARM:
		return "ARM";
	case PROCESSOR_ARCHITECTURE_ARM64:
		return "ARM64";
	default:
		return "Unknown";
	}
}

unsigned GetProcessorCount()
{
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);

	return sysInfo.dwNumberOfProcessors;
}

void PrintLogicalDrives()
{
	DWORD bufferLength = GetLogicalDriveStringsW(0, nullptr);
	if (bufferLength == 0)
	{
		return;
	}

	std::vector<wchar_t> driveStrings(bufferLength);
	if (GetLogicalDriveStringsW(bufferLength, driveStrings.data()) == 0)
	{
		return;
	}

	wchar_t* currentDrive = driveStrings.data();
	while (*currentDrive)
	{
		ULARGE_INTEGER freeMemory;
		ULARGE_INTEGER totalMemory;
		wchar_t fsNameBuffer[MAX_PATH] = {0};

		if (GetDiskFreeSpaceExW(currentDrive, &freeMemory, &totalMemory, nullptr) &&
			GetVolumeInformationW(currentDrive, nullptr, 0, nullptr, nullptr, nullptr, fsNameBuffer, MAX_PATH))
		{
			uint64_t freeGB = freeMemory.QuadPart / Constants::GIGABYTE;
			uint64_t totalGB = totalMemory.QuadPart / Constants::GIGABYTE;

			std::wcout << L"  - " << currentDrive << L" (" << fsNameBuffer << L"): "
					   << freeGB << L" GB free / " << totalGB << L" GB total" << std::endl;
		}
		else
		{
			std::wcout << L"  - " << currentDrive << L" (No media or info)" << std::endl;
		}

		// Выяснить для
		currentDrive += wcslen(currentDrive) + 1;
	}
}

int main()
{
	try
	{
		const auto memoryInfo = GetMemoryInfo();

		std::cout << "OS: " << GetOperationSystemName() << std::endl;
		std::cout << "Computer Name: " << GetPCName() << std::endl;
		std::cout << "User: " << GetUsername() << std::endl;
		std::cout << "Architecture: " << GetArchitecture() << std::endl;
		std::cout << "RAM: " << GetFreeMemory(memoryInfo) << " MB / " << GetTotalMemory(memoryInfo) << " MB" << std::endl;
		std::cout << "Virtual memory: " << GetVirtualMemory(memoryInfo) << " MB" << std::endl;
		std::cout << "Memory load: " << GetMemoryLoad(memoryInfo) << " %" << std::endl;
		std::cout << "Pagefile: " << GetPagefileFree(memoryInfo) << " MB / " << GetPagefileTotal(memoryInfo) << " MB" << std::endl;
		std::cout << "Processors: " << GetProcessorCount() << std::endl;
		std::cout << "Drivers: " << std::endl;

		PrintLogicalDrives();
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}