#pragma once

#include <ctime>
#include <iostream>
#include <string>
#include <deque>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <resources/networking/cluster/ClusterEndpoint.h>
#include <resources/networking/cluster/ClusterManager.h>


class connection : public std::enable_shared_from_this<connection> {
	std::shared_ptr<networking::cluster::ClusterManager> clusterManager;
	std::shared_ptr<networking::cluster::ClusterEndpoint> clusterEndpoint;
	boost::asio::io_context& ioContext;
	boost::asio::ip::tcp::socket socket_;
	std::deque<std::vector<uint8_t>> outgoingMessageQueue;
	std::vector<uint8_t> inboundBuffer;
	
	public:
	static std::shared_ptr<connection> createOutboundConnection(boost::asio::io_context &io_context, const std::shared_ptr<networking::cluster::ClusterManager>& manager, const std::string& host, const std::string& port);
	
	connection(boost::asio::io_context& io_context, std::shared_ptr<networking::cluster::ClusterManager> manager);
	
	inline boost::asio::ip::tcp::socket& socket() { return socket_; }
	
	void start();
	void sendMessage(std::vector<uint8_t> && message);
	
	private:
	
	void write();
	void readAttempt();
};
