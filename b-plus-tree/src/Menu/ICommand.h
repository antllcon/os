#pragma once

class ICommand
{
public:
	virtual ~ICommand() = default;
	virtual void Execute() = 0;

protected:
	ICommand() = default;
	ICommand(const ICommand&) = delete;
	ICommand& operator=(const ICommand&) = delete;
	ICommand(ICommand&&) = delete;
	ICommand& operator=(ICommand&&) = delete;
};