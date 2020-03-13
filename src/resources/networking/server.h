#pragma once


#include <ctime>
#include <iostream>
#include <string>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <resources/networking/connection.h>

using boost::asio::ip::tcp;

class server
{
public:
  server(boost::asio::io_context& io_context);

private:
  void start_accept();

  void handle_accept(connection::pointer new_connection,
      const boost::system::error_code& error);

  boost::asio::io_context& io_context_;
  tcp::acceptor acceptor_;
};