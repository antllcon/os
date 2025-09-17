#include <iostream>
#include "SysInfo/SysInfo.h"

int main()
{
	try
	{
		std::cout << "OS: " << SysInfo::GetOSVersion() << std::endl;
		std::cout << "User: " << SysInfo::GetOSUsername() << std::endl;
		std::cout << "RAM: " << SysInfo::GetFreeMemory() << " MB / " << SysInfo::GetTotalMemory() << " MB" << std::endl;
		std::cout << "Processor count: " << SysInfo::GetProcessorCount() << std::endl;
	}
	catch (std::exception& e)
	{
		std::cerr << "Error: " <<  e.what() << std::endl;
	}

	return EXIT_SUCCESS;
}