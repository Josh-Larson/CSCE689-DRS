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


using boost::asio::ip::tcp;

class server
{
public:
  server(boost::asio::io_context& io_context, int port, std::shared_ptr<networking::cluster::ClusterManager> clusterMgr);

private:
  void start_accept();

  void handle_accept(connection::pointer new_connection,
      const boost::system::error_code& error);

  void readFromConns();

  boost::asio::io_context& io_context_;
  std::shared_ptr<networking::cluster::ClusterManager> clusterManager;
  tcp::acceptor acceptor_;
  std::vector<connection::pointer> connections;
};