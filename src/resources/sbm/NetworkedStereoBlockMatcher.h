#pragma once

#include "StereoBlockMatcher.h"

namespace sbm {

class NetworkedStereoBlockMatcher : public StereoBlockMatcher {
	
	public:
	NetworkedStereoBlockMatcher() = default;
	~NetworkedStereoBlockMatcher() override = default;
	
	void doSBM(const cv::Mat &leftImage, const cv::Mat &rightImage, cv::Mat &disparityMap, int numDisparities, int blockSize) noexcept override;
	[[nodiscard]] double getComplexity() const noexcept override;
	
};

} // namespace sbm
