#include <stdlib.h>

/*
std::string SysInfo::GetOSName()
{
	DWORD bufferSize = MAX_COMPUTERNAME_LENGTH + 1;
	std::vector<wchar_t> nameBuffer(bufferSize);

	if (GetComputerNameW(nameBuffer.data(), &bufferSize))
	{
		std::wstring name(nameBuffer.data(), bufferSize);
		return std::string(name.begin(), name.end());
	}

	return "";
}
*/

/*
std::string SysInfo::GetArchitecture()
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
*/
int main()
{

	return EXIT_SUCCESS;
}