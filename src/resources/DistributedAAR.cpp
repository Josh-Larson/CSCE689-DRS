#include "DistributedAAR.h"

#include <resources/disparity/DirectStereoBlockMatcher.h>

DistributedAAR::DistributedAAR(int serverPort, const std::string& remoteAddress, int remotePort) :
		serv(server::createServer(ioContext, serverPort, clusterManager)),
		localMatcher(std::make_shared<sbm::DirectStereoBlockMatcher>()) {
	if (!remoteAddress.empty()) {
		conn = connection::createOutboundConnection(ioContext, clusterManager, remoteAddress, std::to_string(remotePort));
	}
}

DistributedAAR::~DistributedAAR() {
	ioContext.stop();
	ioThread.join();
}

void DistributedAAR::run() {
	ioContext.run();
}

void DistributedAAR::doSBM(const cv::Mat &leftImage, const cv::Mat &rightImage, cv::Mat &disparityMap, int numDisparities, int blockSize) noexcept {
	sbm::AggregatedStereoBlockMatcher({clusterManager->getSBM(), localMatcher}).doSBM(leftImage, rightImage, disparityMap, numDisparities, blockSize);
}

double DistributedAAR::getComplexity() const noexcept {
	return sbm::AggregatedStereoBlockMatcher({clusterManager->getSBM(), localMatcher}).getComplexity();
}

void DistributedAAR::waitForServer() {
	ioThread.join();
}
