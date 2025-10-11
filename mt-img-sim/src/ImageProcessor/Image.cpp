#include "Image.h"
#include "stb_image.h"

#include <stdexcept>

namespace
{
void AssertIsValidFileSource(const unsigned char* data)
{
	if (!data)
	{
		throw std::runtime_error("Ошибка загрузки изображения");
	}
}
} // namespace

Image::Image(const std::string& filePath)
{
	m_data = stbi_load(filePath.c_str(), &m_width, &m_height, &m_originalChannels, 0);
	AssertIsValidFileSource(m_data);
	m_channels = m_originalChannels;
}

Image::~Image()
{
	if (m_data)
	{
		stbi_image_free(m_data);
	}
}

unsigned char* Image::GetData() const
{
	return m_data;
}

int Image::GetWidth() const
{
	return m_width;
}

int Image::GetHeight() const
{
	return m_height;
}

int Image::GetChannels() const
{
	return m_channels;
}