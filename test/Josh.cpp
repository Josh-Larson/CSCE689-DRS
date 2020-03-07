#include <iostream>
#include <resources/disparity/StereoSegmentation.h>
#include <messages.pb.h>

int main(int argc, char * argv[]) {
	auto image = cv::Rect2i{0, 0, 2448, 1632};
	auto numDisparities = 256;
	auto blockSize = 21;
	for (int yInd = 0; yInd < 4; yInd++) {
		for (int xInd = 0; xInd < 4; xInd++) {
			auto imageSegment = cv::Rect2i{xInd*(image.width/4), yInd*(image.height/4), (image.width/4), (image.height/4)};
			auto segmentSize = calculateImageSizeForSegment(image, imageSegment, numDisparities, blockSize);
			
			fprintf(stdout, "%d-%d  %d-%d    %d-%d  %d-%d\n", imageSegment.x, imageSegment.width, imageSegment.y, imageSegment.height, segmentSize.disparityImageView.x, segmentSize.disparityImageView.width, segmentSize.disparityImageView.y, segmentSize.disparityImageView.height);
		}
	}
	return 0;
}