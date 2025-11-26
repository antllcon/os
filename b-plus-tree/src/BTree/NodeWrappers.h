#pragma once

#include "Common.h"
#include <algorithm>

class BaseNode
{
public:
	explicit BaseNode(NodeHeader* header)
		: m_header(header)
	{
	}

	NodeType GetType() const
	{
		return m_header->type;
	}

	uint16_t GetCount() const
	{
		return m_header->numKeys;
	}

	void SetCount(uint16_t numKeys)
	{
		m_header->numKeys = numKeys;
	}

	uint64_t GetParent() const
	{
		return m_header->parent;
	}

	void SetParent(uint64_t newParent)
	{
		m_header->parent = newParent;
	}

	bool IsLeaf() const
	{
		return m_header->type == NodeType::Leaf;
	}
	bool IsInternal() const
	{
		return m_header->type == NodeType::Internal;
	}

protected:
	uint8_t* GetPayload() const
	{
		return reinterpret_cast<uint8_t*>(m_header) + HEADER_SIZE;
	}

	NodeHeader* m_header;
};