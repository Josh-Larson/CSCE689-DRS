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
	
	template<typename T>
	void addToReceiveBuffer(const T &buffer, size_t length) {
		receiveBuffer.insert(receiveBuffer.end(), buffer.begin(), buffer.begin() + length);
	}
	
	[[nodiscard]] bool isPacketReady() const noexcept {
		if (receiveBuffer.size() < 8)
			return false;
		const uint64_t length = *reinterpret_cast<const uint64_t *>(receiveBuffer.data());
		return receiveBuffer.size() >= length;
	}
	
	Packet getNextPacket() {
		assert(isPacketReady());
		const uint64_t length = *reinterpret_cast<const uint64_t *>(receiveBuffer.data());
		Packet ret;
		ret.ParseFromArray(receiveBuffer.data() + 8, length - 8);
		receiveBuffer.erase(receiveBuffer.begin(), receiveBuffer.begin() + length);
		return ret;
	}
	
	static std::vector<uint8_t> encodePacket(const Packet &packet) {
		std::vector<uint8_t> ret;
		const auto serialized = packet.SerializeAsString();
		ret.reserve(8 + serialized.length());
		ret.insert(ret.begin(), 8, 0);
		*reinterpret_cast<uint64_t *>(ret.data()) = serialized.length() + 8;
		std::copy(serialized.begin(), serialized.end(), std::back_inserter(ret));
		return ret;
	}
};

} // networking::protobuf
