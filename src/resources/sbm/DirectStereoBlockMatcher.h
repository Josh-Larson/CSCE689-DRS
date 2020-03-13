#pragma once

#include <opencv2/opencv.hpp>
#include "StereoBlockMatcher.h"

namespace sbm {

class DirectStereoBlockMatcher : public StereoBlockMatcher {
	cv::Ptr<cv::StereoBM> stereo;
	
	public:
	DirectStereoBlockMatcher() : stereo(cv::StereoBM::create()) {};
	~DirectStereoBlockMatcher() override = default;
	
	void doSBM(const cv::Mat &leftImage, const cv::Mat &rightImage, cv::Mat &disparityMap, int numDisparities, int blockSize) noexcept override;
	[[nodiscard]] double getComplexity() const noexcept override;
	
};

} // namespace sbm
