#include "VirtualMemory.h"

#include "PageTableEntry.h"

namespace
{
const uint32_t BITS_PER_PAGE = 12;
const uint32_t PAGE_SIZE = 0x1000;

const uint32_t DIRECTORY_INDEX_SHIFT = 22;
const uint32_t TABLE_INDEX_SHIFT = 12;

const uint32_t DIRECTORY_INDEX_MASK = 0x3FF;
const uint32_t TABLE_INDEX_MASK = 0x3FF;
const uint32_t OFFSET_MASK = 0xFFF;

const uint32_t PAGE_4K = 4096;
}

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

// Разделить на отдельные блоки, метод большой!
[[nodiscard]] uint32_t VirtualMemory::TranslateAddress(uint32_t address, Privilege privilege, Access accessType) const
{
	const uint32_t directoryIndex = (address >> DIRECTORY_INDEX_SHIFT) & DIRECTORY_INDEX_MASK;
	const uint32_t tableIndex = (address >> TABLE_INDEX_SHIFT) & TABLE_INDEX_MASK;
	const uint32_t offset = address & OFFSET_MASK;

	uint32_t pageDirectory = m_pageDirectoryBase + (directoryIndex * sizeof(uint32_t));

	PTE pde;
	try
	{
		// Можно обойтись проверкой, не нужно использовать try catch()
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

	uint32_t pageTableBase = pde.GetFrame() * PAGE_4K;
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

	bool needUpdatePte = false;
	if (!pte.IsAccessed())
	{
		pte.SetAccessed(true);
		needUpdatePte = true;
	}
	if (accessType == Access::Write && !pte.IsDirty())
	{
		pte.SetDirty(true);
		needUpdatePte = true;
	}

	if (needUpdatePte)
	{
		m_physicalMemory.Write32(pageTableAddress, pte.raw);
	}

	return (pte.GetFrame() * PAGE_4K) + offset;
}

// Убрать исключения
void VirtualMemory::CheckProtections(PTE entry, Privilege privilege, Access accessType)
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