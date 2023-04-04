#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/use_awaitable.hpp>

#include <iostream>
#include <thread>
#include <vector>
#include <format>

namespace beast = boost::beast;
namespace http = beast::http;
namespace asio = boost::asio;
using tcp = boost::asio::ip::tcp;

using tcp_stream = typename beast::tcp_stream::rebind_executor<
  asio::use_awaitable_t<>::
    executor_with_default<asio::any_io_executor>>::other;

// Handles an HTTP server connection
asio::awaitable<void> do_session(tcp_stream stream)
{
  beast::error_code ec;

  // This buffer is required to persist across reads
  beast::flat_buffer buffer;

  for(;;) {
    try
    {
      // Set the timeout.
      stream.expires_after(std::chrono::seconds(30));

      // Read a request
      http::request<http::string_body> req;
      co_await http::async_read(stream, buffer, req);

      // Handle the request
      auto handle_request = [&req]() -> http::message_generator {
        http::response<http::string_body> res{
          http::status::ok,
            req.version()
        };
        res.set(http::field::server, "Beast");
        res.body() = "Hello ACCU 2023 from the Awaitable Server!";
        res.prepare_payload();
        res.keep_alive(req.keep_alive());
        return res;
      };

      // Send response
      co_await beast::async_write(
        stream, handle_request(), asio::use_awaitable
      );

      // Determine if we should close the connection
      if(!req.keep_alive()) break;
    }
    catch (boost::system::system_error & se)
    {
      if (se.code() != http::error::end_of_stream)
        throw;
    }
  }

  // Send a TCP shutdown
  stream.socket().shutdown(tcp::socket::shutdown_send, ec);
}

// Accepts incoming connections and launches the sessions
asio::awaitable<void> do_listen(tcp::endpoint endpoint)
{
  // Open the acceptor
  auto acceptor = asio::use_awaitable.as_default_on(
    tcp::acceptor(co_await asio::this_coro::executor)
  );
  acceptor.open(endpoint.protocol());

  // Allow address reuse
  acceptor.set_option(asio::socket_base::reuse_address(true));

  // Bind to the server address
  acceptor.bind(endpoint);

  // Start listening for connections
  acceptor.listen(asio::socket_base::max_listen_connections);

  for(;;)
    boost::asio::co_spawn(
      acceptor.get_executor(),
      do_session(tcp_stream(co_await acceptor.async_accept())),
      [](std::exception_ptr e)
      {
        try
        {
          if (e) std::rethrow_exception(e);
        }
        catch (std::exception &e) {
          std::cerr << "Error in session: " << e.what() << "\n";
        }
      }
    );
}

int main(int argc, char* argv[])
{
  // Check command line arguments.
  if (argc != 4) {
    std::cerr << std::format(
      "Usage: {} <ip-address> <port> <threads>\n"
      "E.g.: {} 0.0.0.0 8080 1\n",
      argv[0], argv[0]
    );
    return EXIT_FAILURE;
  }

  auto const address = asio::ip::make_address(argv[1]);
  auto const port = static_cast<unsigned short>(std::atoi(argv[2]));
  auto const num_threads = std::max<int>(1, std::atoi(argv[3]));

  // The io_context is required for all I/O
  asio::io_context ioc{num_threads};

  // Spawn a listening port
  boost::asio::co_spawn(ioc,
    do_listen(tcp::endpoint{address, port}),
    [](std::exception_ptr e)
    {
      try
      {
        if (e) std::rethrow_exception(e);
      }
      catch(std::exception & e)
      {
        std::cerr << "Error in acceptor: " << e.what() << "\n";
      }
    }
  );

  // Run the I/O service on the requested number of threads
  std::vector<std::thread> v(num_threads-1);
  for (auto i = num_threads - 1; i; --i)
    v.emplace_back([&ioc]{ ioc.run(); });

  // Use the main thread as well
  ioc.run();

  return EXIT_SUCCESS;
}
