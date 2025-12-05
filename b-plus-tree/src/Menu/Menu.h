#pragma once

#include "ICommand.h"
#include <memory>
#include <string>
#include <vector>

class Menu
{
public:
	void AddItem(const std::string& shortcut, const std::string& description, std::unique_ptr<ICommand> command);
	void ShowInstructions() const;
	void Run();
	void Exit();

private:
	struct Item
	{
		std::string shortcut;
		std::string description;
		std::unique_ptr<ICommand> command;
	};

	bool ExecuteCommand(const std::string& commandName);

	std::vector<Item> m_items;
	bool m_exit = false;
};