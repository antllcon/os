#pragma once

#include <string>
#include <vector>

class ArgParser
{
	constexpr static size_t MIN_THREADS = 1;

public:
	ArgParser(int argc, char* argv[]);
	void Parse();

	const std::string& GetInputDir() const;
	const std::string& GetOutputDir() const;
	size_t GetNumThreads() const;
	int GetThumbWidth() const;
	int GetThumbHeight() const;

private:
	const std::string& GetValueFor(const std::string& argName, size_t& index);
	void ParseSize(const std::string& sizeStr);

	std::vector<std::string> m_args;
	std::string m_inputDir;
	std::string m_outputDir;
	size_t m_numThreads = MIN_THREADS;
	int m_thumbWidth = 0;
	int m_thumbHeight = 0;
};