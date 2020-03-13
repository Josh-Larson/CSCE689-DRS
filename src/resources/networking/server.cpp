#include <resources/networking/server.h>


  server::server(boost::asio::io_context& io_context)
    : io_context_(io_context),
      acceptor_(io_context, tcp::endpoint(tcp::v4(), 13))
  {
    start_accept();
  }


  void server::start_accept()
  {
    connection::pointer new_connection =
      connection::create(io_context_);

    acceptor_.async_accept(new_connection->socket(),
        boost::bind(&server::handle_accept, this, new_connection,
          boost::asio::placeholders::error));
  }

  void server::handle_accept(connection::pointer new_connection,
      const boost::system::error_code& error)
  {
    if (!error)
    {
      new_connection->start();
    }

    start_accept();
  }
