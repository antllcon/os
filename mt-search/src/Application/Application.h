#pragma once

#include "../FileSystem/FileSystem.h"
#include "../SearchService/SearchService.h"
#include "../ThreadPool/ThreadPool.h"
#include <atomic>
#include <chrono>
#include <string>
#include <vector>

class Application
{
public:
	explicit Application(unsigned numThreads = std::thread::hardware_concurrency());
	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;

	void Run();

private:
	void ProcessCommand(const std::string& line);
	void HandleAddFile(const std::vector<std::string>& args);
	void HandleAddDir(const std::vector<std::string>& args, bool recursive);
	void HandleRemoveFile(const std::vector<std::string>& args);
	void HandleRemoveDir(const std::vector<std::string>& args, bool recursive);
	void HandleFind(const std::vector<std::string>& args);
	void HandleFindBatch(const std::vector<std::string>& args);

	static void PrintResults(const std::vector<SearchResult>& results) noexcept;
	static void PrintBatchResult(size_t queryId, std::string_view query, const std::vector<SearchResult>& results) noexcept;

	ThreadPool m_threadPool;
	FileSystem m_fileSystem;
	InvertedIndex m_index;
	SearchService m_service;
	std::atomic<bool> m_isRunning{true};
};