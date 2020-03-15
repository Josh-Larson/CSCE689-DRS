#pragma once

#include <messages.pb.h>
#include <functional>

resources::protobuf::Packet wrap(resources::protobuf::MessageType type, bool response, const google::protobuf::Message & message);
resources::protobuf::Packet wrap(resources::protobuf::MessageType type, bool response, google::protobuf::Message && message);

resources::protobuf::Packet wrap(const resources::protobuf::ListClusterMessage & message);
resources::protobuf::Packet wrap(resources::protobuf::ListClusterMessage && message);

resources::protobuf::Packet wrap(const resources::protobuf::CalculateDisparitiesMessage & message);
resources::protobuf::Packet wrap(resources::protobuf::CalculateDisparitiesMessage && message);

resources::protobuf::Packet wrap(const resources::protobuf::CalculateDisparitiesMessageReponse & message);
resources::protobuf::Packet wrap(resources::protobuf::CalculateDisparitiesMessageReponse && message);

template<resources::protobuf::MessageType TYPE, bool RESPONSE, typename UNWRAPPED>
inline void unwrap_if_generic(const resources::protobuf::Packet & packet, const std::function<void(UNWRAPPED &&)> & handler) {
	if (packet.type() == TYPE && packet.response() == RESPONSE) {
		UNWRAPPED message;
		message.ParseFromString(packet.data());
		handler(std::move(message));
	}
}

void unwrap_if(const resources::protobuf::Packet & packet, const std::function<void(resources::protobuf::ListClusterMessage &&)> & handler);
void unwrap_if(const resources::protobuf::Packet & packet, const std::function<void(resources::protobuf::CalculateDisparitiesMessage &&)> & handler);
void unwrap_if(const resources::protobuf::Packet & packet, const std::function<void(resources::protobuf::CalculateDisparitiesMessageReponse &&)> & handler);
