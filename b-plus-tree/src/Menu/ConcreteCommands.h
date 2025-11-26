#pragma once

#include "../BTree/BTree.h"
#include "Menu.h"

class PutCommand : public ICommand
{
public:
	PutCommand(BTree& tree);
	void Execute() override;

private:
	BTree& m_tree;
};

class GetCommand : public ICommand
{
public:
	GetCommand(BTree& tree);
	void Execute() override;

private:
	BTree& m_tree;
};

class DelCommand : public ICommand
{
public:
	DelCommand(BTree& tree);
	void Execute() override;

private:
	BTree& m_tree;
};

class StatsCommand : public ICommand
{
public:
	StatsCommand(const BTree& tree);
	void Execute() override;

private:
	const BTree& m_tree;
};

class HelpCommand : public ICommand
{
public:
	HelpCommand(const Menu& menu);
	void Execute() override;

private:
	const Menu& m_menu;
};

class ExitCommand : public ICommand
{
public:
	ExitCommand(Menu& menu);
	void Execute() override;

private:
	Menu& m_menu;
};
