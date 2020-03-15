#include <cstdio>
#include <resources/protobuf/ProtobufStream.h>
#include <resources/protobuf/ProtobufWrapper.h>
#include <messages.pb.h>

#include <string>
#include <thread>

using namespace resources::protobuf;

int main(int argc, char * argv[]) {
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	Packet original;
	{
		ListClusterMessage message;
		Node n;
		n.set_ip("127.0.0.1");
		n.set_port(44463);
		n.set_self(false);
		n.set_cpuparallelism(std::thread::hardware_concurrency());
		n.set_gpuparallelism(0);
		message.mutable_nodes()->Add(std::move(n));
		original = wrap(message);
	}
	
	ProtobufStream stream;
	stream.addToReceiveBuffer(stream.encodePacket(original));
	Packet recreated = stream.getNextPacket();
	
	unwrap_if(recreated, [](ListClusterMessage && message) {
		fprintf(stdout, "ListClusterMessage:\n");
		int index = 0;
		for (const auto & node : message.nodes()) {
			fprintf(stdout, "    Node #%d\n", index);
			fprintf(stdout, "        IP:              %s\n", node.ip().c_str());
			fprintf(stdout, "        Port:            %d\n", node.port());
			fprintf(stdout, "        Self:            %s\n", node.self() ? "True" : "False");
			fprintf(stdout, "        CPU Parallelism: %d\n", node.cpuparallelism());
			fprintf(stdout, "        GPU Parallelism: %d\n", node.gpuparallelism());
			index++;
		}
	});
	
	return 0;
}
