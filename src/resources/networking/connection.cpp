#include <resources/networking/connection.h>

std::string make_daytime_string()
{
  using namespace std; // For time_t, time and ctime;
  time_t now = time(0);
  return ctime(&now);
}

  connection::pointer connection::create(boost::asio::io_context& io_context)
  {
    return connection::pointer(new connection(io_context));
  }

  tcp::socket& connection::socket()
  {
    return socket_;
  }

  void connection::start()
  {
    message_ = make_daytime_string();

    boost::asio::async_write(socket_, boost::asio::buffer(message_),
        boost::bind(&connection::handle_write, shared_from_this(),
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
  }


  connection::connection(boost::asio::io_context& io_context)
    : socket_(io_context)
  {
  }

  void connection::handle_write(const boost::system::error_code& /*error*/,
      size_t /*bytes_transferred*/)
  {
  }

