#include "ArgParser.h"
#include <iostream>
#include <stdexcept>

namespace
{
void AssertIsNumberArgsValid(const std::vector<std::string>& args)
{
	if (args.size() < 2)
	{
		throw std::invalid_argument("Не достаточно аргументов, дожно быть -> mt-img-sim QUERY_IMAGE INPUT_DIR -j NUM_THREADS --top K --threshold T");
	}
}

void AssertIsNumberThreadsValid(size_t numThreads)
{
	if (numThreads < 1)
	{
		throw std::invalid_argument("Количество потоков должно быть не меньше одного");
	}
}

} // namespace

ArgParser::ArgParser(int argc, char* argv[])
	: m_args(argv + 1, argv + argc)
{
	AssertIsNumberArgsValid(m_args);
}

void ArgParser::Parse()
{
	m_targetImagePath = m_args[0];
	m_inputDir = m_args[1];

	for (size_t i = 2; i < m_args.size(); ++i)
	{
		const std::string& arg = m_args[i];

		if (arg == "-j")
		{
			m_numThreads = std::stoul(GetValueFor(arg, i));
			AssertIsNumberThreadsValid(m_numThreads);
		}
		else if (arg == "--top")
		{
			m_topK = std::stoul(GetValueFor(arg, i));
		}
		else if (arg == "--threshold")
		{
			m_threshold = std::stod(GetValueFor(arg, i));
		}
		else
		{
			throw std::invalid_argument("Неизвестный аргумент: " + arg);
		}
	}
}

const std::string& ArgParser::GetTargetImagePath() const
{
	return m_targetImagePath;
}

const std::string& ArgParser::GetInputDir() const
{
	return m_inputDir;
}

size_t ArgParser::GetNumThreads() const
{
	return m_numThreads;
}

size_t ArgParser::GetTopK() const
{
	return m_topK;
}

double ArgParser::GetThreshold() const
{
	return m_threshold;
}

const std::string& ArgParser::GetValueFor(const std::string& argName, size_t& index)
{
	if (index >= m_args.size())
	{
		throw std::invalid_argument("Для аргумента " + argName + " требуется значение");
	}

	return m_args[++index];
}