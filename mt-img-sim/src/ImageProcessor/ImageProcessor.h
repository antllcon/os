#pragma once
#include <string>
#include <vector>

namespace ImageProcessor
{
using PixelData = std::vector<float>;
constexpr int TARGET_DIMENSION = 256;

PixelData ProcessImage(const std::string& filePath);
double CalculateMse(const PixelData& target, const PixelData& candidate);
} // namespace ImageProcessor