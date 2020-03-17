#include "ClusterEndpoint.h"

#include "ClusterManager.h"
#include <resources/protobuf/ProtobufWrapper.h>

#include <utility>
#include <thread>
#include <condition_variable>
#include <resources/disparity/DirectStereoBlockMatcher.h>

namespace networking::cluster {

template<int pixelSize=1>
std::string getImageData(const cv::Mat& img) {
	// Code based on: https://stackoverflow.com/questions/26681713/convert-mat-to-array-vector-in-opencv
	std::string ret;
	if (img.isContinuous()) {
		ret.assign(img.data, img.data + img.total() * pixelSize);
	} else {
		for (int i = 0; i < img.rows; ++i) {
			ret.insert(ret.end(), img.ptr<char>(i), img.ptr<char>(i) + img.cols * pixelSize);
		}
	}
	return ret;
}

cv::Mat getImageFromData(const std::string & data, int width, int height, int type) {
	auto ret = cv::Mat(height, width, type);
	memcpy(ret.data, data.data(), data.length());
	return ret;
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
	constexpr auto weight = 1000 * 1.0;
	if (parallelism == 0)
		return weight;
	else
		return weight * parallelism;
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
	{
		auto imageData = getImageData<2>(disparity);
		protobuf::CalculateDisparitiesMessageResponse disparitiesMessage;
		disparitiesMessage.set_messageid(message.messageid());
		disparitiesMessage.set_width(disparity.cols);
		disparitiesMessage.set_height(disparity.rows);
		disparitiesMessage.set_disparity(imageData);
		sendMessage(protobuf::ProtobufStream::encodePacket(wrap(disparitiesMessage)));
	}
}

void ClusterEndpoint::onReceiveCalculateDisparitiesResponse(protobuf::CalculateDisparitiesMessageResponse &&message) {
	auto disparity = getImageFromData(message.disparity(), message.width(), message.height(), CV_16S);
	std::unique_lock lk(dataLock);
	
	const auto callbackIterator = disparityCallbacks.find(message.messageid());
	if (callbackIterator != disparityCallbacks.end()) {
		callbackIterator->second(std::move(disparity));
	}
}

} // namespace networking::cluster
