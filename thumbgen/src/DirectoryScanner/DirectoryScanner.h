#pragma once
#include <string>
#include <unordered_set>
#include <vector>

class DirectoryScanner
{
public:
	static std::vector<std::string> Scan(const std::string& dirPath, const std::unordered_set<std::string>& fileExtensions);
};