#include "ScopedHandle.hpp"
#include <utility>

ScopedHandle::ScopedHandle(HANDLE handle)
	: m_handle(handle)
{
}

ScopedHandle::~ScopedHandle()
{
	reset();
}

ScopedHandle::ScopedHandle(ScopedHandle&& other) noexcept
	: m_handle(std::exchange(other.m_handle, nullptr))
{
}

ScopedHandle& ScopedHandle::operator=(ScopedHandle&& other) noexcept
{
	if (this != &other)
	{
		reset();
		m_handle = std::exchange(other.m_handle, nullptr);
	}

	return *this;
}

HANDLE ScopedHandle::get() const
{
	return m_handle;
}

void ScopedHandle::reset(HANDLE handle)
{
	if (m_handle && m_handle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_handle);
	}

	m_handle = handle;
}

ScopedHandle::operator HANDLE() const
{
	return m_handle;
}
