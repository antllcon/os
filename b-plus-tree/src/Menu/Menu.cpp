#include "Menu.h"
#include <algorithm>
#include <iostream>
#include <limits>

namespace
{
void RecoverInputStream()
{
	if (std::cin.fail())
	{
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}
}

void SkipLine()
{
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}
} // namespace

void Menu::AddItem(const std::string& shortcut, const std::string& description, std::unique_ptr<ICommand> command)
{
	auto it = std::ranges::find_if(m_items, [&](const Item& item) {
		return item.shortcut == shortcut;
	});

	if (it != m_items.end())
	{
		throw std::invalid_argument("Shortcut '" + shortcut + "' already exists");
	}

	m_items.emplace_back(Item{shortcut, description, std::move(command)});
}

void Menu::Run()
{
	ShowInstructions();

	std::string commandName;
	while (!m_exit)
	{
		std::cout << "> ";

		if (!(std::cin >> commandName))
		{
			break;
		}

		ExecuteCommand(commandName);
	}
}

void Menu::ShowInstructions() const
{
	std::cout << "Available commands:" << std::endl;
	for (const auto& item : m_items)
	{
		std::cout << "  " << item.shortcut << "\t: " << item.description << std::endl;
	}
}

void Menu::Exit()
{
	m_exit = true;
}

bool Menu::ExecuteCommand(const std::string& commandName)
{
	auto it = std::ranges::find_if(m_items, [&](const Item& item) {
		return item.shortcut == commandName;
	});

	if (it != m_items.end())
	{
		try
		{
			it->command->Execute();
		}
		catch (const std::exception& e)
		{
			std::cerr << "Error executing '" << commandName << "': " << e.what() << std::endl;

			RecoverInputStream();
		}
		return true;
	}

	std::cout << "Unknown command. Use 'help' to see available commands" << std::endl;

	SkipLine();

	return false;
}