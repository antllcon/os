#include "cmath"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "Image.h"
#include "ImageProcessor.h"

#include <filesystem>
#include <stdexcept>
#include <vector>

namespace fs = std::filesystem;

namespace
{
constexpr int TARGET_CHANNELS = 3;
constexpr float GAMMA_VALUE = 2.2f;
const float INVERSE_GAMMA_VALUE = 1.0f / GAMMA_VALUE;

std::vector<unsigned char> ConvertToRgb(
	const unsigned char* sourceData,
	int width,
	int height,
	int channels)
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

std::vector<unsigned char> ApplyInverseGamma(const std::vector<float>& linearData)
{
	std::vector<unsigned char> srgbData;
	srgbData.reserve(linearData.size());

	for (float linearValue : linearData)
	{
		// Обратная операция: pow(float, 1.0 / 2.2)
		float srgbValue = std::pow(linearValue, INVERSE_GAMMA_VALUE);

		// 1. Клиппинг (ограничение) значений в диапазоне [0, 255]
		if (srgbValue > 255.0f) srgbValue = 255.0f;
		if (srgbValue < 0.0f) srgbValue = 0.0f;

		// 2. Округление (+0.5f) и каст к unsigned char
		srgbData.emplace_back(static_cast<unsigned char>(srgbValue + 0.5f));
	}
	return srgbData;
}

// Вспомогательная функция для "зажима" значения в диапазоне [min, max]
// Нужна, чтобы не выйти за границы исходного изображения при чтении пикселей
template <typename T>
T clamp(T value, T min, T max)
{
	return std::max(min, std::min(value, max));
}

std::vector<float> ResizeImageBilinear(
	const std::vector<float>& sourceData,
	int sourceWidth,
	int sourceHeight,
	int targetWidth,
	int targetHeight)
{
	std::vector<float> resizedData(targetWidth * targetHeight * TARGET_CHANNELS);

	const float scaleX = static_cast<float>(sourceWidth) / targetWidth;
	const float scaleY = static_cast<float>(sourceHeight) / targetHeight;

	for (int y = 0; y < targetHeight; ++y)
	{
		for (int x = 0; x < targetWidth; ++x)
		{
			// Находим проекцию (u,v) в исходном изображении
			// u = x * Wsrc / Wdst
			float u = (x + 0.5f) * scaleX - 0.5f;
			float v = (y + 0.5f) * scaleY - 0.5f;

			// Находим 4 опорных пикселя (u0,v0), (u1,v0), (u0,v1), (u1,v1)
			int u0 = static_cast<int>(std::floor(u));
			int v0 = static_cast<int>(std::floor(v));
			int u1 = u0 + 1;
			int v1 = v0 + 1;

			// Находим веса (альфа, бета)
			float alpha = u - u0;
			float beta = v - v0;

			// Защита от выхода за границы (Clamping)
			// Опорные пиксели не должны выйти за пределы [0, W-1] и [0, H-1]
			u0 = clamp(u0, 0, sourceWidth - 1);
			u1 = clamp(u1, 0, sourceWidth - 1);
			v0 = clamp(v0, 0, sourceHeight - 1);
			v1 = clamp(v1, 0, sourceHeight - 1);

			// Интерполяция (для каждого канала R, G, B)
			const int targetIndex = (y * targetWidth + x) * TARGET_CHANNELS;

			for (int c = 0; c < TARGET_CHANNELS; ++c)
			{
				// Получаем индексы 4-х опорных пикселей в 1D-массиве
				const int indexC00 = (v0 * sourceWidth + u0) * TARGET_CHANNELS + c;
				const int indexC10 = (v0 * sourceWidth + u1) * TARGET_CHANNELS + c;
				const int indexC01 = (v1 * sourceWidth + u0) * TARGET_CHANNELS + c;
				const int indexC11 = (v1 * sourceWidth + u1) * TARGET_CHANNELS + c;

				// Получаем значения цвета (float) в этих 4-х точках
				const float C00 = sourceData[indexC00];
				const float C10 = sourceData[indexC10];
				const float C01 = sourceData[indexC01];
				const float C11 = sourceData[indexC11];

				// Линейная интерполяция по X (сначала C0, потом C1)
				float c_inter_v0 = (1.0f - alpha) * C00 + alpha * C10;
				float c_inter_v1 = (1.0f - alpha) * C01 + alpha * C11;

				// Линейная интерполяция по Y (между C0 и C1)
				float final_c = (1.0f - beta) * c_inter_v0 + beta * c_inter_v1;

				resizedData[targetIndex + c] = final_c;
			}
		}
	}

	return resizedData;
}
} // namespace

void ImageProcessor::CreateThumbnail(const std::string& inputPath, const std::string& outputPath, int targetWidth, int targetHeight)
{
	Image image(inputPath);

	std::vector<unsigned char> rgbData = ConvertToRgb(
		image.GetData(),
		image.GetWidth(),
		image.GetHeight(),
		image.GetChannels());

	std::vector<float> linearData = ApplyGammaCorrection(rgbData);

	std::vector<float> resizedData = ResizeImageBilinear(
		linearData,
		image.GetWidth(),
		image.GetHeight(),
		targetWidth,
		targetHeight);

	std::vector<unsigned char> finalSrgbData = ApplyInverseGamma(resizedData);

	const int strideInBytes = targetWidth * TARGET_CHANNELS;

	int result = stbi_write_png(
		outputPath.c_str(),
		targetWidth,
		targetHeight,
		TARGET_CHANNELS,
		finalSrgbData.data(),
		strideInBytes);

	if (result == 0)
	{
		throw std::runtime_error("Не удалось сохранить PNG файл: " + outputPath);
	}
}

void ImageProcessor::ProcessTask(
	const std::string& inputPathStr,
	const std::string& inputDirStr,
	const std::string& outputDirStr,
	int targetWidth,
	int targetHeight)
{
	fs::path inputPath(inputPathStr);
	fs::path inputDir(inputDirStr);
	fs::path outputDir(outputDirStr);

	fs::path relativePath = fs::relative(inputPath.parent_path(), inputDir);
	fs::path outParentPath = outputDir / relativePath;

	std::string newStem = inputPath.stem().string() + "_thumb";
	fs::path outputPath = outParentPath / newStem;
	outputPath.replace_extension(".png");

	fs::create_directories(outParentPath);

	CreateThumbnail(
		inputPathStr,
		outputPath.string(),
		targetWidth,
		targetHeight);
}