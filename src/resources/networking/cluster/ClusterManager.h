#pragma once

#include "ClusterEndpointId.h"

#include <resources/sbm/AggregatedStereoBlockMatcher.h>

#include <map>
#include <vector>
#include <memory>
#include <functional>
#include <optional>

namespace networking::cluster {

class ClusterEndpoint;

class ClusterManager : public std::enable_shared_from_this<ClusterManager> {
	std::map<ClusterEndpointId, std::vector<std::weak_ptr<ClusterEndpoint>>> endpoints;
	
	public:
	ClusterManager() = default;
	~ClusterManager() = default;
	
	std::shared_ptr<ClusterEndpoint> createEndpoint(const ClusterEndpointId& id, std::function<void(std::vector<uint8_t> &&)> sendMessage);
	
	sbm::AggregatedStereoBlockMatcher getSBM();
	
	private:
	template<typename T>
	std::vector<T> transformEndpoints(const std::function<std::optional<T>(const ClusterEndpointId &, const std::vector<std::weak_ptr<ClusterEndpoint>> &)> & transformer) {
		std::vector<T> ret;
		for (const auto & [id, endpointList] : endpoints) {
			auto transformed = transformer(id, endpointList);
			if (transformed)
				ret.emplace_back(std::move(*transformed));
		}
		return ret;
	}
};

} // namespace networking::cluster