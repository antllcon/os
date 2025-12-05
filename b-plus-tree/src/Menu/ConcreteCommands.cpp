#include "ConcreteCommands.h"
#include <iostream>

PutCommand::PutCommand(BTree& tree)
	: m_tree(tree)
{
}

void PutCommand::Execute()
{
	auto key = IoUtils::Read<uint64_t>("Invalid key format. Expected <uint64>");
	auto value = IoUtils::Read<std::string>("Invalid value format. Expected <string>");

	m_tree.Put(key, value);
	std::cout << "OK" << std::endl;
}

GetCommand::GetCommand(BTree& tree)
	: m_tree(tree)
{
}

void GetCommand::Execute()
{
	auto key = IoUtils::Read<uint64_t>("Invalid key format. Expected <uint64>");

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
	auto key = IoUtils::Read<uint64_t>("Invalid key format. Expected <uint64>");

	if (m_tree.Remove(key))
	{
		std::cout << "Deleted" << std::endl;
	}
	else
	{
		std::cout << "Key not found" << std::endl;
	}
}

StatsCommand::StatsCommand(const BTree& tree)
	: m_tree(tree)
{
}

void StatsCommand::Execute()
{
	m_tree.PrintStats();
}

TreeCommand::TreeCommand(const BTree& tree)
	: m_tree(tree)
{
}

void TreeCommand::Execute()
{
	m_tree.PrintStructure();
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