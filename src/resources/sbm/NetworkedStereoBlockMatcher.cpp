#include "NetworkedStereoBlockMatcher.h"

namespace sbm {

void NetworkedStereoBlockMatcher::doSBM(const cv::Mat &leftImage, const cv::Mat &rightImage, cv::Mat &disparityMap, int numDisparities, int blockSize) noexcept {

}

double NetworkedStereoBlockMatcher::getComplexity() const noexcept {
	return 0;
}

} // namespace sbm
