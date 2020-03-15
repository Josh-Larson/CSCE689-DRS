#include "ClusterManager.h"

#include "ClusterEndpoint.h"

namespace networking::cluster {

std::shared_ptr<ClusterEndpoint> ClusterManager::createEndpoint(const ClusterEndpointId& id, std::function<void(std::vector<uint8_t> &&)> sendMessage) {
	auto endpoint = std::make_shared<ClusterEndpoint>(id, shared_from_this(), std::move(sendMessage));
	endpoints[id].push_back(std::weak_ptr(endpoint));
	return endpoint;
}

}
