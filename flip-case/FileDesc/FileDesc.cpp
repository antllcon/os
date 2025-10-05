#include "FileDesc.hpp"
#include <fcntl.h>
#include <system_error>
#include <unistd.h>
#include <utility>

FileDesc::FileDesc() = default;

FileDesc::FileDesc(int desc)
	: m_desc(desc == InvalidDesc || desc >= 0
			  ? desc
			  : throw std::invalid_argument("Invalid file descriptor"))
{
}


FileDesc::FileDesc(FileDesc&& other) noexcept
	: m_desc(std::exchange(other.m_desc, InvalidDesc))
{
}

FileDesc& FileDesc::operator=(FileDesc&& rhs)
{
	if (this != &rhs)
	{
		Swap(rhs);
		rhs.Close();
	}
	return *this;
}

FileDesc::~FileDesc()
{
	try
	{
		Close();
	}
	catch (...)
	{
	}
}

void FileDesc::Swap(FileDesc& other)
{
	std::swap(m_desc, other.m_desc);
}

bool FileDesc::IsOpen() const noexcept
{
	return m_desc != InvalidDesc;
}

void FileDesc::Close()
{
	if (IsOpen())
	{
		if (close(m_desc) != 0)
		{
			throw std::system_error(errno, std::generic_category());
		}
		m_desc = InvalidDesc;
	}
}

void FileDesc::Open(const char* pathname, const int flags)
{
	Close();
	if ((m_desc = open(pathname, flags)) == -1)
	{
		throw std::system_error(errno, std::generic_category());
	}
}

size_t FileDesc::Read(void* buffer, const size_t lengh) const
{
	EnsureOpen();
	if (const auto bytesRead = read(m_desc, buffer, lengh); bytesRead != -1)
	{
		return static_cast<size_t>(bytesRead);
	}
	throw std::system_error(errno, std::generic_category());
}

size_t FileDesc::Write(const void* buffer, const size_t lengh) const
{
	EnsureOpen();
	if (const auto bytesWritten = write(m_desc, buffer, lengh); bytesWritten != -1)
	{
		return static_cast<size_t>(bytesWritten);
	}
	throw std::system_error(errno, std::generic_category());
}

void FileDesc::EnsureOpen() const
{
	if (!IsOpen())
	{
		throw std::logic_error("File is not open");
	}
}