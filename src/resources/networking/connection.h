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

using boost::asio::ip::tcp;

std::string make_daytime_string();

class connection
  : public boost::enable_shared_from_this<connection>
{
public:
  typedef boost::shared_ptr<connection> pointer;

  static pointer create(boost::asio::io_context& io_context);
  static pointer create(boost::asio::io_context& io_context, const char* host, const char* port);
  
  tcp::socket& socket();
  
  void start();
  void readMessageFromQueue(std::vector<uint8_t>& r);
  void sendMessage(std::string message);

private:
  connection(boost::asio::io_context& io_context);
  connection(boost::asio::io_context& io_context, const char* host, const char* port);

  
  void write();

  
  void readAttempt();

  void close();

  boost::asio::io_context& io_context_;
  tcp::socket socket_;
  std::string message_;
  std::vector<uint8_t> incomingMessageQueue;
  std::deque<std::string> outgoingMessageQueue;
};