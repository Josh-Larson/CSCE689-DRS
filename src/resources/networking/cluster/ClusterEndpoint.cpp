#include "ClusterEndpoint.h"

#include "ClusterManager.h"
#include <resources/protobuf/ProtobufWrapper.h>

#include <utility>
#include <thread>
#include <condition_variable>
#include <resources/disparity/DirectStereoBlockMatcher.h>

namespace networking::cluster {

std::string getImageData(const cv::Mat& img) {
	std::vector<uint8_t> ret;
//	auto time1 = std::chrono::high_resolution_clock::now();
	cv::imencode(".png", img, ret);
//	auto time2 = std::chrono::high_resolution_clock::now();
//	fprintf(stdout, "Encoding time: %ld  Size: %ld\n", std::chrono::duration_cast<std::chrono::nanoseconds>(time2 - time1).count(), ret.size());
	return std::string(ret.begin(), ret.end());
}

cv::Mat getImageFromData(const std::string & data, int width, int height, int type) {
	return cv::imdecode(cv::Mat(std::vector<uint8_t>(data.begin(), data.end()), true), -1);
}

ClusterEndpoint::ClusterEndpoint(ClusterEndpointId id, std::shared_ptr<ClusterManager> manager, std::function<void(std::vector<uint8_t> &&)> sendMessage) :
		id(std::move(id)), manager(std::move(manager)), sendMessage(std::move(sendMessage)) {
	sendInitConnection();
}

void ClusterEndpoint::doSBM(const cv::Mat &leftImage, const cv::Mat &rightImage, cv::Mat &disparityMap, int numDisparities, int blockSize) noexcept {
	assert(leftImage.rows == rightImage.rows && leftImage.cols == rightImage.cols);
	
	auto messageId = disparityMessageId.fetch_add(1);
	std::mutex callbackWaitMutex;
	std::condition_variable callbackWait;
	{
		std::unique_lock lk(dataLock);
		disparityCallbacks[messageId] = [&](cv::Mat && mat) {
			disparityMap = std::move(mat);
			std::unique_lock callbackLock(callbackWaitMutex);
			callbackWait.notify_all();
		};
	}
	
	{
		protobuf::CalculateDisparitiesMessage message;
		message.set_messageid(messageId);
		message.set_width(leftImage.cols);
		message.set_height(leftImage.rows);
		message.set_numdisparities(numDisparities);
		message.set_blocksize(blockSize);
		message.set_leftimage(getImageData(leftImage));
		message.set_rightimage(getImageData(rightImage));
		sendMessage(protobuf::ProtobufStream::encodePacket(wrap(message)));
	}
	
	std::unique_lock callbackLock(callbackWaitMutex);
	callbackWait.wait(callbackLock);
}

double ClusterEndpoint::getComplexity() const noexcept {
	auto parallelism = id.cpuParalleism;
	if (parallelism == 0)
		return 1000.0;
	else
		return 1000.0 / parallelism;
}

void ClusterEndpoint::onDataReceived(const std::vector<uint8_t> &data, size_t length) {
	receiveStream.addToReceiveBuffer(data, length);
	while (receiveStream.isPacketReady()) {
		protobuf::Packet packet = receiveStream.getNextPacket();
		unwrap_if(packet, std::bind(&ClusterEndpoint::onReceiveInitializeConnection, this, std::placeholders::_1));
		unwrap_if(packet, std::bind(&ClusterEndpoint::onReceiveListClusterMessage, this, std::placeholders::_1));
		unwrap_if(packet, std::bind(&ClusterEndpoint::onReceiveCalculateDisparities, this, std::placeholders::_1));
		unwrap_if(packet, std::bind(&ClusterEndpoint::onReceiveCalculateDisparitiesResponse, this, std::placeholders::_1));
	}
}

void ClusterEndpoint::sendInitConnection() const {
	protobuf::InitializeConnectionMessage message;
	message.set_localserverport(manager->getLocalServerPort());
	message.set_cpuparallelism(std::thread::hardware_concurrency());
	message.set_gpuparallelism(0); // TODO: Figure out how to figure this out
	sendMessage(protobuf::ProtobufStream::encodePacket(wrap(message)));
}

void ClusterEndpoint::sendClusterInfo() const {
	protobuf::ListClusterMessage message;
	for (auto node : manager->getClusterNodes()) {
		node.set_self(node.ip() == id.ip && node.port() == id.port);
		message.mutable_nodes()->Add(std::move(node));
	}
	sendMessage(protobuf::ProtobufStream::encodePacket(wrap(message)));
}

void ClusterEndpoint::onReceiveInitializeConnection(protobuf::InitializeConnectionMessage &&message) {
	if (id.port == 0)
		id.port = message.localserverport();
	id.cpuParalleism = message.cpuparallelism();
	id.gpuParalleism = message.gpuparallelism();
	sendClusterInfo();
}

void ClusterEndpoint::onReceiveListClusterMessage(protobuf::ListClusterMessage &&message) {
	for (const auto & node : message.nodes()) {
		fprintf(stdout, "Cluster Node:\n");
		fprintf(stdout, "    IP:   %s\n", node.ip().c_str());
		fprintf(stdout, "    Port: %d\n", node.port());
		fprintf(stdout, "    Self: %s\n", node.self() ? "True" : "False");
		fprintf(stdout, "    CPU:  %d\n", node.cpuparallelism());
		fprintf(stdout, "    GPU:  %d\n", node.gpuparallelism());
	}
}

void ClusterEndpoint::onReceiveCalculateDisparities(protobuf::CalculateDisparitiesMessage &&message) {
	auto leftImage = getImageFromData(message.leftimage(), message.width(), message.height(), CV_8U);
	auto rightImage = getImageFromData(message.rightimage(), message.width(), message.height(), CV_8U);
	cv::Mat disparity, disparityOutput;
	blockMatcher.doSBM(leftImage, rightImage, disparity, message.numdisparities(), message.blocksize());
	disparity.convertTo(disparityOutput, CV_16U);
	disparity = disparityOutput;
	{
		auto imageData = getImageData(disparity);
		protobuf::CalculateDisparitiesMessageResponse disparitiesMessage;
		disparitiesMessage.set_messageid(message.messageid());
		disparitiesMessage.set_width(disparity.cols);
		disparitiesMessage.set_height(disparity.rows);
		disparitiesMessage.set_disparity(imageData);
		sendMessage(protobuf::ProtobufStream::encodePacket(wrap(disparitiesMessage)));
	}
}

void ClusterEndpoint::onReceiveCalculateDisparitiesResponse(protobuf::CalculateDisparitiesMessageResponse &&message) {
	auto disparity = getImageFromData(message.disparity(), message.width(), message.height(), CV_16U);
	cv::Mat disparityOutput;
	disparity.convertTo(disparityOutput, CV_16S);
	std::unique_lock lk(dataLock);
	
	const auto callbackIterator = disparityCallbacks.find(message.messageid());
	if (callbackIterator != disparityCallbacks.end()) {
		callbackIterator->second(std::move(disparityOutput));
	}
}

} // namespace networking::cluster
