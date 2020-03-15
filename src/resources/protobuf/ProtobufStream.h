#pragma once

#include <vector>
#include <messages.pb.h>

namespace networking::protobuf {

class ProtobufStream {
	std::vector<uint8_t> receiveBuffer;
	
	public:
	
	template<typename T>
	void addToReceiveBuffer(const T &buffer) {
		receiveBuffer.insert(receiveBuffer.end(), buffer.begin(), buffer.end());
	}
	
	[[nodiscard]] bool isPacketReady() const noexcept {
		if (receiveBuffer.size() < 8)
			return false;
		const uint64_t length = *reinterpret_cast<const uint64_t *>(receiveBuffer.data());
		return (receiveBuffer.size() + 8) >= length;
	}
	
	Packet getNextPacket() {
		assert(isPacketReady());
		const uint64_t length = *reinterpret_cast<const uint64_t *>(receiveBuffer.data());
		Packet ret;
		ret.ParseFromArray(receiveBuffer.data() + 8, length);
		receiveBuffer.erase(receiveBuffer.begin(), receiveBuffer.begin() + length + 8);
		return ret;
	}
	
	static std::vector<uint8_t> encodePacket(const Packet &packet) {
		std::vector<uint8_t> ret;
		const auto serialized = packet.SerializeAsString();
		ret.reserve(8 + serialized.length());
		ret.insert(ret.begin(), 8, 0);
		*reinterpret_cast<uint64_t *>(ret.data()) = serialized.length();
		std::copy(serialized.begin(), serialized.end(), std::back_inserter(ret));
		return ret;
	}
};

} // networking::protobuf
