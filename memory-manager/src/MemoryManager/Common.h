#pragma once
#include <cstdint>
#include <stdexcept>

enum class Access
{
	Read,
	Write,
	Execute
};

enum class PageFaultReason
{
	NotPresent,
	WriteToReadOnly,
	ExecOnNX,
	UserAccessToSupervisor,
	PhysicalAccessOutOfRange,
	MisalignedAccess,
};

enum class Privilege
{
	User,
	Supervisor
};

struct PhysicalMemoryConfig
{
	uint32_t numFrames = 1024;
	uint32_t frameSize = 4096;
};

class MemoryException final : public std::runtime_error
{
public:
	using std::runtime_error::runtime_error;
};