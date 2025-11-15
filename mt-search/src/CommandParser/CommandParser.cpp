#include "CommandParser.h"

#include <map>
#include <string_view>

namespace
{
constexpr std::string_view WHITESPACE = " \n\r\t\v\f";

const std::map<std::string_view, command_parser::CommandType> commandMap = {
	{ "add_file", command_parser::CommandType::AddFile },
	{ "add_dir", command_parser::CommandType::AddDir },
	{ "add_dir_recursive", command_parser::CommandType::AddDirRecursive },
	{ "find", command_parser::CommandType::Find },
	{ "find_batch", command_parser::CommandType::FindBatch },
	{ "remove_file", command_parser::CommandType::RemoveFile },
	{ "remove_dir", command_parser::CommandType::RemoveDir },
	{ "remove_dir_recursive", command_parser::CommandType::RemoveDirRecursive },
};

std::string_view TrimLeft(std::string_view str)
{
	str.remove_prefix(std::min(str.find_first_not_of(WHITESPACE), str.size()));
	return str;
}

std::string_view TrimRight(std::string_view str)
{
	const auto lastChar = str.find_last_not_of(WHITESPACE);
	if (lastChar == std::string_view::npos)
	{
		return {};
	}
	str.remove_suffix(str.size() - lastChar - 1);
	return str;
}

std::string_view Trim(std::string_view str)
{
	return TrimLeft(TrimRight(str));
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

std::pair<std::string_view, std::string_view> SplitCommandAndArgs(std::string_view line)
{
	const auto firstSpace = line.find_first_of(WHITESPACE);

	if (firstSpace == std::string_view::npos)
	{
		return { line, {} };
	}

	const std::string_view command = line.substr(0, firstSpace);
	const std::string_view args = TrimLeft(line.substr(firstSpace));
	return { command, args };
}

std::vector<std::string> TokenizeArguments(std::string_view argsLine)
{
	std::vector<std::string> arguments;

	std::string_view::size_type pos = 0;
	std::string_view::size_type prev = 0;

	while ((pos = argsLine.find_first_of(WHITESPACE, prev)) != std::string_view::npos)
	{
		if (pos > prev)
		{
			arguments.emplace_back(argsLine.substr(prev, pos - prev));
		}
		prev = pos + 1;
	}

	if (prev < argsLine.length())
	{
		arguments.emplace_back(argsLine.substr(prev, std::string_view::npos));
	}

	return arguments;
}
} // namespace

namespace command_parser
{
ParsedCommand Parse(std::string_view line)
{
	ParsedCommand cmd;

	const std::string_view trimmedLine = Trim(line);
	if (trimmedLine.empty())
	{
		cmd.type = CommandType::Empty;
		return cmd;
	}

	const auto [commandStr, argsLine] = SplitCommandAndArgs(trimmedLine);
	cmd.type = GetCommandType(commandStr);

	if (!argsLine.empty())
	{
		cmd.arguments = TokenizeArguments(argsLine);
	}

	return cmd;
}
} // namespace command_parser