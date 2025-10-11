#pragma once
#include <vector>
#include "ArgParser.h"
#include "ImageProcessor.h"

namespace ResultFilter
{
void FilterResults(std::vector<ImageProcessor::MseImage>& allResults, const ArgParser& parser);
} // namespace ResultFilter
