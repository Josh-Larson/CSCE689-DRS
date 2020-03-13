#include "StereoSegmentation.h"

#include <cmath>

StereoSegmentation calculateImageSizeForSegment(const cv::Rect2i& imageSize, const cv::Rect2i& segmentSize, int numDisparities, int blockSize) {
	fprintf(stdout, "%d,%d  %d,%d\n", segmentSize.x, segmentSize.y, segmentSize.width, segmentSize.height);
	assert(imageSize.width > 0 && imageSize.height > 0);
	assert(segmentSize.x >= 0 && segmentSize.y >= 0);
	assert(segmentSize.width > 0 && segmentSize.height > 0);
	assert((segmentSize.x + segmentSize.width) <= imageSize.width);
	assert((segmentSize.y + segmentSize.height) <= imageSize.height);
	assert(numDisparities > 0);
	assert(blockSize > 0);
	assert(blockSize % 2 == 1);
	auto halfBlockSize = (blockSize - 1) / 2 + 1;
	auto xStart = std::max(0, segmentSize.x - numDisparities - halfBlockSize);
	auto yStart = std::max(0, segmentSize.y - halfBlockSize);
	auto xEnd = std::min(imageSize.width, segmentSize.x + segmentSize.width + halfBlockSize);
	auto yEnd = std::min(imageSize.height, segmentSize.y + segmentSize.height + halfBlockSize);
	
	auto xStartOffset = segmentSize.x - xStart;
	auto yStartOffset = segmentSize.y - yStart;
	
	return StereoSegmentation{
			cv::Rect2i{xStart, yStart, xEnd - xStart, yEnd - yStart},
			cv::Rect2i{xStartOffset, yStartOffset, segmentSize.width, segmentSize.height}
	};
}
