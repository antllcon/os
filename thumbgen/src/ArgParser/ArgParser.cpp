#include "ArgParser.h"
#include <stdexcept>

namespace
{
void AssertMinArgsValid(const std::vector<std::string>& args)
{
	if (args.size() < 2)
	{
		throw std::invalid_argument("Недостаточно аргументов. Ожидается: thumbgen INPUT_DIR OUTPUT_DIR --size=WxH -j NUM_THREADS");
	}
}

void AssertIsNumberThreadsValid(size_t numThreads)
{
	if (numThreads < 1)
	{
		throw std::invalid_argument("Количество потоков должно быть не меньше 1");
	}
}
} // namespace

ArgParser::ArgParser(int argc, char* argv[])
	: m_args(argv + 1, argv + argc)
{
	AssertMinArgsValid(m_args);
}

void ArgParser::Parse()
{
	m_inputDir = m_args[0];
	m_outputDir = m_args[1];

	for (size_t i = 2; i < m_args.size(); ++i)
	{
		const std::string& arg = m_args[i];

		if (arg == "-j")
		{
			m_numThreads = std::stoul(GetValueFor(arg, i));
			AssertIsNumberThreadsValid(m_numThreads);
		}
		else if (arg == "--size")
		{
			ParseSize(GetValueFor(arg, i));
		}
		else
		{
			throw std::invalid_argument("Неизвестный аргумент: " + arg);
		}
	}

	if (m_thumbWidth == 0 || m_thumbHeight == 0)
	{
		throw std::invalid_argument("Аргумент --size=WxH является обязательным");
	}
}

void ArgParser::ParseSize(const std::string& sizeStr)
{
	size_t delimiterPos = sizeStr.find('x');
	if (delimiterPos == std::string::npos)
	{
		throw std::invalid_argument("Неверный формат --size");
	}

	std::string widthStr = sizeStr.substr(0, delimiterPos);
	std::string heightStr = sizeStr.substr(delimiterPos + 1);

	if (widthStr.empty() || heightStr.empty())
	{
		throw std::invalid_argument("Неверный формат --size. Ширина и высота не могут быть пустыми");
	}

	try
	{
		m_thumbWidth = std::stoi(widthStr);
		m_thumbHeight = std::stoi(heightStr);
	}
	catch (const std::exception& _)
	{
		throw std::invalid_argument("Не удалось распознать --size как числа: " + sizeStr);
	}

	if (m_thumbWidth <= 0 || m_thumbHeight <= 0)
	{
		throw std::invalid_argument("Размеры --size (WxH) должны быть положительными числами");
	}
}

const std::string& ArgParser::GetValueFor(const std::string& argName, size_t& index)
{
	if (index + 1 >= m_args.size())
	{
		throw std::invalid_argument("Для аргумента " + argName + " требуется значение");
	}

	return m_args[++index];
}

const std::string& ArgParser::GetInputDir() const
{
	return m_inputDir;
}

const std::string& ArgParser::GetOutputDir() const
{
	return m_outputDir;
}

size_t ArgParser::GetNumThreads() const
{
	return m_numThreads;
}

int ArgParser::GetThumbWidth() const
{
	return m_thumbWidth;
}

int ArgParser::GetThumbHeight() const
{
	return m_thumbHeight;
}