#pragma once

#include "../FileSystem/IFileSystem.h"
#include "../InvertedIndex/InvertedIndex.h"
#include "../ThreadPool/ThreadPool.h"
#include <filesystem>
#include <functional>
#include <string_view>
#include <vector>
#include <future>

class SearchService final
{
public:
	using BatchCallback = std::function<void(size_t queryId, std::string_view query, const std::vector<SearchResult>& results)>;

	SearchService(InvertedIndex& index, ThreadPool& pool, IFileSystem& fs);
	~SearchService() = default;

	SearchService(const SearchService&) = delete;
	SearchService& operator=(const SearchService&) = delete;

	std::vector<SearchResult> Find(std::string_view query) const;
	std::future<void> AddFileAsync(const Path& path) const;
	std::future<void> AddDirectoryAsync(const Path& path, bool recursive) const;
	std::future<void> RemoveFileAsync(const Path& path) const;
	std::future<void> RemoveDirectoryAsync(const Path& path, bool recursive) const;
	void FindBatchAsync(const Path& path, const BatchCallback& onResult) const;

private:
	void DoIndexFile(const Path& path) const;
	void DoRemoveFile(const Path& path) const noexcept;
	void DoProcessBatchFile(const Path& path, const BatchCallback& onResult) const;
	void DoSearchQuery(const std::string& query, size_t queryId, const BatchCallback& onResult) const noexcept;

	InvertedIndex& m_index;
	ThreadPool& m_threadPool;
	IFileSystem& m_fileSystem;
};