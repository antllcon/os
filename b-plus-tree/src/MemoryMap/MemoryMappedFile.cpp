#include "MemoryMappedFile.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdexcept>
#include <winbase.h>

namespace
{
std::string GetErrorMessage(DWORD code)
{
	char* messageBuffer = nullptr;
	DWORD len = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPSTR>(&messageBuffer), 0, nullptr);
	std::string message = (len && messageBuffer) ? std::string(messageBuffer, len) : "Unknown error";

	if (messageBuffer)
	{
		LocalFree(messageBuffer);
	}

	return message;
}
} // namespace

MemoryMappedFile::MemoryMappedFile(const std::filesystem::path& filePath, bool createNew)
	: m_fileHandle(INVALID_HANDLE_VALUE)
{
	OpenFile(filePath, createNew);

	if (m_size > 0)
	{
		MapFile();
	}
}

MemoryMappedFile::~MemoryMappedFile()
{
	Cleanup();
}

void* MemoryMappedFile::GetBaseAddress() const
{
	return m_baseAddress;
}

size_t MemoryMappedFile::GetSize() const
{
	return m_size;
}

void MemoryMappedFile::FlushToDisk()
{
	if (!m_baseAddress)
	{
		return;
	}

	if (!FlushViewOfFile(m_baseAddress, m_size))
	{
		throw std::runtime_error("FlushViewOfFile failed: " + GetErrorMessage(GetLastError()));
	}
}

void MemoryMappedFile::Resize(size_t newSize)
{
	if (m_baseAddress)
	{
		UnmapViewOfFile(m_baseAddress);
		m_baseAddress = nullptr;
	}

	if (m_mapHandle)
	{
		CloseHandle(m_mapHandle);
		m_mapHandle = nullptr;
	}

	HANDLE fileHandle = m_fileHandle;

	LARGE_INTEGER liSize;
	liSize.QuadPart = static_cast<LONGLONG>(newSize);

	if (!SetFilePointerEx(fileHandle, liSize, nullptr, FILE_BEGIN))
	{
		throw std::runtime_error("SetFilePointerEx failed: " + GetErrorMessage(GetLastError()));
	}

	if (!SetEndOfFile(fileHandle))
	{
		throw std::runtime_error("SetEndOfFile failed: " + GetErrorMessage(GetLastError()));
	}

	m_size = newSize;

	if (m_size > 0)
	{
		MapFile();
	}
}

// Private

void MemoryMappedFile::OpenFile(const std::filesystem::path& filePath, bool createNew)
{
	HANDLE fileHandle = CreateFileA(filePath.string().c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, nullptr, createNew ? CREATE_ALWAYS : OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (fileHandle == INVALID_HANDLE_VALUE)
	{
		throw std::runtime_error("CreateFile failed: " + GetErrorMessage(GetLastError()));
	}

	m_fileHandle = fileHandle;

	if (!createNew)
	{
		LARGE_INTEGER fileSize;

		if (!GetFileSizeEx(fileHandle, &fileSize))
		{
			CloseHandle(fileHandle);
			m_fileHandle = INVALID_HANDLE_VALUE;
			throw std::runtime_error("GetFileSizeEx failed: " + GetErrorMessage(GetLastError()));
		}

		m_size = fileSize.QuadPart;
	}
	else
	{
		m_size = 0;
	}
}

void MemoryMappedFile::MapFile()
{
	HANDLE fileHandle = m_fileHandle;
	HANDLE mapHandle = CreateFileMappingA(fileHandle, nullptr, PAGE_READWRITE, 0, 0, nullptr);

	if (!mapHandle)
	{
		throw std::runtime_error("CreateFileMappingA failed: " + GetErrorMessage(GetLastError()));
	}

	m_mapHandle = mapHandle;

	void* address = MapViewOfFile(mapHandle, FILE_MAP_ALL_ACCESS, 0, 0, m_size);

	if (!address)
	{
		throw std::runtime_error("MapViewOfFile failed: " + GetErrorMessage(GetLastError()));
	}

	m_baseAddress = address;
}

void MemoryMappedFile::Cleanup()
{
	if (m_baseAddress)
	{
		UnmapViewOfFile(m_baseAddress);
		m_baseAddress = nullptr;
	}

	if (m_mapHandle)
	{
		CloseHandle(m_mapHandle);
		m_mapHandle = nullptr;
	}

	if (m_fileHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_fileHandle);
		m_fileHandle = INVALID_HANDLE_VALUE;
	}
}