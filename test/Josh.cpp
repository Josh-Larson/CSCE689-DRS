#include <cstdio>
#include <resources/protobuf/ProtobufStream.h>
#include <resources/protobuf/ProtobufWrapper.h>
#include <resources/networking/cluster/ClusterEndpoint.h>
#include <resources/networking/cluster/ClusterManager.h>
#include <messages.pb.h>

#include <string>
#include <thread>
#include <memory>
#include <queue>
#include <vector>

using namespace networking::protobuf;
using namespace networking::cluster;

int main(int argc, char * argv[]) {
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	
	auto manager = std::make_shared<ClusterManager>();
	std::queue<std::vector<uint8_t>> aInboundMessages;
	std::queue<std::vector<uint8_t>> bInboundMessages;
	auto a = manager->createEndpoint(ClusterEndpointId {"127.0.0.1", 5000, 0, 0}, [&](auto && msg) { bInboundMessages.emplace(msg); });
	auto b = manager->createEndpoint(ClusterEndpointId {"127.0.0.1", 5001, 0, 0}, [&](auto && msg) { aInboundMessages.emplace(msg); });
	
	while (!aInboundMessages.empty() && !bInboundMessages.empty()) {
		while (!aInboundMessages.empty()) {
			auto rx = aInboundMessages.front();
			aInboundMessages.pop();
			a->onDataReceived(rx);
		}
		while (!bInboundMessages.empty()) {
			auto rx = bInboundMessages.front();
			bInboundMessages.pop();
			b->onDataReceived(rx);
		}
	}
	return 0;
}
