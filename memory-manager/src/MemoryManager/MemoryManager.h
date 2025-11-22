#pragma once

#include <cstddef>
#include <iostream>

class MemoryManager
{
	struct alignas(std::max_align_t) BlockHeader
	{
		BlockHeader* prev;
		BlockHeader* next;
		size_t size;
		bool isFree;
	};

public:
	MemoryManager(void* start, size_t size) noexcept;
	MemoryManager(const MemoryManager&) = delete;
	MemoryManager& operator=(const MemoryManager&) = delete;

	void* Allocate(size_t size, size_t align = sizeof(std::max_align_t)) noexcept;
	static void Free(void* address) noexcept;

private:
	static uintptr_t AlignUp(uintptr_t address, size_t align);
	static void* GetDataPtr(BlockHeader* header);
	static BlockHeader* GetHeader(void* dataPtr);

	BlockHeader* m_head = nullptr;
};