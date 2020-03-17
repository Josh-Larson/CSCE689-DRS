#pragma once

#include <ctime>
#include <iostream>
#include <string>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <resources/networking/connection.h>
#include <resources/networking/cluster/ClusterEndpoint.h>
#include <resources/networking/cluster/ClusterManager.h>

class server : public std::enable_shared_from_this<server> {
	std::shared_ptr<networking::cluster::ClusterManager> clusterManager;
	boost::asio::io_context& ioContext;
	boost::asio::ip::tcp::acceptor acceptor;
	
	public:
	static std::shared_ptr<server> createServer(boost::asio::io_context &io_context, int port, std::shared_ptr<networking::cluster::ClusterManager> manager);
	~server();
	
	private:
	server(boost::asio::io_context &io_context, int port, std::shared_ptr<networking::cluster::ClusterManager> clusterMgr);
	
	void accept();
};
