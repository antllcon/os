#include "ImageProcessor.h"

#include <cmath>
#include <memory>
#include <stdexcept>

namespace
{
// RAII-обертка для управления памятью, выделенной stbi_load
struct StbiDeleter
{
	void operator()(unsigned char* data) const
	{
		if (data) stbi_image_free(data);
	}
};
using StbiUniquePtr = std::unique_ptr<unsigned char, StbiDeleter>;

// sRGB -> linear RGB
void applyGamma(const unsigned char* srgbData, float* linearData, size_t numPixels)
{
	constexpr float gamma = 2.2f;
	for (size_t i = 0; i < numPixels * 3; ++i)
	{
		linearData[i] = powf(srgbData[i] / 255.0f, gamma);
	}
}
} // namespace

ImageProcessor::PixelData ImageProcessor::ProcessImage(const std::string& filePath)
{
	int width, height, channels;
	StbiUniquePtr loadedPixels(stbi_load(filePath.c_str(), &width, &height, &channels, 3)); // Принудительно 3 канала (RGB)

	if (!loadedPixels)
	{
		throw std::runtime_error("Failed to load image: " + filePath);
	}

	// Масштабирование до 256x256
	std::vector<unsigned char> resizedPixels(TARGET_DIMENSION * TARGET_DIMENSION * 3);
	stbir_resize_uint8(loadedPixels.get(), width, height, 0, resizedPixels.data(), TARGET_DIMENSION, TARGET_DIMENSION, 0, 3);

	// Гамма-коррекция и перевод в float
	PixelData finalPixels(TARGET_DIMENSION * TARGET_DIMENSION * 3);
	applyGamma(resizedPixels.data(), finalPixels.data(), TARGET_DIMENSION * TARGET_DIMENSION);

	return finalPixels;
}

double ImageProcessor::CalculateMse(const PixelData& target, const PixelData& candidate)
{
	if (target.size() != candidate.size() || target.empty())
	{
		return -1.0;
	}

	double squaredErrorSum = 0.0;
	for (size_t i = 0; i < target.size(); ++i)
	{
		const double diff = static_cast<double>(target[i]) - static_cast<double>(candidate[i]);
		squaredErrorSum += diff * diff;
	}

	// Формула: MSE = 1/(3*W*H) * SUM((Q-C)^2)
	// Размер вектора query.size() = 3 * W * H
	return squaredErrorSum / target.size();
}