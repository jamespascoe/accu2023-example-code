#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/config.hpp>

#include <iostream>
#include <format>

namespace beast = boost::beast;
namespace http = beast::http;
namespace asio = boost::asio;
using tcp = boost::asio::ip::tcp;

int main(int argc, char *argv[])
{
  if (argc != 3) {
    std::cerr << std::format(
      "Usage: {} <ip-address> <port>\nE.g.: {} 0.0.0.0 8080\n",
      argv[0], argv[0]
    );
    return EXIT_FAILURE;
  }

  try {
    auto const address = asio::ip::make_address(argv[1]);
    auto const port = static_cast<unsigned short>(std::atoi(argv[2]));

    asio::io_context ioc;

    // The acceptor receives incoming connections
    tcp::acceptor acceptor{ioc, {address, port}};
    for(;;)
    {
      // This will receive the new connection
      tcp::socket socket{ioc};

      // Block until we get a connection
      acceptor.accept(socket);

      // Launch the session, transferring ownership of the socket
      beast::error_code ec;

      // This buffer is required to persist across reads
      beast::flat_buffer buffer;

      for(;;)
      {
        // Read a request
        http::request<http::string_body> req;
        http::read(socket, buffer, req, ec);
        if(ec == http::error::end_of_stream)
          break;

        // Handle request
        http::response<http::string_body> res{http::status::ok, req.version()};
        res.set(http::field::server, "Beast");
        res.body() = "Hello, world!";
        res.prepare_payload();
        res.keep_alive(req.keep_alive());

        // Determine if we should close the connection
        bool keep_alive = res.keep_alive();

        auto handle_request = [&req]() -> http::message_generator {
          http::response<http::string_body> res{http::status::ok, req.version()};
          res.set(http::field::server, "Beast");
          res.body() = "Hello, ACCU 2023!";
          res.prepare_payload();
          res.keep_alive(req.keep_alive());
          return res;
        };

        // Send the response
        beast::write(socket, handle_request(), ec);

        if(! keep_alive)
        {
          // This means we should close the connection, usually because
          // the response indicated the "Connection: close" semantic.
          break;
        }
      }

      // Send a TCP shutdown
      socket.shutdown(tcp::socket::shutdown_send, ec);
   }
  }
  catch (const std::exception& e)
  {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
