#include "AggregatedStereoBlockMatcher.h"

#include <resources/disparity/StereoSegmentation.h>

#include <mutex>
#include <condition_variable>

namespace sbm {

void AggregatedStereoBlockMatcher::doSBM(const cv::Mat &leftImage, const cv::Mat &rightImage, cv::Mat &disparityMap, int numDisparities, int blockSize) noexcept {
	assert(leftImage.rows == rightImage.rows && leftImage.cols == rightImage.cols);
	assert(rightImage.rows == disparityMap.rows && rightImage.cols == disparityMap.cols);
	
	std::vector<double> complexities;
	complexities.reserve(matchers.size());
	std::transform(matchers.begin(), matchers.end(), std::back_inserter(complexities), [](const auto & matcher) { return matcher->getComplexity(); });
	const auto complexitySum = std::accumulate(complexities.begin(), complexities.end(), 0.0);
	
	auto imageSize = cv::Rect2i{0, 0, leftImage.cols, leftImage.rows};
	int topBoundary = 0;
	int i = 0;
	std::mutex mutex;
	std::condition_variable cv;
	auto remaining = matchers.size();
	for (const auto & matcher : matchers) {
		const auto bottomBoundary = static_cast<int>(std::lround((std::accumulate(complexities.begin(), complexities.begin()+i+1, 0.0) / complexitySum) * leftImage.rows));
		
		const auto transfer = cv::Rect2i{0, topBoundary, imageSize.width, bottomBoundary - topBoundary};
		const auto segmentWindow = calculateImageSizeForSegment(imageSize, transfer, numDisparities, blockSize);
		
		threadPool.push([&, segmentWindow, transfer, numDisparities, blockSize](int id) {
			cv::Mat segment;
			matcher->doSBM(cv::Mat(leftImage, segmentWindow.originalImageView), cv::Mat(rightImage, segmentWindow.originalImageView), segment, numDisparities, blockSize);
			cv::Mat(segment, segmentWindow.disparityImageView).copyTo(cv::Mat(disparityMap, transfer));
			
			std::unique_lock lk(mutex);
			remaining--;
			cv.notify_one();
		});
		topBoundary = bottomBoundary;
		i++;
	}
	
	while (true) {
		std::unique_lock lk(mutex);
		if (remaining == 0)
			break;
		cv.wait(lk);
	}
}

} // namespace sbm
