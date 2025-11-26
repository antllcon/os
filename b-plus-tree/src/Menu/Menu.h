#pragma once

#include <memory>
#include <string>
#include <vector>

class ICommand
{
public:
	virtual void Execute() = 0;
	virtual ~ICommand() = default;
};

class Menu
{
public:
	void AddItem(const std::string& shortcut, const std::string& description, std::unique_ptr<ICommand>&& command);
	void Run();
	void ShowInstructions() const;
	void Exit();

private:
	struct Item
	{
		Item(const std::string& shortcut, const std::string& description, std::unique_ptr<ICommand>&& command);
		std::string shortcut;
		std::string description;
		std::unique_ptr<ICommand> command;
	};

	bool ExecuteCommand(const std::string& command);

	std::vector<Item> m_items;
	bool m_exit = false;
};
