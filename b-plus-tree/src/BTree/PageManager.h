#pragma once

#include "Common.h"
#include "../MemoryMap/MemoryMappedFile.h"
#include <filesystem>

class PageManager
{
public:
	PageManager(const std::filesystem::path& path, bool createNew);

	template <typename T = NodeHeader>
		T* GetPage(uint64_t pageId);

	template <typename T = NodeHeader>
	const T* GetPage(uint64_t pageId) const;

	TreeHeader* GetHeader() const;
	uint64_t AllocatePage();
	void FreePage(uint64_t pageId);
	void Flush();

private:
	void InitNewFile();
	void ExpandFile(size_t count);
	void CheckPageId(uint64_t pageId) const;

	MemoryMappedFile m_file;
	TreeHeader* m_headerCache = nullptr;
};

template <typename T>
T* PageManager::GetPage(uint64_t pageId)
{
	CheckPageId(pageId);
	uint8_t* base = static_cast<uint8_t*>(m_file.GetBaseAddress());
	return reinterpret_cast<T*>(base + pageId * PAGE_SIZE);
}

template <typename T>
const T* PageManager::GetPage(uint64_t pageId) const
{
	CheckPageId(pageId);
	const uint8_t* base = static_cast<const uint8_t*>(m_file.GetBaseAddress());
	return reinterpret_cast<const T*>(base + pageId * PAGE_SIZE);
}