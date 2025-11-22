#pragma once
#include "IOSHandler.h"
#include "PageTableEntry.h"
#include "PhysicalMemory.h"

#include <iostream>

class StepanOS final : public IOSHandler
{
public:
	explicit StepanOS(PhysicalMemory& physicalMemory)
		: m_physicalMemory(physicalMemory)
		, m_nextFreeFrame(1)
	{
	}

	StepanOS(const StepanOS&) = delete;
	StepanOS& operator=(const StepanOS&) = delete;
	StepanOS(StepanOS&&) = delete;
	StepanOS& operator=(StepanOS&&) = delete;

	bool OnPageFault(VirtualMemory& virtualMemory, uint32_t virtualPageNumber, Access access, PageFaultReason reason) override
	{
		if (reason == PageFaultReason::NotPresent)
		{
			// Алгоритм загрузки фрейма
			return true;
		}

		return false;
	}

	void MapPage(uint32_t virtualAddress, bool writable, bool user)
	{
		uint32_t directoryIndex = (virtualAddress >> 22) & 0x3FF;
		uint32_t tableIndex = (virtualAddress >> 12) & 0x3FF;
		uint32_t pdeAddress = (directoryIndex * 4);
		PTE pde;

		pde.raw = m_physicalMemory.Read32(pdeAddress);

		if (!pde.IsPresent())
		{
			uint32_t ptFrame = AllocateFrame();
			pde.SetFrame(ptFrame);
			pde.SetPresent(true);
			pde.SetWritable(true);
			pde.SetUser(true);
			m_physicalMemory.Write32(pdeAddress, pde.raw);
		}

		uint32_t pteAddress = pde.GetFrame() * 4096 + tableIndex * 4;
		PTE pte;
		pte.raw = m_physicalMemory.Read32(pteAddress);

		if (!pte.IsPresent())
		{
			uint32_t dataFrame = AllocateFrame();
			pte.SetFrame(dataFrame);
			pte.SetPresent(true);
			pte.SetWritable(writable);
			pte.SetUser(user);
			m_physicalMemory.Write32(pteAddress, pte.raw);
		}
	}

private:
	uint32_t AllocateFrame()
	{
		if (m_nextFreeFrame >= m_physicalMemory.GetSize() / 4096)
		{
			throw std::runtime_error("Out of physical memory");
		}

		return m_nextFreeFrame++;
	}

	PhysicalMemory& m_physicalMemory;
	uint32_t m_nextFreeFrame;
};