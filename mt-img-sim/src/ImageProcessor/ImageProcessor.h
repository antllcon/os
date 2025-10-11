#pragma once
#include <string>
#include <vector>

namespace ImageProcessor
{
using PixelData = std::vector<float>;
constexpr int TARGET_DIMENSION = 256;

struct MseImage
{
	std::string path;
	double mse;

	bool operator<(const MseImage& other) const
	{
		return mse < other.mse;
	}
};

PixelData PreprocessImage(const std::string& filePath);
double CalculateMse(const PixelData& target, const PixelData& candidate);
} // namespace ImageProcessor