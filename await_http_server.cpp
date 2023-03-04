// g++-12.2 -std=c++2b ../http_server_awaitable.cpp -L /usr/local/lib64 -I /home/pascoej/boost_1_81_0/ -L /home/pascoej/boost_1_81_0/libs/ -l pthread -o http_server_awaitable

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/config.hpp>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <format>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

using tcp_stream = typename beast::tcp_stream::rebind_executor<
        net::use_awaitable_t<>::executor_with_default<net::any_io_executor>>::other;

// Handles an HTTP server connection
net::awaitable<void> do_session(tcp_stream stream)
{
    beast::error_code ec;

    // This buffer is required to persist across reads
    beast::flat_buffer buffer;

    // This lambda is used to send messages
    for(;;)
    try
    {
        // Set the timeout.
        stream.expires_after(std::chrono::seconds(30));

        // Read a request
        http::request<http::string_body> req;
        co_await http::async_read(stream, buffer, req);

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
        co_await beast::async_write(stream, std::move(msg), net::use_awaitable);

        if(! keep_alive)
        {
            // This means we should close the connection, usually because
            // the response indicated the "Connection: close" semantic.
            break;
        }
    }
    catch (boost::system::system_error & se)
    {
        if (se.code() != http::error::end_of_stream )
            throw ;
    }

    // Send a TCP shutdown
    stream.socket().shutdown(tcp::socket::shutdown_send, ec);

    // At this point the connection is closed gracefully
}

//------------------------------------------------------------------------------

// Accepts incoming connections and launches the sessions
net::awaitable<void> do_listen(tcp::endpoint endpoint)
{
    // Open the acceptor
    auto acceptor = net::use_awaitable.as_default_on(tcp::acceptor(co_await net::this_coro::executor));
    acceptor.open(endpoint.protocol());

    // Allow address reuse
    acceptor.set_option(net::socket_base::reuse_address(true));

    // Bind to the server address
    acceptor.bind(endpoint);

    // Start listening for connections
    acceptor.listen(net::socket_base::max_listen_connections);

    for(;;)
        boost::asio::co_spawn(
            acceptor.get_executor(),
                do_session(tcp_stream(co_await acceptor.async_accept())),
                [](std::exception_ptr e)
                {
                    try
                    {
                        // Print out stack trace here
                        std::rethrow_exception(e);
                    }
                    catch (std::exception &e) {
                        std::cerr << "Error in session: " << e.what() << "\n";
                    }
                });

}

int main(int argc, char* argv[])
{
    // Check command line arguments.
    if (argc != 4) {
      std::cerr << std::format(
        "Usage: {} <ip-address> <port> <threads>\nE.g.: {} 0.0.0.0 8080\n",
        argv[0], argv[0]
      );
      return EXIT_FAILURE;
    }

    auto const address = net::ip::make_address(argv[1]);
    auto const port = static_cast<unsigned short>(std::atoi(argv[2]));
    auto const num_threads = std::max<int>(1, std::atoi(argv[4]));

    // The io_context is required for all I/O
    net::io_context ioc{num_threads};

    // Spawn a listening port
    boost::asio::co_spawn(ioc,
                          do_listen(tcp::endpoint{address, port}),
                          [](std::exception_ptr e)
                          {
                              if (e)
                                  try
                                  {
                                      std::rethrow_exception(e);
                                  }
                                  catch(std::exception & e)
                                  {
                                      std::cerr << "Error in acceptor: " << e.what() << "\n";
                                  }
                          });

    // Run the I/O service on the requested number of threads
    std::vector<std::thread> v(num_threads-1);
    for (auto i = num_threads - 1; i; --i)
      v.emplace_back([&ioc]{ ioc.run(); });

    // Use the main thread as well
    ioc.run();

    return EXIT_SUCCESS;
}
