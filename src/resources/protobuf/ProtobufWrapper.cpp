#include "ProtobufWrapper.h"

using namespace networking::protobuf;
using namespace google::protobuf;

Packet wrap(MessageType type, bool response, const Message & message) {
	Packet original;
	original.set_type(type);
	original.set_response(response);
	original.set_data(std::move(message.SerializeAsString()));
	return original;
}

Packet wrap(MessageType type, bool response, Message && message) {
	Packet original;
	original.set_type(type);
	original.set_response(response);
	original.set_data(std::move(message.SerializeAsString()));
	return original;
}

Packet wrap(const InitializeConnectionMessage & message) { return wrap(MessageType::INIT_CONNECTION, false, message); }
Packet wrap(InitializeConnectionMessage && message) { return wrap(MessageType::INIT_CONNECTION, false, message); }

Packet wrap(const ListClusterMessage & message) { return wrap(MessageType::LIST_CLUSTER, false, message); }
Packet wrap(ListClusterMessage && message) { return wrap(MessageType::LIST_CLUSTER, false, message); }

Packet wrap(const CalculateDisparitiesMessage & message) { return wrap(MessageType::CALCULATE_DISPARITY, false, message); }
Packet wrap(CalculateDisparitiesMessage && message) { return wrap(MessageType::CALCULATE_DISPARITY, false, message); }

Packet wrap(const CalculateDisparitiesMessageResponse & message) { return wrap(MessageType::CALCULATE_DISPARITY, true, message); }
Packet wrap(CalculateDisparitiesMessageResponse && message) { return wrap(MessageType::CALCULATE_DISPARITY, true, message); }


void unwrap_if(const Packet & packet, const std::function<void(InitializeConnectionMessage &&)> & handler) {
	unwrap_if_generic<MessageType::INIT_CONNECTION, false, InitializeConnectionMessage>(packet, handler);
}

void unwrap_if(const Packet & packet, const std::function<void(ListClusterMessage &&)> & handler) {
	unwrap_if_generic<MessageType::LIST_CLUSTER, false, ListClusterMessage>(packet, handler);
}

void unwrap_if(const Packet & packet, const std::function<void(CalculateDisparitiesMessage &&)> & handler) {
	unwrap_if_generic<MessageType::CALCULATE_DISPARITY, false, CalculateDisparitiesMessage>(packet, handler);
}

void unwrap_if(const Packet & packet, const std::function<void(CalculateDisparitiesMessageResponse &&)> & handler) {
	unwrap_if_generic<MessageType::CALCULATE_DISPARITY, true, CalculateDisparitiesMessageResponse>(packet, handler);
}

