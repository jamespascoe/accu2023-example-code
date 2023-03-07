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
      "Usage: {} <ip-address> <port>\n"
      "E.g.: {} 0.0.0.0 8080\n",
      argv[0], argv[0]
    );
    return EXIT_FAILURE;
  }

  try {
    asio::io_context ioc;

    // Create an acceptor
    auto const address = asio::ip::make_address(argv[1]);
    auto const port = static_cast<unsigned short>(std::atoi(argv[2]));

    tcp::acceptor acceptor{ioc, {address, port}};

    for(;;)
    {
      // Create a socket and block until we get a connection
      tcp::socket socket{ioc};
      acceptor.accept(socket);

      // A client has connected - declare a buffer
      beast::flat_buffer buffer;
      beast::error_code ec;

      for(;;)
      {
        // Read an HTTP request
        http::request<http::string_body> req;
        http::read(socket, buffer, req, ec);
        if(ec == http::error::end_of_stream)
          break; // Client disconnected - reset

        // Create a response
        auto handle_request = [&req]() -> http::message_generator {
          http::response<http::string_body> res{http::status::ok, req.version()};
          res.set(http::field::server, "Beast");
          res.body() = "Hello ACCU 2023 from Synchronous Server!";
          res.prepare_payload();
          return res;
        };

        // Send the response
        beast::write(socket, handle_request(), ec);
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
