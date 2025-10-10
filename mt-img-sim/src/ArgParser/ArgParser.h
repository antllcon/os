#pragma once

#include <string>
#include <vector>

class ArgParser
{
	constexpr static unsigned int MIN_THREADS = 1;
	constexpr static size_t MIN_SIM_IMAGES = 3;
	constexpr static double QUALITIY_BOUNDARY = 0.8;

public:
	ArgParser(int argc, char* argv[]);
	void Parse();

	const std::string& GetQueryImagePath() const;
	const std::string& GetInputDir() const;
	size_t GetNumThreads() const;
	size_t GetTopK() const;
	double GetThreshold() const;

private:
	const std::string& GetValueFor(const std::string& argName, size_t& index);

	std::vector<std::string> m_args;
	std::string m_queryImagePath;
	std::string m_inputDir;
	size_t m_numThreads = MIN_THREADS;
	size_t m_topK = MIN_SIM_IMAGES;
	double m_threshold = QUALITIY_BOUNDARY;
};