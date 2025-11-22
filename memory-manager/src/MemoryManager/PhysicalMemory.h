#pragma once
#include <cstdint>
#include <cstring>
#include <vector>
#include "Common.h"

class PhysicalMemory
{
public:
	explicit PhysicalMemory(PhysicalMemoryConfig config);

	[[nodiscard]] uint32_t GetSize() const noexcept;

	[[nodiscard]] uint8_t Read8(uint32_t address) const;
	[[nodiscard]] uint16_t Read16(uint32_t address) const;
	[[nodiscard]] uint32_t Read32(uint32_t address) const;
	[[nodiscard]] uint64_t Read64(uint32_t address) const;

	void Write8(uint32_t address, uint8_t value);
	void Write16(uint32_t address, uint16_t value);
	void Write32(uint32_t address, uint32_t value);
	void Write64(uint32_t address, uint64_t value);

private:
	template <typename T>
	[[nodiscard]] T ReadImpl(uint32_t address) const
	{
		ValidateAccess<T>(address);
		T value;
		std::memcpy(&value, &m_memory[address], sizeof(T));
		return value;
	}

	template <typename T>
	void WriteImpl(uint32_t address, T value)
	{
		ValidateAccess<T>(address);
		std::memcpy(&m_memory[address], &value, sizeof(T));
	}

	template <typename T>
	void ValidateAccess(uint32_t address) const
	{
		if (address % sizeof(T) != 0)
		{
			throw MemoryException("Misaligned physical memory access");
		}
		if (address + sizeof(T) > m_memory.size())
		{
			throw MemoryException("Physical memory access out of range");
		}
	}

	PhysicalMemoryConfig m_config;
	std::vector<uint8_t> m_memory;
};
