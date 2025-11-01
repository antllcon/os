#include "ArgParser.h"
#include "DirectoryScanner.h"
#include "ImageProcessor.h"

#include <atomic>
#include <boost/asio/post.hpp>
#include <boost/asio/thread_pool.hpp>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;
const std::unordered_set<std::string> IMG_EXTENSIONS = {".png", ".jpg", ".jpeg"};

int main(int argc, char* argv[])
{
	auto startTime = std::chrono::high_resolution_clock::now();

	try
	{
		ArgParser parser(argc, argv);
		parser.Parse();

		auto files = DirectoryScanner::Scan(parser.GetInputDir(), IMG_EXTENSIONS);
		std::sort(files.begin(), files.end());

		boost::asio::thread_pool pool(parser.GetNumThreads());
		std::atomic<int> processedCount = 0;
		std::atomic<int> failedCount = 0;

		const int thumbW = parser.GetThumbWidth();
		const int thumbH = parser.GetThumbHeight();
		const std::string inputDirStr = parser.GetInputDir();
		const std::string outputDirStr = parser.GetOutputDir();

		for (const auto& filePathStr : files)
		{
			boost::asio::post(pool, [&, filePathStr] {
				try
				{
					ImageProcessor::ProcessTask(
						filePathStr,
						inputDirStr,
						outputDirStr,
						thumbW,
						thumbH);

					++processedCount;
				}
				catch (const std::exception& e)
				{
					std::cerr << "Ошибка при обработке файла " << filePathStr << ": " << e.what() << std::endl;
					++failedCount;
				}
			});
		}

		pool.join();

		std::cout << "Обработано = " << processedCount << std::endl;
		std::cout << "Ошибок = " << failedCount << std::endl;

		auto endTime = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
		std::cout << "Общее время: " << duration.count() << " мс" << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}