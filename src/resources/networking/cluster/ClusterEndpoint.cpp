#include "ClusterEndpoint.h"

#include "ClusterManager.h"
#include <resources/protobuf/ProtobufWrapper.h>

#include <utility>
#include <thread>

namespace networking::cluster {

ClusterEndpoint::ClusterEndpoint(ClusterEndpointId id, std::shared_ptr<ClusterManager> manager, std::function<void(std::vector<uint8_t> &&)> sendMessage) :
		id(std::move(id)), manager(std::move(manager)), sendMessage(std::move(sendMessage)) {
	sendInitConnection();
}

void ClusterEndpoint::onDataReceived(const std::vector<uint8_t> &data) {
	receiveStream.addToReceiveBuffer(data);
	while (receiveStream.isPacketReady()) {
		protobuf::Packet packet = receiveStream.getNextPacket();
		unwrap_if(packet, std::bind(&ClusterEndpoint::onReceiveInitializeConnection, this, std::placeholders::_1));
		unwrap_if(packet, std::bind(&ClusterEndpoint::onReceiveListClusterMessage, this, std::placeholders::_1));
		unwrap_if(packet, std::bind(&ClusterEndpoint::onReceiveCalculateDisparities, this, std::placeholders::_1));
		unwrap_if(packet, std::bind(&ClusterEndpoint::onReceiveCalculateDisparitiesResponse, this, std::placeholders::_1));
	}
}

void ClusterEndpoint::sendInitConnection() const {
	protobuf::InitializeConnectionMessage message;
	message.set_cpuparallelism(std::thread::hardware_concurrency());
	message.set_gpuparallelism(0); // TODO: Figure out how to figure this out
	sendMessage(protobuf::ProtobufStream::encodePacket(wrap(message)));
}

void ClusterEndpoint::sendClusterInfo() const {
	protobuf::ListClusterMessage message;
	// TODO: Figure this out
	sendMessage(protobuf::ProtobufStream::encodePacket(wrap(message)));
}

void ClusterEndpoint::onReceiveInitializeConnection(protobuf::InitializeConnectionMessage &&message) {
	fprintf(stdout, "Received Initialize Connection:\n");
	fprintf(stdout, "    CPU: %d\n", message.cpuparallelism());
	fprintf(stdout, "    GPU: %d\n", message.gpuparallelism());
	sendClusterInfo();
}

void ClusterEndpoint::onReceiveListClusterMessage(protobuf::ListClusterMessage &&message) {
	// TODO: Do something with this
}

void ClusterEndpoint::onReceiveCalculateDisparities(protobuf::CalculateDisparitiesMessage &&message) {
	// TODO: SBM
}

void ClusterEndpoint::onReceiveCalculateDisparitiesResponse(protobuf::CalculateDisparitiesMessageResponse &&message) {

}

} // namespace networking::cluster
