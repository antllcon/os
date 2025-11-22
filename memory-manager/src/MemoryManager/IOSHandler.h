#pragma once
#include "Common.h"

#include <cstdint>

class VirtualMemory;

class IOSHandler
{
public:
	virtual ~IOSHandler() = default;
	virtual bool OnPageFault(VirtualMemory& virtualMemory, uint32_t virtualPageNumber, Access access, PageFaultReason reason) = 0;
};
