#ifndef SCOPEDHANDLE_HPP
#define SCOPEDHANDLE_HPP

#include <windows.h>

class ScopedHandle
{
public:
	explicit ScopedHandle(HANDLE handle = nullptr);
	~ScopedHandle();

	ScopedHandle(const ScopedHandle&) = delete;
	ScopedHandle& operator=(const ScopedHandle&) = delete;

	ScopedHandle(ScopedHandle&& other) noexcept;
	ScopedHandle& operator=(ScopedHandle&& other) noexcept;

	HANDLE get() const;
	void reset(HANDLE handle = nullptr);

	operator HANDLE() const;

private:
	HANDLE m_handle;
};

#endif // SCOPEDHANDLE_HPP
