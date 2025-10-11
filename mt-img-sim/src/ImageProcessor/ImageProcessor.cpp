#include "stb_image.h"
#include "stb_image_resize2.h"
#include "cmath"

#include "Image.h"
#include "ImageProcessor.h"

#include <stdexcept>

namespace
{
constexpr int TARGET_WIDTH = 256;
constexpr int TARGET_HEIGHT = 256;
constexpr int TARGET_CHANNELS = 3;
constexpr float GAMMA_VALUE = 2.2f;
constexpr double EQUAL_IMAGES_MSE = 0.0;

void AssertIsImagesValid(const ImageProcessor::PixelData& target, const ImageProcessor::PixelData& candidate)
{
	if (target.empty() || candidate.empty())
	{
		throw std::invalid_argument("Изображения не должны быть пустыми");
	}

	if (target.size() != candidate.size())
	{
		throw std::invalid_argument("Для вычисления MSE размеры изображений должны совпадать");
	}
}

std::vector<unsigned char> ConvertToRgb(const unsigned char* sourceData, int width, int height, int channels)
{
	if (channels == TARGET_CHANNELS)
	{
		const size_t dataSize = width * height * channels;
		return std::vector(sourceData, sourceData + dataSize);
	}

	std::vector<unsigned char> rgbData;
	rgbData.reserve(width * height * TARGET_CHANNELS);

	for (int i = 0; i < width * height; ++i)
	{
		switch (channels)
		{
		case 1:
			rgbData.emplace_back(sourceData[i]);
			rgbData.emplace_back(sourceData[i]);
			rgbData.emplace_back(sourceData[i]);
			break;
		case 4:
			rgbData.emplace_back(sourceData[i * 4 + 0]);
			rgbData.emplace_back(sourceData[i * 4 + 1]);
			rgbData.emplace_back(sourceData[i * 4 + 2]);
			break;
		default:
			throw std::runtime_error("Не поддерживается такое число каналов для перевода в RGB");
		}
	}

	return rgbData;
}

std::vector<float> ApplyGammaCorrection(const std::vector<unsigned char>& rgbData)
{
	std::vector<float> linearData;
	linearData.reserve(rgbData.size());

	for (unsigned char value : rgbData)
	{
		linearData.emplace_back(std::pow(value, GAMMA_VALUE));
	}

	return linearData;
}

std::vector<float> ResizeImage(const std::vector<float>& sourceData, int sourceWidth, int sourceHeight)
{
	std::vector<float> resizedData(TARGET_WIDTH * TARGET_HEIGHT * TARGET_CHANNELS);

	stbir_resize_float_linear(
		sourceData.data(),
		sourceWidth,
		sourceHeight,
		0,
		resizedData.data(),
		TARGET_WIDTH,
		TARGET_HEIGHT,
		0,
		STBIR_RGB);

	return resizedData;
}
} // namespace

ImageProcessor::PixelData ImageProcessor::PreprocessImage(const std::string& filePath)
{
	Image image(filePath);

	std::vector<unsigned char> rgbData = ConvertToRgb(
		image.GetData(),
		image.GetWidth(),
		image.GetHeight(),
		image.GetChannels());

	std::vector<float> linearData = ApplyGammaCorrection(rgbData);

	std::vector<float> finalData = ResizeImage(
		linearData,
		image.GetWidth(),
		image.GetHeight());

	return finalData;
}

double ImageProcessor::CalculateMse(const PixelData& target, const PixelData& candidate)
{
	AssertIsImagesValid(target, candidate);

	double sumSquaredError = EQUAL_IMAGES_MSE;
	const size_t numElements = target.size();

	for (size_t i = 0; i < numElements; ++i)
	{
		double diff = static_cast<double>(target[i]) - static_cast<double>(candidate[i]);
		sumSquaredError += diff * diff;
	}

	return sumSquaredError / numElements;
}