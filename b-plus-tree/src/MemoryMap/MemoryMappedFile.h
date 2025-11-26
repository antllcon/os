#pragma once
#include <filesystem>

class MemoryMappedFile
{
public:
	MemoryMappedFile(const std::filesystem::path& filePath, bool createNew);
	~MemoryMappedFile();

	MemoryMappedFile(const MemoryMappedFile& other) = delete;
	MemoryMappedFile& operator=(const MemoryMappedFile& other) = delete;

	MemoryMappedFile(MemoryMappedFile&& other) = delete;
	MemoryMappedFile& operator=(MemoryMappedFile&& other) = delete;

	void* GetBaseAddress() const;
	size_t GetSize() const;
	void FlushToDisk();
	void Resize(size_t newSize);

private:
	void OpenFile(const std::filesystem::path& filePath, bool createNew);
	void MapFile();
	void Cleanup();

	void* m_fileHandle = nullptr;
	void* m_mapHandle = nullptr;
	void* m_baseAddress = nullptr;
	size_t m_size = 0;
};