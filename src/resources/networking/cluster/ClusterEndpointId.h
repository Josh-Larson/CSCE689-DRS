#pragma once

#include <string>

namespace networking::cluster {

struct ClusterEndpointId {
	std::string ip;
	int         port;
	uint32_t    cpuParalleism;
	uint32_t    gpuParalleism;
	
	[[nodiscard]] bool operator==(const ClusterEndpointId &id) const noexcept { return ip == id.ip && port == id.port; }
	
	[[nodiscard]] bool operator==(ClusterEndpointId &&id) const noexcept { return ip == id.ip && port == id.port; }
	
	[[nodiscard]] bool operator<(const ClusterEndpointId &id) const noexcept { return ip < id.ip || (ip == id.ip && port < id.port); }
	
	[[nodiscard]] bool operator<(ClusterEndpointId &&id) const noexcept { return ip < id.ip || (ip == id.ip && port < id.port); }
};

} // namespace networking::cluster
