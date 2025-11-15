#include "SearchService.h"

#include "../ScopeTimer/ScopeTimer.h"
#include "../TextParser/TextParser.h"

#include <fstream>
#include <sstream>

SearchService::SearchService(InvertedIndex& index, ThreadPool& pool, IFileSystem& fs)
	: m_index(index)
	, m_threadPool(pool)
	, m_fileSystem(fs)
{
}

std::future<void> SearchService::AddFileAsync(const Path& path) const
{
	return m_threadPool.Dispatch([this, path] {
	   DoIndexFile(path);
	});
}

std::future<void> SearchService::AddDirectoryAsync(const Path& path, bool recursive) const
{
	return m_threadPool.Dispatch([this, path, recursive] {
	   try
	   {
		  const auto files = recursive
			 ? m_fileSystem.ListDirectoryRecursive(path)
			 : m_fileSystem.ListDirectory(path);

		  std::vector<std::future<void>> futures;
		  futures.reserve(files.size());

		  for (const auto& file : files)
		  {
			 futures.push_back(AddFileAsync(file));
		  }

		  for (auto& f : futures)
		  {
			 f.get();
		  }
	   }
	   catch (const std::filesystem::filesystem_error& e)
	   {
		  (void)fprintf(stderr, "failed: %s\n", e.what());
		  throw;
	   }
	});
}

std::future<void> SearchService::RemoveFileAsync(const Path& path) const
{
	return m_threadPool.Dispatch([this, path] {
	   DoRemoveFile(path);
	});
}

std::future<void> SearchService::RemoveDirectoryAsync(const Path& path, bool recursive) const
{
	return m_threadPool.Dispatch([this, path, recursive] {
	   try
	   {
		  const auto files = recursive
			 ? m_fileSystem.ListDirectoryRecursive(path)
			 : m_fileSystem.ListDirectory(path);

		  std::vector<std::future<void>> futures;
		  futures.reserve(files.size());

		  for (const auto& file : files)
		  {
			 futures.push_back(RemoveFileAsync(file));
		  }

		  for (auto& f : futures)
		  {
			 f.get();
		  }
	   }
	   catch (const std::filesystem::filesystem_error& e)
	   {
		  (void)fprintf(stderr, "failed: %s\n", e.what());
		  throw;
	   }
	});
}

std::vector<SearchResult> SearchService::Find(std::string_view query) const
{
	const auto tokens = text_parser::TokenizeQuery(query);
	return m_index.Search(tokens);
}

void SearchService::FindBatchAsync(const Path& path, const BatchCallback& onResult) const
{
	m_threadPool.Dispatch([this, path, onResult] {
		DoProcessBatchFile(path, onResult);
	});
}

void SearchService::DoIndexFile(const Path& path) const
{
	try
	{
		const std::string content = m_fileSystem.ReadFile(path);
		const auto [wordCounts, totalWords] = text_parser::TokenizeAndCount(content);
		m_index.AddDocument(path, wordCounts, totalWords);
	}
	catch (const std::filesystem::filesystem_error& e)
	{
		(void)fprintf(stderr, "failed: %s\n", e.what());
	}
}

void SearchService::DoRemoveFile(const Path& path) const noexcept
{
	m_index.RemoveDocument(path);
}

void SearchService::DoProcessBatchFile(const Path& path, const BatchCallback& onResult) const
{
	try
	{
		const std::string content = m_fileSystem.ReadFile(path);
		std::stringstream ss(content);
		std::string line;

		size_t queryId = 1;

		while (std::getline(ss, line))
		{
			if (line.empty())
			{
				continue;
			}

			std::string query(line);

			m_threadPool.Dispatch([this, query = std::move(query), queryId, onResult] {
				DoSearchQuery(std::move(query), queryId, onResult);
			});

			queryId++;
		}
	}
	catch (const std::filesystem::filesystem_error& e)
	{
		(void)fprintf(stderr, "failed: %s\n", e.what());
	}
}

void SearchService::DoSearchQuery(const std::string& query, size_t queryId, const BatchCallback& onResult) const noexcept
{
	try
	{
		ScopeTimer time("Search took");
		const auto tokens = text_parser::TokenizeQuery(query);
		const auto results = m_index.Search(tokens);
		onResult(queryId, query, results);
	}
	catch (...)
	{
		// onResult(queryId, query, {});
	}
}