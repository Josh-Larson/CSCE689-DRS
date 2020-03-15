#pragma once

#include "ClusterEndpointId.h"

#include <map>
#include <vector>
#include <memory>
#include <functional>

namespace networking::cluster {

class ClusterEndpoint;

class ClusterManager : public std::enable_shared_from_this<ClusterManager> {
	std::map<ClusterEndpointId, std::vector<std::weak_ptr<ClusterEndpoint>>> endpoints;
	
	public:
	ClusterManager() = default;
	~ClusterManager() = default;
	
	std::shared_ptr<ClusterEndpoint> createEndpoint(const ClusterEndpointId& id, std::function<void(std::vector<uint8_t> &&)> sendMessage);
	
	
	private:
	
};

} // namespace networking::cluster
