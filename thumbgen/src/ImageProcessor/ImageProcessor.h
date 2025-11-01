#pragma once

#include <string>

namespace ImageProcessor
{
void CreateThumbnail(const std::string& inputPath, const std::string& outputPath, int targetWidth, int targetHeight);
void ProcessTask(const std::string& inputPathStr, const std::string& inputDirStr, const std::string& outputDirStr, int targetWidth, int targetHeight);
} // namespace ImageProcessor