#pragma once

#include "../FileSystem/IFileSystem.h"
#include <atomic>
#include <map>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <vector>

using ID = uint64_t;

struct Document
{
	ID id = 0;
	Path path;
	size_t wordCount = 0;
};

struct Posting
{
	ID documentId = 0;
	size_t termFrequency = 0;
};

struct SearchResult
{
	ID documentId = 0;
	double relevance = 0.0;
	Path path;
};

class InvertedIndex final
{
public:
	InvertedIndex();
	InvertedIndex(const InvertedIndex&) = delete;
	InvertedIndex& operator=(const InvertedIndex&) = delete;

	void AddDocument(const Path& path, const std::map<std::string, size_t>& wordCounts, size_t totalWords);
	void RemoveDocument(const Path& path);
	std::vector<SearchResult> Search(const std::vector<std::string>& queryTokens) const;

private:
	void InternalRemove(ID documentId, const Path& path);
	void StoreDocument(ID newId, const Path& path, size_t totalWords, const std::map<std::string, size_t>& wordCounts);
	void AddDocumentToIndex(ID newId, const std::map<std::string, size_t>& wordCounts);
	void RemoveDocumentFromIndex(ID documentId, const std::map<std::string, size_t>& wordCounts);
	void RemoveDocumentMetadata(ID documentId, const Path& path);
	std::map<ID, double> CalculateRelevanceScores(const std::vector<std::string>& queryTokens, size_t totalDocumentCount) const;
	static double CalculateIdf(size_t documentsWithTerm, size_t totalDocumentCount) noexcept;
	double CalculateTf(const Posting& posting) const;
	void AccumulateScoresForTerm(std::map<ID, double>& docScores, const std::vector<Posting>& postings, double idf) const;
	std::vector<SearchResult> CollectAndFilterResults(const std::map<ID, double>& docScores) const;
	static void RankAndTruncateResults(std::vector<SearchResult>& results) noexcept;

	std::map<std::string, std::vector<Posting>> m_index;
	std::map<ID, Document> m_documents;
	std::map<ID, std::map<std::string, size_t>> m_documentWordCounts;
	std::map<Path, ID> m_pathToId;
	std::atomic<ID> m_nextDocumentId;
	mutable std::shared_mutex m_mutex;
};