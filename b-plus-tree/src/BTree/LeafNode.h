#pragma once
#include "Common.h"
#include "NodeWrappers.h"

#include <stdexcept>

class LeafNode : public BaseNode
{
public:
    explicit LeafNode(NodeHeader* header)
       : BaseNode(header)
    {
       if (header->type != NodeType::Leaf)
       {
          throw std::logic_error("Used on non-leaf page");
       }
    }

    LeafRecord& GetRecord(uint16_t index)
    {
       if (index >= m_header->numKeys)
       {
          throw std::out_of_range("Leaf index out of range");
       }

       return GetRecordsArray()[index];
    }

    const LeafRecord& GetRecord(uint16_t index) const
    {
       if (index >= m_header->numKeys)
          throw std::out_of_range("Leaf index out of range");

       return GetRecordsArray()[index];
    }

    void InsertAt(uint16_t index, uint64_t key, const std::string_view& value)
    {
       LeafRecord* records = GetRecordsArray();
       uint16_t count = m_header->numKeys;

       if (index < count)
       {
          std::memmove(&records[index + 1], &records[index],
             (count - index) * sizeof(LeafRecord));
       }

       records[index].key = key;

       size_t copyLen = std::min(value.size(), MAX_VALUE_LENGTH);
       records[index].len = static_cast<uint8_t>(copyLen);

       std::memcpy(records[index].value, value.data(), copyLen);

       if (copyLen < MAX_VALUE_LENGTH)
       {
          std::memset(records[index].value + copyLen, 0, MAX_VALUE_LENGTH - copyLen);
       }

       m_header->numKeys++;
    }

    void RemoveAt(uint16_t index)
    {
       LeafRecord* records = GetRecordsArray();
       uint16_t count = m_header->numKeys;

       if (index >= count) throw std::out_of_range("RemoveAt index out of range");

       if (index < count - 1)
       {
          std::memmove(&records[index], &records[index + 1],
             (count - index - 1) * sizeof(LeafRecord));
       }

       m_header->numKeys--;
    }

    uint64_t GetNext() const { return m_header->nextLeaf; }
    void SetNext(uint64_t pid) { m_header->nextLeaf = pid; }

    uint64_t GetPrev() const { return m_header->prevLeaf; }
    void SetPrev(uint64_t pid) { m_header->prevLeaf = pid; }

private:
    LeafRecord* GetRecordsArray() const
    {
       return reinterpret_cast<LeafRecord*>(GetPayload());
    }
};