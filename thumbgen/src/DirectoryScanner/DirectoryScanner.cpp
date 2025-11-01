#include "DirectoryScanner.h"

#include <algorithm>
#include <filesystem>
#include <stdexcept>

namespace fs = std::filesystem;

namespace
{
void AssertIsDirecotryValid(const fs::path& dirPath)
{
	if (!fs::exists(dirPath) || !fs::is_directory(dirPath))
	{
		throw std::runtime_error("Нет такого файла: " + dirPath.string());
	}
}
} // namespace

std::vector<std::string> DirectoryScanner::Scan(const std::string& dirPath, const std::unordered_set<std::string>& extensions)
{
	std::vector<std::string> filePaths;
	AssertIsDirecotryValid(dirPath);

	for (const auto& file : fs::recursive_directory_iterator(dirPath))
	{
		if (file.is_regular_file())
		{
			std::string extensionFile = file.path().extension().string();
			if (extensions.contains(extensionFile))
			{
				filePaths.emplace_back(file.path().string());
			}
		}
	}

	std::ranges::sort(filePaths);
	return filePaths;
}