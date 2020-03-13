#pragma once

#include <ctime>
#include <iostream>
#include <string>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

std::string make_daytime_string();

class connection
  : public boost::enable_shared_from_this<connection>
{
public:
  typedef boost::shared_ptr<connection> pointer;

  static pointer create(boost::asio::io_context& io_context);
  
  tcp::socket& socket();
  
  void start();

private:
  connection(boost::asio::io_context& io_context);

  void handle_write(const boost::system::error_code& /*error*/,
      size_t /*bytes_transferred*/);

  tcp::socket socket_;
  std::string message_;
};