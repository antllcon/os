#include "ResultFilter.h"
#include <algorithm>

void ResultFilter::FilterResults(std::vector<ImageProcessor::MseImage>& allResults, const ArgParser& parser)
{
	std::sort(allResults.begin(), allResults.end());

	std::vector<ImageProcessor::MseImage> filteredResults;
	const double threshold = parser.GetThreshold();

	std::ranges::copy_if(allResults, std::back_inserter(filteredResults), [threshold](const ImageProcessor::MseImage& res) {
		return res.mse <= threshold;
	});

	if (allResults.size() > parser.GetTopK())
	{
		allResults.resize(parser.GetTopK());
	}
}