#include "MemoryManager.h"
#include <algorithm>

MemoryManager::MemoryManager(void* start, size_t size) noexcept
{
	void* alignedStart = start;

	if (!std::align(alignof(BlockHeader), sizeof(BlockHeader), alignedStart, size))
	{
		m_head = nullptr;
		return;
	}

	m_head = static_cast<BlockHeader*>(alignedStart);
	m_head->size = size - sizeof(BlockHeader);
	m_head->isFree = true;
	m_head->next = nullptr;
	m_head->prev = nullptr;
}

void* MemoryManager::Allocate(size_t size, size_t align) noexcept
{
	if (!m_head || size == 0)
	{
		return nullptr;
	}

	BlockHeader* current = m_head;

	while (current)
	{
		if (!current->isFree)
		{
			current = current->next;
			continue;
		}

		const auto currentDataPtr = reinterpret_cast<uintptr_t>(GetDataPtr(current));
		const auto alignedDataPtr = AlignUp(currentDataPtr, align);
		const auto padding = alignedDataPtr - currentDataPtr;

		if (current->size < padding + size)
		{
			current = current->next;
			continue;
		}

		if (padding > 0)
		{
			if (current->prev == nullptr)
			{
				current = current->next;
				continue;
			}

			const auto newHeaderPtr = reinterpret_cast<uintptr_t>(current) + padding;
			auto* newHeader = reinterpret_cast<BlockHeader*>(newHeaderPtr);

			newHeader->size = current->size - padding;
			newHeader->isFree = true;
			newHeader->next = current->next;
			newHeader->prev = current->prev;

			if (newHeader->next)
			{
				newHeader->next->prev = newHeader;
			}

			newHeader->prev->next = newHeader;
			newHeader->prev->size += padding;
			current = newHeader;
		}

		const size_t requiredSize = size;
		const size_t minBlockSize = sizeof(BlockHeader) + 1;

		if (current->size >= requiredSize + minBlockSize)
		{
			const auto splitBlockPtr = reinterpret_cast<uintptr_t>(GetDataPtr(current)) + requiredSize;
			auto* splitBlock = reinterpret_cast<BlockHeader*>(splitBlockPtr);

			splitBlock->size = current->size - requiredSize - sizeof(BlockHeader);
			splitBlock->isFree = true;
			splitBlock->next = current->next;
			splitBlock->prev = current;

			if (splitBlock->next)
			{
				splitBlock->next->prev = splitBlock;
			}

			current->next = splitBlock;
			current->size = requiredSize;
		}

		current->isFree = false;
		return GetDataPtr(current);
	}

	return nullptr;
}

void MemoryManager::Free(void* addr) noexcept
{
	if (!addr)
	{
		return;
	}

	BlockHeader* current = GetHeader(addr);

	if (current->isFree)
	{
		return;
	}

	current->isFree = true;

	if (current->next && current->next->isFree)
	{
		BlockHeader* nextBlock = current->next;
		current->size += sizeof(BlockHeader) + nextBlock->size;

		current->next = nextBlock->next;
		if (current->next)
		{
			current->next->prev = current;
		}
	}

	if (current->prev && current->prev->isFree)
	{
		BlockHeader* prevBlock = current->prev;
		prevBlock->size += sizeof(BlockHeader) + current->size;

		prevBlock->next = current->next;
		if (prevBlock->next)
		{
			prevBlock->next->prev = prevBlock;
		}
	}
}

uintptr_t MemoryManager::AlignUp(uintptr_t address, size_t align)
{
	if (align == 0) return address;
	size_t remainder = address % align;
	if (remainder == 0) return address;
	return address + align - remainder;
}

void* MemoryManager::GetDataPtr(BlockHeader* header)
{
	return reinterpret_cast<char*>(header) + sizeof(BlockHeader);
}

MemoryManager::BlockHeader* MemoryManager::GetHeader(void* dataPtr)
{
	return reinterpret_cast<BlockHeader*>(static_cast<char*>(dataPtr) - sizeof(BlockHeader));
}