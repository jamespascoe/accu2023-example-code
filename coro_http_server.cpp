#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/config.hpp>

#include <iostream>
#include <memory>
#include <format>
#include <thread>
#include <vector>

namespace beast = boost::beast;
namespace http = beast::http;
namespace asio = boost::asio;
using tcp = boost::asio::ip::tcp;

// Report a failure
void
fail(beast::error_code ec, char const* what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}

// Handles an HTTP server connection
void
do_session(
    beast::tcp_stream& stream,
    asio::yield_context yield)
{
    beast::error_code ec;

    // This buffer is required to persist across reads
    beast::flat_buffer buffer;

    // This lambda is used to send messages
    for(;;)
    {
        // Set the timeout.
        stream.expires_after(std::chrono::seconds(30));

        // Read a request
        http::request<http::string_body> req;
        http::async_read(stream, buffer, req, yield[ec]);
        if(ec == http::error::end_of_stream)
            break;
        if(ec)
            return fail(ec, "read");

        // Handle the request
        auto handle_request = [&req]() -> http::message_generator {
          http::response<http::string_body> res{http::status::ok, req.version()};
          res.set(http::field::server, "Beast");
          res.body() = "Hello, ACCU 2023!";
          res.prepare_payload();
          res.keep_alive(req.keep_alive());
          return res;
        };

        http::message_generator msg = handle_request();

        // Determine if we should close the connection
        bool keep_alive = msg.keep_alive();

        // Send the response
        beast::async_write(stream, std::move(msg), yield[ec]);

        if(ec)
            return fail(ec, "write");

        if(! keep_alive)
        {
            // This means we should close the connection, usually because
            // the response indicated the "Connection: close" semantic.
            break;
        }
    }

    // Send a TCP shutdown
    stream.socket().shutdown(tcp::socket::shutdown_send, ec);

    // At this point the connection is closed gracefully
}

// Accepts incoming connections and launches the sessions
void do_listen(
    asio::io_context& ioc,
    tcp::endpoint endpoint,
    asio::yield_context yield)
{
    beast::error_code ec;

    // Open the acceptor
    tcp::acceptor acceptor(ioc);
    acceptor.open(endpoint.protocol(), ec);
    if(ec)
        return fail(ec, "open");

    // Allow address reuse
    acceptor.set_option(asio::socket_base::reuse_address(true), ec);
    if(ec)
        return fail(ec, "set_option");

    // Bind to the server address
    acceptor.bind(endpoint, ec);
    if(ec)
        return fail(ec, "bind");

    // Start listening for connections
    acceptor.listen(asio::socket_base::max_listen_connections, ec);
    if(ec)
        return fail(ec, "listen");

    for(;;)
    {
        tcp::socket socket(ioc);
        acceptor.async_accept(socket, yield[ec]);
        if(ec)
            fail(ec, "accept");
        else
            boost::asio::spawn(
                acceptor.get_executor(),
                std::bind(
                    do_session,
                    beast::tcp_stream(std::move(socket)),
                    std::placeholders::_1));
    }
}

int main(int argc, char *argv[])
{
  if (argc != 4) {
    std::cerr << std::format(
      "Usage: {} <ip-address> <port> <num_threads>\n"
      "E.g.: {} 0.0.0.0 8080 2\n",
      argv[0], argv[0]
    );
    return EXIT_FAILURE;
  }

  auto const address = asio::ip::make_address(argv[1]);
  auto const port = static_cast<unsigned short>(std::atoi(argv[2]));
  auto const num_threads = std::max<int>(1, std::atoi(argv[3]));

  asio::io_context ioc{num_threads};

  // Spawn a stackful coroutine
  boost::asio::spawn(ioc,
      std::bind(
          &do_listen,
          std::ref(ioc),
          tcp::endpoint{address, port},
          std::placeholders::_1));

  // Run the IO service with the requested number of threads
  std::vector<std::thread> v(num_threads-1);
  for (auto i = num_threads - 1; i; --i)
    v.emplace_back([&ioc]{ ioc.run(); });

  // Use the main thread as well
  ioc.run();

  return EXIT_SUCCESS;
}
