#include <resources/networking/connection.h>

#include <utility>

using boost::asio::ip::tcp;
using boost::asio::buffer;
using boost::asio::mutable_buffer;
using boost::asio::const_buffer;

std::shared_ptr<connection> connection::createOutboundConnection(boost::asio::io_context &io_context, const std::shared_ptr<networking::cluster::ClusterManager>& manager, const std::string& host, const std::string& port) {
	auto resolver = tcp::resolver(io_context);
	auto endpoints = resolver.resolve(host, port);
	auto ret = std::make_shared<connection>(io_context, manager);
	
	boost::asio::async_connect(ret->socket(), endpoints, [self=ret](const boost::system::error_code &ec, const tcp::endpoint &endpoint) {
		if (!ec) {
			self->start();
		} else {
			self->socket_.close();
		}
	});
	
	return ret;
}

connection::connection(boost::asio::io_context &io_context, std::shared_ptr<networking::cluster::ClusterManager> manager) :
		socket_(io_context),
		ioContext(io_context),
		clusterManager(std::move(manager)) {
	inboundBuffer.resize(8 * 1024);
}

void connection::start() {
	clusterEndpoint = clusterManager->createEndpoint({socket().remote_endpoint().address().to_string(), 0}, std::bind(&connection::sendMessage, this, std::placeholders::_1));
	readAttempt();
}

void connection::sendMessage(std::vector<uint8_t> && message) {
	boost::asio::post(ioContext, [this, message]() {
		bool writeInProgress = !outgoingMessageQueue.empty();
		outgoingMessageQueue.push_back(message);
		if (!writeInProgress) {
			write();
		}
	});
}

void connection::readAttempt() {
	auto & buf = this->inboundBuffer;
	socket_.async_read_some(mutable_buffer(buf.data(), 8 * 1024), [&, self=shared_from_this()](boost::system::error_code error, std::size_t length) {
		if (!error) {
			self->clusterEndpoint->onDataReceived(buf, length);
			self->readAttempt();
		} else {
			self->socket_.close();
		}
	});
}

void connection::write() {
	boost::asio::async_write(socket_, const_buffer(buffer(outgoingMessageQueue.front())), [self=shared_from_this()](boost::system::error_code error, std::size_t length) {
		if (!error) {
			self->outgoingMessageQueue.pop_front();
			if (!self->outgoingMessageQueue.empty()) {
				self->write();
			}
		} else {
			self->socket_.close();
		}
	});
}
