#pragma once

#include "StereoBlockMatcher.h"

namespace sbm {

class DirectStereoBlockMatcher : public StereoBlockMatcher {
	
	public:
	DirectStereoBlockMatcher() = default;
	~DirectStereoBlockMatcher() override = default;
	
	void doSBM(const cv::Mat &leftImage, const cv::Mat &rightImage, cv::Mat &disparityMap) noexcept override;
	[[nodiscard]] double getComplexity() const noexcept override;
	
};

} // namespace sbm
