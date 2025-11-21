#include "InvertedIndex.h"

#include <algorithm>
#include <ranges>

InvertedIndex::InvertedIndex()
	: m_nextDocumentId{1}
{
}

void InvertedIndex::AddDocument(const Path& path, const std::map<std::string, size_t>& wordCounts, size_t totalWords)
{
	std::unique_lock<std::shared_mutex> lock(m_mutex);

	if (auto it = m_pathToId.find(path); it != m_pathToId.end())
	{
		InternalRemove(it->second, it->first);
	}

	const ID newId = m_nextDocumentId.fetch_add(1);
	StoreDocument(newId, path, totalWords, wordCounts);
	AddDocumentToIndex(newId, wordCounts);
}

// Добавить безопасное добавление
void InvertedIndex::StoreDocument(ID newId, const Path& path, size_t totalWords, const std::map<std::string, size_t>& wordCounts)
{
	m_documents[newId] = Document{newId, path, totalWords};
	m_pathToId[path] = newId;
	m_documentWordCounts[newId] = wordCounts;
}

void InvertedIndex::AddDocumentToIndex(ID newId, const std::map<std::string, size_t>& wordCounts)
{
	for (const auto& [term, frequency] : wordCounts)
	{
		m_index[term].push_back(Posting{newId, frequency});
	}
}

void InvertedIndex::RemoveDocument(const Path& path)
{
	std::unique_lock<std::shared_mutex> lock(m_mutex);

	const auto it = m_pathToId.find(path);
	if (it == m_pathToId.end())
	{
		return;
	}

	InternalRemove(it->second, it->first);
}

void InvertedIndex::InternalRemove(ID documentId, const Path& path)
{
	const auto wordsIt = m_documentWordCounts.find(documentId);
	if (wordsIt == m_documentWordCounts.end())
	{
		return;
	}

	RemoveDocumentFromIndex(documentId, wordsIt->second);
	RemoveDocumentMetadata(documentId, path);
}

void InvertedIndex::RemoveDocumentFromIndex(ID documentId, const std::map<std::string, size_t>& wordCounts)
{
	for (const auto& term : wordCounts | std::views::keys)
	{
		auto indexIt = m_index.find(term);
		if (indexIt == m_index.end())
		{
			continue;
		}

		auto& postings = indexIt->second;

		std::erase_if(postings, [documentId](const Posting& p) {
			return p.documentId == documentId;
		});

		if (postings.empty())
		{
			m_index.erase(indexIt);
		}
	}
}

void InvertedIndex::RemoveDocumentMetadata(ID documentId, const Path& path)
{
	m_documentWordCounts.erase(documentId);
	m_documents.erase(documentId);
	m_pathToId.erase(path);
}

std::vector<SearchResult> InvertedIndex::Search(const std::vector<std::string>& queryTokens) const
{
	std::shared_lock<std::shared_mutex> lock(m_mutex);
	const size_t n = m_documents.size();
	if (n == 0 || queryTokens.empty())
	{
		return {};
	}

	std::map<ID, double> docScores = CalculateRelevanceScores(queryTokens, n);
	std::vector<SearchResult> results = CollectAndFilterResults(docScores);
	RankAndTruncateResults(results);
	return results;
}

std::map<ID, double> InvertedIndex::CalculateRelevanceScores(const std::vector<std::string>& queryTokens, size_t totalDocumentCount) const
{
	// использовать вектор
	std::map<ID, double> docScores;
	for (const auto& term : queryTokens)
	{
		const auto termIt = m_index.find(term);
		if (termIt == m_index.end())
		{
			continue;
		}

		const auto& postings = termIt->second;
		const double idf = CalculateIdf(postings.size(), totalDocumentCount);

		if (idf == 0.0)
		{
			continue;
		}

		AccumulateScoresForTerm(docScores, postings, idf);
	}

	return docScores;
}

double InvertedIndex::CalculateIdf(size_t documentsWithTerm, size_t totalDocumentCount) noexcept
{
	return std::log(1.0 + (static_cast<double>(totalDocumentCount) / documentsWithTerm));
}

void InvertedIndex::AccumulateScoresForTerm(std::map<ID, double>& docScores, const std::vector<Posting>& postings, double idf) const
{
	for (const auto& posting : postings)
	{
		const double tf = CalculateTf(posting);
		if (tf > 0.0)
		{
			const double tfIdfScore = tf * idf;
			docScores[posting.documentId] += tfIdfScore;
		}
	}
}

double InvertedIndex::CalculateTf(const Posting& posting) const
{
	const auto docIt = m_documents.find(posting.documentId);
	if (docIt == m_documents.end())
	{
		return 0.0;
	}

	const Document& doc = docIt->second;
	if (doc.wordCount == 0)
	{
		return 0.0;
	}

	return static_cast<double>(posting.termFrequency) / doc.wordCount;
}

std::vector<SearchResult> InvertedIndex::CollectAndFilterResults(const std::map<ID, double>& docScores) const
{
	std::vector<SearchResult> results;
	results.reserve(docScores.size());

	for (const auto& [documentId, relevance] : docScores)
	{
		if (relevance > 0.0)
		{
			const auto docIt = m_documents.find(documentId);
			if (docIt != m_documents.end())
			{
				results.push_back(SearchResult{documentId, relevance, docIt->second.path});
			}
		}
	}
	return results;
}

void InvertedIndex::RankAndTruncateResults(std::vector<SearchResult>& results) noexcept
{
	std::ranges::sort(results, [](const auto& a, const auto& b) {
		return a.relevance > b.relevance;
	});

	constexpr size_t MAX_RESULTS = 10;
	if (results.size() > MAX_RESULTS)
	{
		results.resize(MAX_RESULTS);
	}
}