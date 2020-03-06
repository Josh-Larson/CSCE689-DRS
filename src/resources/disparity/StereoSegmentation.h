#pragma once

#include <opencv2/core.hpp>
#include <cmath>

cv::Rect2i calculateImageSizeForSegment(const cv::Rect2i& imageSize, const cv::Rect2i& segmentSize, int numDisparities, int blockSize) {
	assert(imageSize.width > 0 && imageSize.height > 0);
	assert(segmentSize.x >= 0 && segmentSize.y >= 0);
	assert(segmentSize.width > 0 && segmentSize.height > 0);
	assert((segmentSize.x + segmentSize.width) <= imageSize.width);
	assert((segmentSize.y + segmentSize.height) <= imageSize.height);
	assert(numDisparities > 0);
	assert(blockSize > 0);
	assert(blockSize % 2 == 1);
	auto halfBlockSize = (blockSize - 1) / 2;
	auto xStart = std::max(0, segmentSize.x - numDisparities - halfBlockSize);
	auto yStart = std::max(0, segmentSize.y - halfBlockSize);
	
	auto xStartOffset = segmentSize.x - xStart;
	auto yStartOffset = segmentSize.y - yStart;
	
	return cv::Rect2i{xStartOffset, yStartOffset, segmentSize.width, segmentSize.height};
}
