#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <resources/networking/server.h>
#include <resources/networking/connection.h>

constexpr uint16_t DEFAULT_PORT = 30781;

int main(int argc, char *argv[]) {
	auto serverPort = ((argc >= 2) ? uint16_t(std::stoi(argv[1])) : DEFAULT_PORT);
	auto clientPort = std::to_string(((argc >= 4) ? uint16_t(std::stoi(argv[3])) : DEFAULT_PORT));
	auto io = boost::asio::io_context();
	auto clusterManager = std::make_shared<networking::cluster::ClusterManager>();
	auto serv = server(io, serverPort, clusterManager);
	auto client = ((argc >= 3) ? connection::create(io, clusterManager, argv[2], clientPort.c_str()) : nullptr);
	io.run();
	return 0;
}
