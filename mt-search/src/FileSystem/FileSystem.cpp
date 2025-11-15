#include "FileSystem.h"
#include <fstream>
#include <sstream>

namespace
{
void AssertIsFileOpen(const std::ifstream& file, const Path& path)
{
	if (!file.is_open())
	{
		throw std::filesystem::filesystem_error("Failed to open file", path, std::make_error_code(std::errc::no_such_file_or_directory));
	}
}

void AssertIsFileNotBad(const std::ifstream& file, const Path& path)
{
	if (file.bad())
	{
		throw std::filesystem::filesystem_error("Failed to read file", path, std::make_error_code(std::errc::io_error));
	}
}
} // namespace

bool FileSystem::Exists(const Path& path)
{
	return std::filesystem::exists(path);
}

bool FileSystem::IsDirectory(const Path& path)
{
	return std::filesystem::is_directory(path);
}

std::string FileSystem::ReadFile(const Path& path)
{
	const std::ifstream file(path);
	AssertIsFileOpen(file, path);

	std::stringstream buffer;
	buffer << file.rdbuf();

	AssertIsFileNotBad(file, path);
	return buffer.str();
}

std::vector<Path> FileSystem::ListDirectory(const Path& path)
{
	std::vector<Path> files;
	for (const auto& entry : std::filesystem::directory_iterator(path))
	{
		std::error_code ec;
		if (entry.is_regular_file(ec))
		{
			files.emplace_back(entry.path());
		}
	}

	return files;
}

std::vector<Path> FileSystem::ListDirectoryRecursive(const Path& path)
{
	std::vector<Path> files;
	for (const auto& entry : std::filesystem::recursive_directory_iterator(path))
	{
		std::error_code ec;
		if (entry.is_regular_file(ec))
		{
			files.emplace_back(entry.path());
		}
	}

	return files;
}