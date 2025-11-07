#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace command_parser
{
enum class CommandType
{
	AddFile,
	AddDir,
	AddDirRecursive,
	Find,
	FindBatch,
	RemoveFile,
	RemoveDir,
	RemoveDirRecursive,
	Unknown,
	Empty
};

struct ParsedCommand
{
	CommandType type = CommandType::Unknown;
	std::vector<std::string> arguments;
};

ParsedCommand Parse(std::string_view line);
} // namespace command_parser