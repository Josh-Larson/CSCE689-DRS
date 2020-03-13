#pragma once

#include <opencv2/core.hpp>

struct StereoSegmentation {
	cv::Rect2i originalImageView;
	cv::Rect2i disparityImageView;
};

StereoSegmentation calculateImageSizeForSegment(const cv::Rect2i& imageSize, const cv::Rect2i& segmentSize, int numDisparities, int blockSize);
