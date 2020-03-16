#include "ClusterManager.h"

#include "ClusterEndpoint.h"

namespace networking::cluster {

std::shared_ptr<ClusterEndpoint> ClusterManager::createEndpoint(const ClusterEndpointId& id, std::function<void(std::vector<uint8_t> &&)> sendMessage) {
	auto endpoint = std::make_shared<ClusterEndpoint>(id, shared_from_this(), std::move(sendMessage));
	endpoints[id].push_back(std::weak_ptr(endpoint));
	return endpoint;
}

std::shared_ptr<sbm::AggregatedStereoBlockMatcher> ClusterManager::getSBM() {
	return std::make_shared<sbm::AggregatedStereoBlockMatcher>(transformEndpoints<std::shared_ptr<sbm::StereoBlockMatcher>>([](const ClusterEndpointId & id, const std::vector<std::weak_ptr<ClusterEndpoint>> & endpointList) -> std::optional<std::shared_ptr<sbm::StereoBlockMatcher>> {
		for (const std::weak_ptr<ClusterEndpoint> & endpoint : endpointList) {
			std::shared_ptr<ClusterEndpoint> ret = endpoint.lock();
			if (ret != nullptr)
				return ret;
		}
		return std::nullopt;
	}));
}

std::vector<protobuf::Node> ClusterManager::getClusterNodes() const {
	return transformEndpoints<protobuf::Node>([](const ClusterEndpointId & id, const std::vector<std::weak_ptr<ClusterEndpoint>> & endpointList) -> std::optional<protobuf::Node> {
		for (const std::weak_ptr<ClusterEndpoint> & endpoint : endpointList) {
			std::shared_ptr<ClusterEndpoint> ret = endpoint.lock();
			if (ret != nullptr) {
				auto node = protobuf::Node();
				auto endpointId = ret->getId();
				node.set_ip(endpointId.ip);
				node.set_port(endpointId.port);
				node.set_cpuparallelism(endpointId.cpuParalleism);
				node.set_gpuparallelism(endpointId.gpuParalleism);
				return node;
			}
		}
		return std::nullopt;
	});
}

}
