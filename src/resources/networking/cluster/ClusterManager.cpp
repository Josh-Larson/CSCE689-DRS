#include "ClusterManager.h"

#include "ClusterEndpoint.h"

namespace networking::cluster {

std::shared_ptr<ClusterEndpoint> ClusterManager::createEndpoint(const ClusterEndpointId& id, std::function<void(std::vector<uint8_t> &&)> sendMessage) {
	auto endpoint = std::make_shared<ClusterEndpoint>(id, shared_from_this(), std::move(sendMessage));
	endpoints[id].push_back(std::weak_ptr(endpoint));
	return endpoint;
}

sbm::AggregatedStereoBlockMatcher ClusterManager::getSBM() {
	return sbm::AggregatedStereoBlockMatcher(transformEndpoints<std::shared_ptr<sbm::StereoBlockMatcher>>([](const ClusterEndpointId & id, const std::vector<std::weak_ptr<ClusterEndpoint>> & endpointList) -> std::optional<std::shared_ptr<sbm::StereoBlockMatcher>> {
		for (const std::weak_ptr<ClusterEndpoint> & endpoint : endpointList) {
			std::shared_ptr<ClusterEndpoint> ret = endpoint.lock();
			if (ret != nullptr)
				return ret;
		}
		return std::nullopt;
	}));
}

}