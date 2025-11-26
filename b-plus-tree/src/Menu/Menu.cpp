#include "Menu.h"
#include <algorithm>
#include <iostream>

void Menu::AddItem(const std::string& shortcut, const std::string& description, std::unique_ptr<ICommand>&& command)
{
	m_items.emplace_back(shortcut, description, std::move(command));
}

void Menu::Run()
{
	std::string command;
	while (std::cout << "> " && std::cin >> command && ExecuteCommand(command))
	{
	}
}

void Menu::ShowInstructions() const
{
	std::cout << "Available commands: " << std::endl;
	for (const auto& item : m_items)
	{
		std::cout << "  " << item.shortcut << ": \t" << item.description << std::endl;
	}
}

void Menu::Exit()
{
	m_exit = true;
}

Menu::Item::Item(const std::string& shortcut, const std::string& description, std::unique_ptr<ICommand>&& command)
	: shortcut(shortcut)
	, description(description)
	, command(std::move(command))
{
}

bool Menu::ExecuteCommand(const std::string& command)
{
	m_exit = false;

	auto it = std::ranges::find_if(m_items, [&](const Item& item) {
		return item.shortcut == command;
	});

	if (it != m_items.end())
	{
		try
		{
			it->command->Execute();
		}
		catch (const std::exception& e)
		{
			std::cerr << "Error executing command '" << command << "': " << e.what() << std::endl;
		}
	}
	else
	{
		std::cout << "Unknown command. Use 'help'" << std::endl;
	}

	return !m_exit;
}
