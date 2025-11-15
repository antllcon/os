// #include <boost/asio/post.hpp>
// #include <boost/asio/thread_pool.hpp>
#include "Application.h"
#include "ScopeTimer.h"

#include <iostream>

namespace fs = std::filesystem;

int main()
{
	std::cout << "Current directory: " << std::filesystem::current_path() << std::endl;

	try
	{
		ScopeTimer time;
		Application application;
		application.Run();
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