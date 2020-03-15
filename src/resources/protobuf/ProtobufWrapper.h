#pragma once

#include <messages.pb.h>
#include <functional>

// TODO: Figure out how to provide the same API using templates

networking::protobuf::Packet wrap(networking::protobuf::MessageType type, bool response, const google::protobuf::Message & message);
networking::protobuf::Packet wrap(networking::protobuf::MessageType type, bool response, google::protobuf::Message && message);

networking::protobuf::Packet wrap(const networking::protobuf::InitializeConnectionMessage & message);
networking::protobuf::Packet wrap(networking::protobuf::InitializeConnectionMessage && message);

networking::protobuf::Packet wrap(const networking::protobuf::ListClusterMessage & message);
networking::protobuf::Packet wrap(networking::protobuf::ListClusterMessage && message);

networking::protobuf::Packet wrap(const networking::protobuf::CalculateDisparitiesMessage & message);
networking::protobuf::Packet wrap(networking::protobuf::CalculateDisparitiesMessage && message);

networking::protobuf::Packet wrap(const networking::protobuf::CalculateDisparitiesMessageResponse & message);
networking::protobuf::Packet wrap(networking::protobuf::CalculateDisparitiesMessageResponse && message);

template<networking::protobuf::MessageType TYPE, bool RESPONSE, typename UNWRAPPED>
inline void unwrap_if_generic(const networking::protobuf::Packet & packet, const std::function<void(UNWRAPPED &&)> & handler) {
	if (packet.type() == TYPE && packet.response() == RESPONSE) {
		UNWRAPPED message;
		message.ParseFromString(packet.data());
		handler(std::move(message));
	}
}

// TODO: Make this template array-based, so a chain of these could compile into a chain of compile-time-verified if statements
void unwrap_if(const networking::protobuf::Packet & packet, const std::function<void(networking::protobuf::InitializeConnectionMessage &&)> & handler);
void unwrap_if(const networking::protobuf::Packet & packet, const std::function<void(networking::protobuf::ListClusterMessage &&)> & handler);
void unwrap_if(const networking::protobuf::Packet & packet, const std::function<void(networking::protobuf::CalculateDisparitiesMessage &&)> & handler);
void unwrap_if(const networking::protobuf::Packet & packet, const std::function<void(networking::protobuf::CalculateDisparitiesMessageResponse &&)> & handler);
