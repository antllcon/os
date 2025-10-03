#include "ProcessScanner.hpp"
#include <algorithm>

namespace
{
constexpr DWORD MAX_NAME_LENGTH = 256;
constexpr DWORD MAX_DOMAIN_LENGTH = 256;

void AssertIsValidHandleValue(HANDLE handle)
{
	if (handle == INVALID_HANDLE_VALUE)
	{
		throw std::runtime_error("Failed to create process snapshot");
	}
}

void AssertProcessEnumSuccess(BOOL result, const char* operation)
{
	if (!result)
	{
		throw std::runtime_error(std::string("Failed to ") + operation + " process from snapshot");
	}
}
} // namespace

std::vector<ProcessInfo> ProcessScanner::GetProcesses()
{
	return ScanProcesses();
}

void ProcessScanner::SortByMemory(std::vector<ProcessInfo>& processes)
{
	std::sort(processes.begin(), processes.end(), [](const ProcessInfo& a, const ProcessInfo& b) {
		SIZE_T aTotal = a.privateBytes + a.sharedMemory;
		SIZE_T bTotal = b.privateBytes + b.sharedMemory;
		return aTotal > bTotal;
	});
}

std::vector<ProcessInfo> ProcessScanner::ScanProcesses()
{
	std::vector<ProcessInfo> processes;

	ScopedHandle snapshot(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0));
	AssertIsValidHandleValue(snapshot);

	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32);

	AssertProcessEnumSuccess(Process32First(snapshot, &pe32), "get first");

	do
	{
		ProcessInfo info;
		info.pid = pe32.th32ProcessID;
		info.processName = pe32.szExeFile;
		info.threadCount = pe32.cntThreads;
		info.userName = GetUserName(info.pid);
		GetMemoryInfo(info.pid, info.privateBytes, info.sharedMemory);
		info.commandLine = GetCommandLine(info.pid);

		processes.emplace_back(std::move(info));

	} while (Process32Next(snapshot, &pe32));

	return processes;
}

std::string ProcessScanner::GetUserName(uint32_t pid)
{
	ScopedHandle processHandle(OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid));
	if (!processHandle)
	{
		return "(no rights)";
	}

	HANDLE processToken;
	if (!OpenProcessToken(processHandle, TOKEN_QUERY, &processToken))
	{
		return "(no rights)";
	}
	ScopedHandle tokenHandle(processToken);

	DWORD tokenInfoLength = 0;
	GetTokenInformation(tokenHandle, TokenUser, nullptr, 0, &tokenInfoLength);
	if (tokenInfoLength == 0)
	{
		return "(zero length)";
	}

	std::vector<char> buffer(tokenInfoLength);
	PTOKEN_USER pTokenUser = reinterpret_cast<PTOKEN_USER>(buffer.data());

	if (!GetTokenInformation(tokenHandle, TokenUser, pTokenUser, tokenInfoLength, &tokenInfoLength))
	{
		return "(unknown)";
	}

	char domainName[MAX_DOMAIN_LENGTH];
	char userName[MAX_NAME_LENGTH];
	DWORD domainSize = sizeof(domainName);
	DWORD userSize = sizeof(userName);
	SID_NAME_USE sidNameUse;

	if (!LookupAccountSidA(nullptr, pTokenUser->User.Sid, userName, &userSize, domainName, &domainSize, &sidNameUse))
	{
		return "(unknown)";
	}

	return userName;
}

bool ProcessScanner::GetMemoryInfo(uint32_t pid, uint64_t& privateBytes, uint64_t& sharedMemory)
{
	ScopedHandle processHandle(OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid));
	if (!processHandle)
	{
		return false;
	}

	PROCESS_MEMORY_COUNTERS_EX pmc;
	if (GetProcessMemoryInfo(processHandle, reinterpret_cast<PPROCESS_MEMORY_COUNTERS>(&pmc), sizeof(pmc)))
	{
		privateBytes = pmc.PrivateUsage;
		SIZE_T ws = pmc.WorkingSetSize;

		if (ws >= privateBytes)
		{
			sharedMemory = ws - privateBytes;
		}
		else
		{
			sharedMemory = 0;
		}

		return true;
	}

	return false;
}

bool ProcessScanner::GetCpuUsage(uint32_t pid, double& usage)
{
	// TODO: доделать
	return false;
}

std::string ProcessScanner::GetCommandLine(uint32_t pid)
{
	ScopedHandle processHandle(OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid));
	if (!processHandle)
	{
		return "(no rights)";
	}

	char buffer[MAX_PATH];
	DWORD size = MAX_PATH;
	if (QueryFullProcessImageNameA(processHandle, 0, buffer, &size))
	{
		return std::string(buffer);
	}

	return "(unknown)";
}
