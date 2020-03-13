#pragma once

#include <opencv2/core.hpp>

namespace sbm {

class StereoBlockMatcher {
	
	public:
	virtual ~StereoBlockMatcher() = default;
	
	virtual void doSBM(const cv::Mat &leftImage, const cv::Mat &rightImage, cv::Mat &disparityMap) noexcept = 0;
	[[nodiscard]] virtual double getComplexity() const noexcept = 0;
	
	protected:
	StereoBlockMatcher() = default;
	
};

} // namespace sbm
