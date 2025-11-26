#pragma once
#include <cstdint>

constexpr uint32_t PAGE_SIZE = 4096;
constexpr uint32_t HEADER_SIZE = 64;
constexpr uint32_t PAYLOAD_SIZE = 4032;

constexpr size_t MAX_VALUE_LENGTH = 119;
constexpr size_t NODE_HEADER_PADDING_SIZE = 32;
constexpr size_t FREE_NODE_PADDING_SIZE = 7;

constexpr uint32_t MAGIC_NUMBER = 0x314C5042;
constexpr uint32_t FILE_VERSION = 1;
constexpr uint64_t NULL_PAGE = static_cast<uint64_t>(-1);

constexpr uint16_t MAX_INTERNAL_KEYS = 250;
constexpr uint16_t MIN_INTERNAL_KEYS = 125;
constexpr uint16_t MAX_LEAF_KEYS = 31;
constexpr uint16_t MIN_LEAF_KEYS = 16;

enum class NodeType : uint8_t
{
	Internal = 0,
	Leaf = 1,
	Free = 0xFF
};

#pragma pack(push, 1)

struct TreeHeader
{
	uint32_t magic;
	uint32_t version;
	uint32_t pageSize;
	uint32_t flags;
	uint64_t rootPage;
	uint32_t height;
	uint16_t orderLeaf;
	uint16_t orderInternal;
	uint64_t freeHead;
	uint64_t nextPid;
	uint64_t keysCount;
	uint64_t nodesCount;
	uint8_t padding[PAYLOAD_SIZE];
};

struct NodeHeader
{
	NodeType type;
	uint8_t reserved;
	uint16_t numKeys;
	uint32_t align;
	uint64_t parent;
	uint64_t nextLeaf;
	uint64_t prevLeaf;
	uint8_t padding[NODE_HEADER_PADDING_SIZE];
};

struct LeafRecord
{
	uint64_t key;
	uint8_t len;
	char value[MAX_VALUE_LENGTH];
};

struct FreePageNode
{
	NodeType type;
	uint8_t padding[FREE_NODE_PADDING_SIZE];
	uint64_t nextFreePage;
};

#pragma pack(pop)