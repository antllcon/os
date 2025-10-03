#include <iomanip>
#include <ios>
#include <iostream>
#include <vector>
#include <windows.h>

#include "ProcessPrinter.hpp"
#include "ProcessScanner.hpp"
#include "ScopedHandle.hpp"

int main()
{
	try
	{
		ProcessScanner scanner;
		auto processes = scanner.GetProcesses();
		ProcessScanner::SortByMemory(processes);

		ProcessPrinter::PrintTable(processes);
		ProcessPrinter::PrintSummary(processes);
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
