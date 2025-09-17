#ifndef FILEDESC_HPP
#define FILEDESC_HPP

#include <stdexcept>

class FileDesc
{
	constexpr static int InvalidDesc = -1;

public:
	FileDesc();
	explicit FileDesc(int desc);

	FileDesc(const FileDesc&) = delete;
	FileDesc& operator=(const FileDesc&) = delete;

	FileDesc(FileDesc&& other) noexcept;
	FileDesc& operator=(FileDesc&& rhs);
	~FileDesc();

	void Swap(FileDesc& other);
	bool IsOpen() const noexcept;

	void Close();
	void Open(const char* pathname, int flags);
	size_t Read(void* buffer, size_t lengh) const;
	size_t Write(const void* buffer, size_t lengh) const;

private:
	void EnsureOpen() const;

	int m_desc = InvalidDesc;
};

#endif // FILEDESC_HPP