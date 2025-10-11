#pragma once

#include <string>

class Image
{
public:
	explicit Image(const std::string& filePath);
	~Image();

	Image(const Image&) = delete;
	Image& operator=(const Image&) = delete;

	unsigned char* GetData() const;
	int GetWidth() const;
	int GetHeight() const;
	int GetChannels() const;

private:
	unsigned char* m_data = nullptr;
	int m_width = 0;
	int m_height = 0;
	int m_originalChannels = 0;
	int m_channels = 0;
};
