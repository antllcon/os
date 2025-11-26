#include "ConcreteCommands.h"
#include "Menu.h"
#include "src/BTree/BTree.h"

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

int main(int argc, char* argv[])
{
	std::cout << "Directory: " << std::filesystem::current_path() << std::endl;
	try
	{
		if (argc < 2)
		{
			std::cerr << "Use: btree.exe <file>" << std::endl;
			return EXIT_FAILURE;
		}

		std::filesystem::path dbPath = argv[1];

		BTree tree(dbPath);
		Menu menu;

		menu.AddItem("GET", "Get value by key (Usage: GET <key>)", std::make_unique<GetCommand>(tree));
		menu.AddItem("PUT", "Insert or update key-value (Usage: PUT <key> <value>)", std::make_unique<PutCommand>(tree));
		menu.AddItem("DEL", "Delete key (Usage: DEL <key>)", std::make_unique<DelCommand>(tree));
		menu.AddItem("STATS", "Show tree statistics", std::make_unique<StatsCommand>(tree));
		menu.AddItem("help", "Show this help", std::make_unique<HelpCommand>(menu));
		menu.AddItem("exit", "Exit program", std::make_unique<ExitCommand>(menu));
		std::cout << "Database opened. Type 'help' for commands" << std::endl;

		menu.Run();
	}
	catch (const std::exception& exception)
	{
		std::cerr << exception.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}