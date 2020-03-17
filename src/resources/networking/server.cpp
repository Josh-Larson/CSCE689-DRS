#include <resources/networking/server.h>

#include <utility>

using boost::asio::ip::tcp;

std::shared_ptr<server> server::createServer(boost::asio::io_context &io_context, int port, std::shared_ptr<networking::cluster::ClusterManager> manager) {
	auto ret = std::shared_ptr<server>(new server(io_context, port, std::move(manager)));
	ret->accept();
	return ret;
}

server::server(boost::asio::io_context &io_context, int port, std::shared_ptr<networking::cluster::ClusterManager> clusterMgr) :
		ioContext   (io_context),
		acceptor     (io_context, tcp::endpoint(tcp::v4(), port)),
		clusterManager(std::move(clusterMgr)) {
	
}

void server::accept() {
	auto newConnection = std::make_shared<connection>(ioContext, clusterManager);
	
	acceptor.async_accept(newConnection->socket(), [self=shared_from_this(), newConnection](const boost::system::error_code &error) {
		newConnection->start();
		self->accept();
	});
}
