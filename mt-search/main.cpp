#include "ScopeTimer.h"
#include <boost/asio/post.hpp>
#include <boost/asio/thread_pool.hpp>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

int main()
{
	try
	{
		ScopeTimer time;

		boost::asio::thread_pool pool(/* кол-во потоков */);
		for (const auto& /*1 данное */ : /* данные которые будем делить */)
		{
			boost::asio::post(pool, [&, /* аргументы */] {
				// Метод
			});
		}

		pool.join();
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	catch (...)
	{
		std::cerr << "Fatal error" << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}