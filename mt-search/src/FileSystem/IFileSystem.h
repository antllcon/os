#pragma once

#include <filesystem>
#include <string>
#include <vector>

using Path = std::filesystem::path;

class IFileSystem
{
public:
	virtual ~IFileSystem() = default;
	virtual bool Exists(const Path& path) = 0;
	virtual bool IsDirectory(const Path& path) = 0;
	virtual std::string ReadFile(const Path& path) = 0;
	virtual std::vector<Path> ListDirectory(const Path& path) = 0;
	virtual std::vector<Path> ListDirectoryRecursive(const Path& path) = 0;
};