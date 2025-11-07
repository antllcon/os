#include "CommandParser.h"

#include <map>
#include <string_view>

namespace
{
constexpr std::string_view WHITESPACE = " \n\r\t\v\f";

static const std::map<std::string_view, command_parser::CommandType> commandMap = {
	{ "add_file", command_parser::CommandType::AddFile },
	{ "add_dir", command_parser::CommandType::AddDir },
	{ "add_dir_recursive", command_parser::CommandType::AddDirRecursive },
	{ "find", command_parser::CommandType::Find },
	{ "find_batch", command_parser::CommandType::FindBatch },
	{ "remove_file", command_parser::CommandType::RemoveFile },
	{ "remove_dir", command_parser::CommandType::RemoveDir },
	{ "remove_dir_recursive", command_parser::CommandType::RemoveDirRecursive },
};

std::string_view TrimLeft(std::string_view s)
{
	s.remove_prefix(std::min(s.find_first_not_of(WHITESPACE), s.size()));
	return s;
}

std::string_view TrimRight(std::string_view s)
{
	const auto lastChar = s.find_last_not_of(WHITESPACE);
	if (lastChar == std::string_view::npos)
	{
		return {};
	}
	s.remove_suffix(s.size() - lastChar - 1);
	return s;
}

std::string_view Trim(std::string_view s)
{
	return TrimLeft(TrimRight(s));
}

command_parser::CommandType GetCommandType(std::string_view command)
{
	const auto it = commandMap.find(command);
	if (it != commandMap.end())
	{
		return it->second;
	}
	return command_parser::CommandType::Unknown;
}
} //