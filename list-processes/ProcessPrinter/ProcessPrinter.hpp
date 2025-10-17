#ifndef PROCESSPRINTER_HPP
#define PROCESSPRINTER_HPP

#include "ProcessScanner.hpp"
#include <vector>

namespace ProcessPrinter
{
void PrintTable(const std::vector<ProcessInfo>& processes);
void PrintSummary(const std::vector<ProcessInfo>& processes);
} // namespace ProcessPrinter

#endif // PROCESSPRINTER_HPP
