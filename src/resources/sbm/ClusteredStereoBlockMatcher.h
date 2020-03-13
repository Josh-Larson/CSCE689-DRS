#pragma once

#include "AggregatedStereoBlockMatcher.h"

namespace sbm {

class ClusteredStereoBlockMatcher : public AggregatedStereoBlockMatcher {
	
	public:
	ClusteredStereoBlockMatcher() = default;
	~ClusteredStereoBlockMatcher() override = default;
	
	void doSBM(const cv::Mat &leftImage, const cv::Mat &rightImage, cv::Mat &disparityMap, int numDisparities, int blockSize) noexcept override;
	
};

} // namespace sbm
