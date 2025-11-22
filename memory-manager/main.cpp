#include "StepanOS.h"
#include "VirtualMemory.h"
#include "src/ScopeTimer/ScopeTimer.h"

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

int main()
{
	std::cout << "Directory: " << std::filesystem::current_path() << std::endl;

	try
	{
		PhysicalMemoryConfig config;
		PhysicalMemory ram(config);
		StepanOS os(ram);

		VirtualMemory virtualMemory(ram, os);
		virtualMemory.SetPageTableAddress(0);

		os.MapPage(0x00400000, true, true);
		virtualMemory.Write32(0x00400000, 0xAAAAAAA, Privilege::User);
		auto info = virtualMemory.Read32(0x00400000, Privilege::Supervisor);
		std::cout << "In phys. mem: " << std::hex << info << std::endl;

	}
	catch (const MemoryException& exception)
	{
		std::cerr << "Error: " << exception.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}