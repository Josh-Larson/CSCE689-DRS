#pragma once

#include "ClusterEndpointId.h"
#include <resources/protobuf/ProtobufStream.h>
#include <resources/sbm/StereoBlockMatcher.h>

#include <opencv2/core.hpp>

#include <string>
#include <functional>
#include <memory>
#include <mutex>

namespace networking::cluster {

class ClusterManager;

class ClusterEndpoint : public sbm::StereoBlockMatcher {
	ClusterEndpointId                            id;
	std::shared_ptr<ClusterManager>              manager;
	std::function<void(std::vector<uint8_t> &&)> sendMessage;
	
	std::map<uint64_t, std::function<void(cv::Mat &&)>> disparityCallbacks = {};
	protobuf::ProtobufStream                            receiveStream      = {};
	std::atomic_uint64_t                                disparityMessageId = 1;
	std::mutex                                          dataLock           = {};
	
	public:
	ClusterEndpoint(ClusterEndpointId id, std::shared_ptr<ClusterManager> manager, std::function<void(std::vector<uint8_t> &&)> sendMessage);
	
	void onDataReceived(const std::vector<uint8_t> & data, size_t length);
	void doSBM(const cv::Mat &leftImage, const cv::Mat &rightImage, cv::Mat &disparityMap, int numDisparities, int blockSize) noexcept override;
	[[nodiscard]] double getComplexity() const noexcept override;
	
	[[nodiscard]] inline ClusterEndpointId getId() const noexcept { return id; }
	
	private:
	void sendInitConnection() const;
	void sendClusterInfo() const;
	
	void onReceiveInitializeConnection(protobuf::InitializeConnectionMessage && message);
	void onReceiveListClusterMessage(protobuf::ListClusterMessage && message);
	void onReceiveCalculateDisparities(protobuf::CalculateDisparitiesMessage && message);
	void onReceiveCalculateDisparitiesResponse(protobuf::CalculateDisparitiesMessageResponse && message);
};

} // namespace networking::cluster
