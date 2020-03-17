#include "DirectStereoBlockMatcher.h"

#include <opencv2/opencv.hpp>

namespace sbm {

void DirectStereoBlockMatcher::doSBM(const cv::Mat &leftImage, const cv::Mat &rightImage, cv::Mat &disparityMap, int numDisparities, int blockSize) noexcept {
	stereo->setNumDisparities(numDisparities);
	stereo->setBlockSize(blockSize);
	stereo->compute(leftImage, rightImage, disparityMap);
}

double DirectStereoBlockMatcher::getComplexity() const noexcept {
	return 1000;
}

} // namespace sbm
