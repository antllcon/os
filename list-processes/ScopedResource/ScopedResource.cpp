#include "ScopedResource.hpp"
#include <utility>

ScopedResource::ScopedResource(const char* path)
	: m_res(opendir(path))
{
}

ScopedResource::~ScopedResource()
{
	closedir(m_res);
}

ScopedResource::ScopedResource(ScopedResource&& other) noexcept
	: m_res(std::exchange(other.m_res, nullptr))
{
}

ScopedResource& ScopedResource::operator=(ScopedResource&& other) noexcept
{
	if (this != &other)
	{
		reset();
		m_res = std::exchange(other.m_res, nullptr);
	}

	return *this;
}

DIR* ScopedResource::get() const
{
	return m_res;
}

void ScopedResource::reset(DIR* res)
{
	if (m_res && m_res != nullptr)
	{
		closedir(res);
	}

	m_res = res;
}
