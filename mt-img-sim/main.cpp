#include "ArgParser.h"
#include "DirectoryScanner.h"
#include "ImageProcessor.h"
#include "src/ResultFilter/ResultFilter.h"

#include <algorithm>
#include <boost/asio/post.hpp>
#include <boost/asio/thread_pool.hpp>
#include <filesystem>
#include <future>
#include <iomanip>
#include <iostream>

using Result = ImageProcessor::MseImage;
const std::unordered_set<std::string> IMG_EXTENSIONS = {".png", ".jpg", ".jpeg"};

struct ThreadData
{
	std::vector<Result> results;
	const ImageProcessor::PixelData* targetPixels;
	const std::vector<std::string>* candidateFiles;
};

std::vector<Result> Worker(const ImageProcessor::PixelData& targetPixels, const std::vector<std::string>& candidateFiles)
{
	std::vector<Result> localResults;
	localResults.reserve(candidateFiles.size());

	for (const auto& filePath : candidateFiles)
	{
		try
		{
			auto candidatePixels = ImageProcessor::PreprocessImage(filePath);
			auto mse = ImageProcessor::CalculateMse(targetPixels, candidatePixels);
			localResults.push_back({filePath, mse});
		}
		catch (const std::exception& e)
		{
			std::cerr << "Ошибка при обработке файла " << filePath << ": " << e.what() << std::endl;
		}
	}

	return localResults;
}

template <typename T>
std::vector<std::vector<T>> SplitIntoBlocks(const std::vector<T>& source, int numThreads)
{
	std::vector<std::vector<T>> blocks(numThreads);
	for (size_t i = 0; i < source.size(); ++i)
	{
		blocks[i % numThreads].emplace_back(source[i]);
	}

	return blocks;
}

int main(int argc, char* argv[])
{
	auto startTime = std::chrono::high_resolution_clock::now();

	try
	{
		ArgParser parser(argc, argv);
		parser.Parse();
		std::cout << "Спарсил аргументы командной строки" << std::endl;

		auto targetPixels = ImageProcessor::PreprocessImage(parser.GetTargetImagePath());
		std::cout << "Предобработка исходного изображения - " << parser.GetTargetImagePath() << std::endl;

		auto candidateFiles = DirectoryScanner::Scan(parser.GetInputDir(), IMG_EXTENSIONS);
		std::cout << "Просканирована директория - " << parser.GetInputDir() << std::endl;

		// // auto listsBlocks = SplitIntoBlocks(candidateFiles, parser.GetNumThreads());
		// for (const auto& block : listsBlocks)
		// {
		// 	for (const auto& filePath : block)
		// 	{
		// 		std::cout << "Файл - " << filePath << std::endl;
		// 	}
		// 	std::cout << "-----" << std::endl;
		// }
		//
		// std::vector<std::future<std::vector<Result>>> futures;
		//
		// for (int i = 0; i < listsBlocks.size(); ++i)
		// {
		// 	if (listsBlocks[i].empty())
		// 	{
		// 		continue;
		// 	}
		//
		// 	futures.emplace_back(std::async(
		// 	std::launch::async,
		// 	Worker,
		// 	std::cref(targetPixels),
		// 	std::cref(listsBlocks[i])));
		// }
		//
		// std::vector<Result> allResults;
		//
		// for (auto& future : futures)
		// {
		//
		// 	auto result = future.get();
		//
		// 	allResults.insert(allResults.end(),
		// 		std::make_move_iterator(result.begin()),
		// 		std::make_move_iterator(result.end()));
		// }

		std::vector<std::future<std::vector<Result>>> futures;

		boost::asio::thread_pool pool(parser.GetNumThreads());
		std::vector<Result> allResults;
		std::mutex resultsMutex;
		allResults.reserve(candidateFiles.size());

		for (const auto& filePath : candidateFiles)
		{
			boost::asio::post(pool, [&, filePath] {
				try
				{
					auto candidatePixels = ImageProcessor::PreprocessImage(filePath);
					auto mse = ImageProcessor::CalculateMse(targetPixels, candidatePixels);

					{
						std::lock_guard<std::mutex> lock(resultsMutex);
						allResults.push_back({filePath, mse});
					}
				}
				catch (const std::exception& e)
				{
					std::cerr << "Ошибка при обработке файла " << filePath << ": " << e.what() << std::endl;
				}
			});
		}

		pool.join();

		ResultFilter::FilterResults(allResults, parser);

		std::cout << std::fixed << std::setprecision(1);
		for (const auto& [path, mse] : allResults)
		{
			std::cout << mse << "  " << path << std::endl;
		}

		auto endTime = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

		std::cout << "Справка" << std::endl;
		std::cout << "Кол-во потоков: " << parser.GetNumThreads() << std::endl;
		std::cout << "Кол-во файлов:  " << candidateFiles.size() << std::endl;
		std::cout << "Общее время:    " << duration.count() << " мс" << std::endl;
	}
	catch (const std::invalid_argument& e)
	{
		std::cerr << "Argument error: " << e.what() << std::endl;
		return 1;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return 2;
	}

	return 0;
}