#pragma once

#include <string>
#include <utility>

namespace networking::cluster {

struct ClusterEndpointId {
	std::string ip;
	int         port;
	uint32_t    cpuParalleism = 0;
	uint32_t    gpuParalleism = 0;
	
	ClusterEndpointId(std::string ip, int port) : ip(std::move(ip)), port(port) {}
	
	[[nodiscard]] bool operator==(const ClusterEndpointId &id) const noexcept { return ip == id.ip && port == id.port; }
	
	[[nodiscard]] bool operator==(ClusterEndpointId &&id) const noexcept { return ip == id.ip && port == id.port; }
	
	[[nodiscard]] bool operator<(const ClusterEndpointId &id) const noexcept { return ip < id.ip || (ip == id.ip && port < id.port); }
	
	[[nodiscard]] bool operator<(ClusterEndpointId &&id) const noexcept { return ip < id.ip || (ip == id.ip && port < id.port); }
};

} // namespace networking::cluster
