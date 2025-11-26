#include "ConcreteCommands.h"
#include <iostream>

PutCommand::PutCommand(BTree& tree)
	: m_tree(tree)
{
}

void PutCommand::Execute()
{
	uint64_t key;
	std::string value;
	if (!(std::cin >> key >> value))
	{
		throw std::runtime_error("Invalid arguments. Use: PUT <key> <value>");
	}
	m_tree.Put(key, value);
}

GetCommand::GetCommand(BTree& tree)
	: m_tree(tree)
{
}

void GetCommand::Execute()
{
	uint64_t key;
	if (!(std::cin >> key))
	{
		throw std::runtime_error("Invalid arguments. Use: GET <key>");
	}
	auto result = m_tree.Get(key);
	if (result.has_value())
	{
		std::cout << result.value() << std::endl;
	}
	else
	{
		std::cout << "Not found" << std::endl;
	}
}

DelCommand::DelCommand(BTree& tree)
	: m_tree(tree)
{
}

void DelCommand::Execute()
{
	uint64_t key;
	if (!(std::cin >> key))
	{
		throw std::runtime_error("Invalid arguments. Use: DEL <key>");
	}
	m_tree.Remove(key);
}

StatsCommand::StatsCommand(const BTree& tree)
	: m_tree(tree)
{
	m_tree.PrintStats();
}

void StatsCommand::Execute()
{
	m_tree.PrintStats();
}

HelpCommand::HelpCommand(const Menu& menu)
	: m_menu(menu)
{
}

void HelpCommand::Execute()
{
	m_menu.ShowInstructions();
}

ExitCommand::ExitCommand(Menu& menu)
	: m_menu(menu)
{
}

void ExitCommand::Execute()
{
	m_menu.Exit();
}
