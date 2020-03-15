#pragma once

#include "ClusterEndpointId.h"
#include <resources/protobuf/ProtobufStream.h>

#include <string>
#include <functional>
#include <memory>

namespace networking::cluster {

class ClusterManager;

class ClusterEndpoint {
	ClusterEndpointId                   id;
	std::shared_ptr<ClusterManager>     manager;
	std::function<void(std::vector<uint8_t> &&)> sendMessage;
	
	protobuf::ProtobufStream receiveStream      = {};
	
	public:
	ClusterEndpoint(ClusterEndpointId id, std::shared_ptr<ClusterManager> manager, std::function<void(std::vector<uint8_t> &&)> sendMessage);
	
	void onDataReceived(const std::vector<uint8_t> & data);
	
	private:
	void sendInitConnection() const;
	void sendClusterInfo() const;
	
	void onReceiveInitializeConnection(protobuf::InitializeConnectionMessage && message);
	void onReceiveListClusterMessage(protobuf::ListClusterMessage && message);
	void onReceiveCalculateDisparities(protobuf::CalculateDisparitiesMessage && message);
	void onReceiveCalculateDisparitiesResponse(protobuf::CalculateDisparitiesMessageResponse && message);
};

} // namespace networking::cluster
