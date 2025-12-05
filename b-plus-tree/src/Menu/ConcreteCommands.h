#pragma once
#include "BTree/BTree.h"
#include "Menu.h"
#include <iostream>

namespace IoUtils
{
template <typename T>
T Read(const std::string& errorMessage)
{
	T value;
	if (!(std::cin >> value))
	{
		throw std::runtime_error(errorMessage);
	}
	return value;
}
} // namespace IoUtils

class PutCommand : public ICommand
{
public:
	explicit PutCommand(BTree& tree);
	void Execute() override;

private:
	BTree& m_tree;
};

class GetCommand : public ICommand
{
public:
	explicit GetCommand(BTree& tree);
	void Execute() override;

private:
	BTree& m_tree;
};

class DelCommand : public ICommand
{
public:
	explicit DelCommand(BTree& tree);
	void Execute() override;

private:
	BTree& m_tree;
};

class StatsCommand : public ICommand
{
public:
	explicit StatsCommand(const BTree& tree);
	void Execute() override;

private:
	const BTree& m_tree;
};

class TreeCommand : public ICommand
{
public:
	explicit TreeCommand(const BTree& tree);
	void Execute() override;

private:
	const BTree& m_tree;
};

class HelpCommand : public ICommand
{
public:
	explicit HelpCommand(const Menu& menu);
	void Execute() override;

private:
	const Menu& m_menu;
};

class ExitCommand : public ICommand
{
public:
	explicit ExitCommand(Menu& menu);
	void Execute() override;

private:
	Menu& m_menu;
};