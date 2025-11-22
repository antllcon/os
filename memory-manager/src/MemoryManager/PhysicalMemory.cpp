#include "PhysicalMemory.h"

PhysicalMemory::PhysicalMemory(PhysicalMemoryConfig config)
	: m_config(config)
	, m_memory(static_cast<size_t>(config.numFrames) * config.frameSize, 0)
{
}

[[nodiscard]] uint32_t PhysicalMemory::GetSize() const noexcept
{
	return static_cast<uint32_t>(m_memory.size());
}

[[nodiscard]] uint8_t PhysicalMemory::Read8(uint32_t address) const
{
	return ReadImpl<uint8_t>(address);
}

[[nodiscard]] uint16_t PhysicalMemory::Read16(uint32_t address) const
{
	return ReadImpl<uint16_t>(address);
}

[[nodiscard]] uint32_t PhysicalMemory::Read32(uint32_t address) const
{
	return ReadImpl<uint32_t>(address);
}

[[nodiscard]] uint64_t PhysicalMemory::Read64(uint32_t address) const
{
	return ReadImpl<uint64_t>(address);
}

void PhysicalMemory::Write8(uint32_t address, uint8_t value)
{
	WriteImpl<uint8_t>(address, value);
}

void PhysicalMemory::Write16(uint32_t address, uint16_t value)
{
	WriteImpl<uint16_t>(address, value);
}

void PhysicalMemory::Write32(uint32_t address, uint32_t value)
{
	WriteImpl<uint32_t>(address, value);
}

void PhysicalMemory::Write64(uint32_t address, uint64_t value)
{
	WriteImpl<uint64_t>(address, value);
}
