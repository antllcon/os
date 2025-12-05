#include "PageManager.h"

constexpr size_t EXPANSION_BATCH_SIZE = 1024;

PageManager::PageManager(const std::filesystem::path& path, bool createNew)
	: m_file(path, createNew)
{
	if (createNew || m_file.GetSize() == 0)
	{
		InitNewFile();
	}

	m_headerCache = reinterpret_cast<TreeHeader*>(m_file.GetBaseAddress());

	if (m_headerCache->magic != MAGIC_NUMBER)
	{
		throw std::runtime_error("File is not BPL1 database");
	}
}

TreeHeader* PageManager::GetHeader() const
{
	return m_headerCache;
}

uint64_t PageManager::AllocatePage()
{
	if (m_headerCache->freeHead == NULL_PAGE)
	{
		ExpandFile(EXPANSION_BATCH_SIZE);
	}

	uint64_t pageId = m_headerCache->freeHead;

	FreePageNode* freeNode = GetPage<FreePageNode>(pageId);
	m_headerCache->freeHead = freeNode->nextFreePage;
	std::memset(freeNode, 0, PAGE_SIZE);

	m_headerCache->nodesCount++;
	return pageId;
}

void PageManager::FreePage(uint64_t pageId)
{
	if (pageId == NULL_PAGE || pageId >= m_headerCache->nextPid)
	{
		throw std::invalid_argument("Invalid page ID for free");
	}

	FreePageNode* node = GetPage<FreePageNode>(pageId);
	std::memset(node, 0, PAGE_SIZE);

	node->type = NodeType::Free;
	node->nextFreePage = m_headerCache->freeHead;
	m_headerCache->freeHead = pageId;

	if (m_headerCache->nodesCount > 0)
	{
		m_headerCache->nodesCount--;
	}
}

void PageManager::Flush()
{
	m_file.FlushToDisk();
}

void PageManager::InitNewFile()
{
	m_file.Resize(PAGE_SIZE);

	auto* header = reinterpret_cast<TreeHeader*>(m_file.GetBaseAddress());
	std::memset(header, 0, PAGE_SIZE);

	header->magic = MAGIC_NUMBER;
	header->version = FILE_VERSION;
	header->pageSize = PAGE_SIZE;
	header->rootPage = NULL_PAGE;
	header->orderLeaf = MAX_LEAF_KEYS;
	header->orderInternal = MAX_INTERNAL_KEYS;
	header->freeHead = NULL_PAGE;
	header->nextPid = 1;
	header->keysCount = 0;
	header->nodesCount = 0;
}

void PageManager::ExpandFile(size_t count)
{
	uint64_t startId = m_headerCache->nextPid;
	uint64_t endId = startId + count;
	size_t newSize = endId * PAGE_SIZE;

	m_file.Resize(newSize);
	m_headerCache = reinterpret_cast<TreeHeader*>(m_file.GetBaseAddress());
	m_headerCache->nextPid = endId;

	for (uint64_t i = startId; i < endId; ++i)
	{
		FreePageNode* node = GetPage<FreePageNode>(i);
		node->type = NodeType::Free;

		if (i < endId - 1)
		{
			node->nextFreePage = i + 1;
		}
		else
		{
			node->nextFreePage = m_headerCache->freeHead;

		}
	}

	m_headerCache->freeHead = startId;
}

void PageManager::CheckPageId(uint64_t pageId) const
{
	if (pageId >= m_headerCache->nextPid)
	{
		throw std::out_of_range("Page ID out of bounds");
	}
}