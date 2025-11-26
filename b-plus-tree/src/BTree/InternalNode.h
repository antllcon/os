#pragma once

#include "Common.h"
#include "NodeWrappers.h"

#include <stdexcept>

class InternalNode : public BaseNode
{
public:
	explicit InternalNode(NodeHeader* header)
		: BaseNode(header)
	{
		if (header->type != NodeType::Internal)
			throw std::logic_error("Used on non-internal page");
	}

	uint64_t GetKey(uint16_t index) const
	{
		if (index >= m_header->numKeys)
		{
			throw std::out_of_range("Internal key index out of range");
		}

		uint8_t* ptr = GetPayload() + 8 + (index * 16);
		return *reinterpret_cast<uint64_t*>(ptr);
	}

	void SetKey(uint16_t index, uint64_t key)
	{
		if (index >= m_header->numKeys) throw std::out_of_range("Internal key index out of range");
		uint8_t* ptr = GetPayload() + 8 + (index * 16);
		*reinterpret_cast<uint64_t*>(ptr) = key;
	}

	uint64_t GetChild(uint16_t index) const
	{
		if (index > m_header->numKeys) throw std::out_of_range("Internal child index out of range");

		if (index == 0)
		{
			return *reinterpret_cast<uint64_t*>(GetPayload());
		}
		uint8_t* ptr = GetPayload() + 8 + ((index - 1) * 16) + 8;
		return *reinterpret_cast<uint64_t*>(ptr);
	}

	void SetChild(uint16_t index, uint64_t pageId)
	{
		if (index > m_header->numKeys) throw std::out_of_range("Internal child index out of range");

		if (index == 0)
		{
			*reinterpret_cast<uint64_t*>(GetPayload()) = pageId;
		}
		else
		{
			uint8_t* ptr = GetPayload() + 8 + ((index - 1) * 16) + 8;
			*reinterpret_cast<uint64_t*>(ptr) = pageId;
		}
	}

	void InsertAt(uint16_t index, uint64_t key, uint64_t rightChild)
	{
		uint16_t count = m_header->numKeys;
		uint8_t* destPtr = GetPayload() + 8 + (index * 16);

		if (index < count)
		{
			std::memmove(destPtr + 16, destPtr, (count - index) * 16);
		}

		*reinterpret_cast<uint64_t*>(destPtr) = key;
		*reinterpret_cast<uint64_t*>(destPtr + 8) = rightChild;

		m_header->numKeys++;
	}

    void RemoveAt(uint16_t index)
    {
        uint16_t count = m_header->numKeys;
        if (index >= count) throw std::out_of_range("RemoveAt index out of range");

        uint8_t* destPtr = GetPayload() + 8 + (index * 16);

        if (index < count - 1)
        {
            std::memmove(destPtr, destPtr + 16, (count - index - 1) * 16);
        }

        m_header->numKeys--;
    }
};