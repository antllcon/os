#pragma once

#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace text_parser
{
struct ParsedDocument
{
	// Слово - совпадения
	std::map<std::string, size_t> wordCounts;
	size_t totalWords = 0;
};

ParsedDocument TokenizeAndCount(std::string_view text);
std::vector<std::string> TokenizeQuery(std::string_view query);
} // namespace text_parser