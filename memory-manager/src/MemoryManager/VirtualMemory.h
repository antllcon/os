#pragma once

#include "Common.h"
#include "IOSHandler.h"
#include "PageTableEntry.h"
#include "PhysicalMemory.h"
#include <cstdint>


struct PageFaultException
{
	PageFaultReason reason;
};

class VirtualMemory
{
public:
	VirtualMemory(PhysicalMemory& physicalMemory, IOSHandler& handler);

	void SetPageTableAddress(uint32_t physicalAddress);
	[[nodiscard]] uint32_t GetPageTableAddress() const noexcept;

	uint8_t Read8(uint32_t address, Privilege privilege, bool execute = false) const;
	uint16_t Read16(uint32_t address, Privilege privilege, bool execute = false) const;
	uint32_t Read32(uint32_t address, Privilege privilege, bool execute = false) const;
	uint64_t Read64(uint32_t address, Privilege privilege, bool execute = false) const;

	void Write8(uint32_t address, uint8_t value, Privilege privilege);
	void Write16(uint32_t address, uint16_t value, Privilege privilege);
	void Write32(uint32_t address, uint32_t value, Privilege privilege);
	void Write64(uint32_t address, uint64_t value, Privilege privilege);

private:
	template <typename T>
	T AccessMemory(uint32_t address, Privilege privilege, Access accessType, T value = 0) const
	{
		if (address % sizeof(T) != 0)
		{
			if (!HandleFault(address >> 12, accessType, PageFaultReason::MisalignedAccess))
			{
				throw MemoryException("Misaligned virtual memory access");
			}
			return AccessMemory<T>(address, privilege, accessType, value);
		}

		// Ограничить количество обращений к памяти
		while (true)
		{
			// Не использовать исключения для передачи управления
			try
			{
				uint32_t physicalAddress = TranslateAddress(address, privilege, accessType);

				if (accessType == Access::Write)
				{
					m_physicalMemory.Write32(physicalAddress & ~0x3, 0);

					// Сделать так, чтобы проверка выполнялась один раз
					switch (sizeof(T))
					{
					case 1:
						m_physicalMemory.Write8(physicalAddress, static_cast<uint8_t>(value));
						break;
					case 2:
						m_physicalMemory.Write16(physicalAddress, static_cast<uint16_t>(value));
						break;
					case 4:
						m_physicalMemory.Write32(physicalAddress, static_cast<uint32_t>(value));
						break;
					case 8:
						m_physicalMemory.Write64(physicalAddress, static_cast<uint64_t>(value));
						break;
					}

					return T{};
				}

				// Добавить касты
				switch (sizeof(T))
				{
				case 1:
					return static_cast<T>(m_physicalMemory.Read8(physicalAddress));
				case 2:
					return static_cast<T>(m_physicalMemory.Read16(physicalAddress));
				case 4:
					return static_cast<T>(m_physicalMemory.Read32(physicalAddress));
				case 8:
					return static_cast<T>(m_physicalMemory.Read64(physicalAddress));
				}
			}
			catch (const PageFaultException& ex)
			{
				if (!HandleFault(address >> 12, accessType, ex.reason))
				{
					throw MemoryException("Page Fault not handled");
				}
			}
		}
	}

	[[nodiscard]] uint32_t TranslateAddress(uint32_t address, Privilege privilege, Access accessType) const;
	static void CheckProtections(PTE entry, Privilege privilege, Access accessType);
	bool HandleFault(uint32_t vpn, Access access, PageFaultReason reason) const;

	PhysicalMemory& m_physicalMemory;
	IOSHandler& m_handler;
	uint32_t m_pageDirectoryBase;
};