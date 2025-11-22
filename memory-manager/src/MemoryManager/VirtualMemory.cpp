#include "VirtualMemory.h"

#include "PageTableEntry.h"

VirtualMemory::VirtualMemory(PhysicalMemory& physicalMemory, IOSHandler& handler)
	: m_physicalMemory(physicalMemory)
	, m_handler(handler)
	, m_pageDirectoryBase(0)
{
}

void VirtualMemory::SetPageTableAddress(uint32_t physicalAddress)
{
	if (physicalAddress % 4096 != 0)
	{
		throw MemoryException("Page Directory Base must be 4KB aligned");
	}
	m_pageDirectoryBase = physicalAddress;
}

[[nodiscard]] uint32_t VirtualMemory::GetPageTableAddress() const noexcept
{
	return m_pageDirectoryBase;
}

[[nodiscard]] uint8_t VirtualMemory::Read8(uint32_t address, Privilege privilege, bool execute) const
{
	return AccessMemory<uint8_t>(address, privilege, execute ? Access::Execute : Access::Read);
}

[[nodiscard]] uint16_t VirtualMemory::Read16(uint32_t address, Privilege privilege, bool execute) const
{
	return AccessMemory<uint16_t>(address, privilege, execute ? Access::Execute : Access::Read);
}

[[nodiscard]] uint32_t VirtualMemory::Read32(uint32_t address, Privilege privilege, bool execute) const
{
	return AccessMemory<uint32_t>(address, privilege, execute ? Access::Execute : Access::Read);
}

[[nodiscard]] uint64_t VirtualMemory::Read64(uint32_t address, Privilege privilege, bool execute) const
{
	return AccessMemory<uint64_t>(address, privilege, execute ? Access::Execute : Access::Read);
}

void VirtualMemory::Write8(uint32_t address, uint8_t value, Privilege privilege)
{
	AccessMemory<uint8_t>(address, privilege, Access::Write, value);
}

void VirtualMemory::Write16(uint32_t address, uint16_t value, Privilege privilege)
{
	AccessMemory<uint16_t>(address, privilege, Access::Write, value);
}

void VirtualMemory::Write32(uint32_t address, uint32_t value, Privilege privilege)
{
	AccessMemory<uint32_t>(address, privilege, Access::Write, value);
}

void VirtualMemory::Write64(uint32_t address, uint64_t value, Privilege privilege)
{
	AccessMemory<uint64_t>(address, privilege, Access::Write, value);
}

[[nodiscard]] uint32_t VirtualMemory::TranslateAddress(uint32_t address, Privilege privilege, Access accessType) const
{
	const uint32_t directoryIndex = (address >> 22) & 0x3FF;
	const uint32_t tableIndex = (address >> 12) & 0x3FF;
	const uint32_t offset = address & 0xFFF;

	uint32_t pageDirectory = m_pageDirectoryBase + (directoryIndex * sizeof(uint32_t));

	PTE pde;
	try
	{
		pde.raw = m_physicalMemory.Read32(pageDirectory);
	}
	catch (...)
	{
		throw PageFaultException{PageFaultReason::PhysicalAccessOutOfRange};
	}

	CheckProtections(pde, privilege, accessType);

	if (!pde.IsAccessed())
	{
		pde.SetAccessed(true);
		m_physicalMemory.Write32(pageDirectory, pde.raw);
	}

	uint32_t pageTableBase = pde.GetFrame() * 4096;
	uint32_t pageTableAddress = pageTableBase + (tableIndex * sizeof(uint32_t));

	PTE pte;
	try
	{
		pte.raw = m_physicalMemory.Read32(pageTableAddress);
	}
	catch (...)
	{
		throw PageFaultException{PageFaultReason::PhysicalAccessOutOfRange};
	}

	CheckProtections(pte, privilege, accessType);

	bool needUpdate = false;
	if (!pte.IsAccessed())
	{
		pte.SetAccessed(true);
		needUpdate = true;
	}
	if (accessType == Access::Write && !pte.IsDirty())
	{
		pte.SetDirty(true);
		needUpdate = true;
	}

	if (needUpdate)
	{
		m_physicalMemory.Write32(pageTableAddress, pte.raw);
	}

	return (pte.GetFrame() * 4096) + offset;
}

void VirtualMemory::CheckProtections(const PTE& entry, Privilege privilege, Access accessType)
{
	if (!entry.IsPresent())
	{
		throw PageFaultException{PageFaultReason::NotPresent};
	}

	if (privilege == Privilege::User && !entry.IsUser())
	{
		throw PageFaultException{PageFaultReason::UserAccessToSupervisor};
	}

	if (accessType == Access::Write)
	{
		if (!entry.IsWritable())
		{
			throw PageFaultException{PageFaultReason::WriteToReadOnly};
		}
	}

	if (accessType == Access::Execute)
	{
		if (entry.IsNX())
		{
			throw PageFaultException{PageFaultReason::ExecOnNX};
		}
	}
}

bool VirtualMemory::HandleFault(uint32_t vpn, Access access, PageFaultReason reason) const
{
	return m_handler.OnPageFault(const_cast<VirtualMemory&>(*this), vpn, access, reason);
}