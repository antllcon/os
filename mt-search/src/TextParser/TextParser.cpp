#include "TextParser.h"
#include <string>
#include <vector>

namespace
{
bool IsAlpha(char ch) noexcept
{
	return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

std::string Normalize(std::string_view token) noexcept
{
	std::string normalized;
	normalized.reserve(token.size());

	for (const char ch : token)
	{
		normalized.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
	}

	return normalized;
}

void SkipSeparators(std::string_view text, std::string_view::size_type& pos, std::string_view::size_type size)
{
	while (pos < size && !IsAlpha(text[pos]))
	{
		++pos;
	}
}

std::string_view ExtractNextToken(std::string_view text, std::string_view::size_type& pos) noexcept
{
	const std::string_view::size_type start = pos;
	while (pos < text.size() && IsAlpha(text[pos]))
	{
		++pos;
	}
	return text.substr(start, pos - start);
}

void ProcessToken(std::string_view token, text_parser::ParsedDocument& result) noexcept
{
	const std::string normalizedToken = Normalize(token);
	result.wordCounts[normalizedToken]++;
	result.totalWords++;
}
} // namespace

namespace text_parser
{
ParsedDocument TokenizeAndCount(std::string_view text)
{
	ParsedDocument result;
	std::string_view::size_type pos = 0;
	const auto size = text.size();

	while (pos < size)
	{
		SkipSeparators(text, pos, size);

		if (pos == size)
		{
			break;
		}

		const std::string_view token = ExtractNextToken(text, pos);
		ProcessToken(token, result);
	}

	return result;
}

std::vector<std::string> TokenizeQuery(std::string_view query)
{
	std::vector<std::string> tokens;
	std::string_view::size_type pos = 0;
	const auto size = query.size();

	while (pos < size)
	{
		SkipSeparators(query, pos, size);

		if (pos == size)
		{
			break;
		}

		const std::string_view token = ExtractNextToken(query, pos);
		tokens.push_back(Normalize(token));
	}

	return tokens;
}
} // namespace text_parser