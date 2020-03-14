#include <resources/networking/connection.h>

  //for server
  connection::pointer connection::create(boost::asio::io_context& io_context)
  {
    return connection::pointer(new connection(io_context));
  }

  //for client
  connection::pointer connection::create(boost::asio::io_context& io_context, const char* host)
  {
    return connection::pointer(new connection(io_context, host));
  }

  connection::connection(boost::asio::io_context& io_context)
    : socket_(io_context)
  {
  }

  connection::connection(boost::asio::io_context& io_context, const char* host)
    : socket_(io_context) 
  {
    tcp::resolver resolver(io_context);
    tcp::resolver::results_type endpoints = resolver.resolve(host, "13");
    boost::asio::connect(this->socket_, endpoints);
  }

  tcp::socket& connection::socket()
  {
    return socket_;
  }

  void connection::start(){
    readAttempt();
  }

  void connection::sendMessage()
  {
    if(this->outgoingMessageQueue.empty()){
      return;
    }
    this->message_ = this->outgoingMessageQueue.front();
    this->outgoingMessageQueue.pop_front();

    boost::asio::async_write(socket_, boost::asio::buffer(message_),
        boost::bind(&connection::handle_conn, shared_from_this()));
  }

  void connection::readAttempt(){

      boost::array<uint8_t, 128> buf;

      this->socket_.async_read_some(boost::asio::buffer(buf), boost::bind(&connection::sendMessage, shared_from_this()));

      for(auto b : buf){
        this->incomingMessageQueue.emplace_back(b);
      }

  }

  void connection::handle_conn()
  { 

    start();
  }

  void connection::addMessageToQueue(std::string s){
    this->outgoingMessageQueue.emplace_back(s);
  }

  void connection::readMessageFromQueue(std::vector<uint8_t>& r){
    if(!this->incomingMessageQueue.empty()){
      r = this->incomingMessageQueue;
    }
  }
