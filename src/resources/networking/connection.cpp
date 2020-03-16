#include <resources/networking/connection.h>

//for incoming
connection::pointer connection::create(boost::asio::io_context& io_context)
{
  return connection::pointer(new connection(io_context));
}

//for outgoin
connection::pointer connection::create(boost::asio::io_context& io_context, std::shared_ptr<networking::cluster::ClusterManager> manager, const char* host, const char* port)
{
  return connection::pointer(new connection(io_context, manager, host, port));
}

connection::connection(boost::asio::io_context& io_context)
  : socket_(io_context) , io_context_(io_context)
{
}

connection::connection(boost::asio::io_context& io_context, std::shared_ptr<networking::cluster::ClusterManager> manager, const char* host, const char* port)
  : socket_(io_context) , io_context_(io_context)
{
  tcp::resolver resolver(io_context);
  tcp::resolver::results_type endpoints = resolver.resolve(host, port);
	clusterEndpoint = manager->createEndpoint( {host, std::stoi(port)} , std::bind(&connection::sendMessage, this, std::placeholders::_1) );
//  boost::asio::steady_timer t(io_context);
//  t.expires_after(std::chrono::seconds(120));
//  t.async_wait([this](const boost::system::error_code& /*e*/)
//    {
//  	fprintf(stdout, "Timed out\n");
//      socket_.close();
//    });
  boost::asio::async_connect(this->socket_, endpoints, [this](const boost::system::error_code& ec,
    const tcp::endpoint& endpoint)
    {
      if (!ec)
      {
	      fprintf(stdout, "Successfully connected\n");
	      readAttempt();
      }
      else
      {
	      fprintf(stdout, "Failed to connect\n");
        socket_.close();
      }
    });
}

tcp::socket& connection::socket()
{
  return socket_;
}


void connection::start(){
  readAttempt();
}

void connection::sendMessage(std::vector<uint8_t>&& message)
{
  boost::asio::post(io_context_,
      [this, message]()
      {
        bool writeInProgress = !outgoingMessageQueue.empty();
        outgoingMessageQueue.push_back(message);
        if (!writeInProgress)
        {
          write();
        }
      });
}

void connection::readAttempt(){

	buf.clear();
  boost::asio::async_read(socket_, boost::asio::dynamic_buffer(buf), [this](boost::system::error_code ec, std::size_t /*length*/)
    {
      if (!ec)
      {
        clusterEndpoint->onDataReceived(buf);
        readAttempt();
      }
      else
      {
        socket_.close();
      }
    });
}


void connection::write()
{
  boost::asio::async_write(socket_, boost::asio::buffer(outgoingMessageQueue.front()),
    [this](boost::system::error_code ec, std::size_t /*length*/)
    {
      if (!ec)
      {
        outgoingMessageQueue.pop_front();
        if (!outgoingMessageQueue.empty())
        {
          write();
        }
      }
      else
      {
        socket_.close();
      }
    });
}

void connection::readMessageFromQueue(std::vector<uint8_t>& r){
  if(!this->incomingMessageQueue.empty()){
    r = this->incomingMessageQueue;
  }
}


void connection::close()
{
  boost::asio::post(io_context_, [this]() { socket_.close(); });
}