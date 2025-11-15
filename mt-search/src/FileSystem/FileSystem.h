#pragma once

#include "IFileSystem.h"

class FileSystem final : public IFileSystem
{
public:
	bool Exists(const Path& path) override;
	bool IsDirectory(const Path& path) override;
	std::string ReadFile(const Path& path) override;
	std::vector<Path> ListDirectory(const Path& path) override;
	std::vector<Path> ListDirectoryRecursive(const Path& path) override;
};