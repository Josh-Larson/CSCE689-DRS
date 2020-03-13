#include "DirectStereoBlockMatcher.h"

namespace sbm {

void DirectStereoBlockMatcher::doSBM(const cv::Mat &leftImage, const cv::Mat &rightImage, cv::Mat &disparityMap, int numDisparities, int blockSize) noexcept {

}

double DirectStereoBlockMatcher::getComplexity() const noexcept {
	return 0;
}

} // namespace sbm
