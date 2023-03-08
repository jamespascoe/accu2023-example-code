#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/strand.hpp>

#include <iostream>
#include <thread>
#include <format>

namespace beast = boost::beast;
namespace http = beast::http;
namespace asio = boost::asio;
using tcp = boost::asio::ip::tcp;

void error(beast::error_code ec, char const* what)
{
    std::cerr << std::format("Error: {} : {}\n", what, ec.message());
    return;
};

// Handles an HTTP server connection
class session : public std::enable_shared_from_this<session>
{
    beast::tcp_stream stream_;
    beast::flat_buffer buffer_;
    http::request<http::string_body> req_;

public:
    // Take ownership of the stream
    session(
        tcp::socket&& socket)
        : stream_(std::move(socket))
    {
    }

    // Start the asynchronous operation
    void
    run()
    {
        // We need to be executing within a strand to perform async operations
        // on the I/O objects in this session. Although not strictly necessary
        // for single-threaded contexts, this example code is written to be
        // thread-safe by default.
        asio::dispatch(stream_.get_executor(),
                       beast::bind_front_handler(
                       &session::do_read,
                       shared_from_this()));
    }

    void
    do_read()
    {
        // Make the request empty before reading,
        // otherwise the operation behavior is undefined.
        req_ = {};

        // Set the timeout.
        stream_.expires_after(std::chrono::seconds(30));

        // Read a request
        http::async_read(stream_, buffer_, req_,
            beast::bind_front_handler(
                &session::on_read,
                shared_from_this()));
    }

    void
    on_read(
        beast::error_code ec,
        std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        // This means they closed the connection
        if(ec == http::error::end_of_stream) {
            stream_.socket().shutdown(tcp::socket::shutdown_send, ec);
            return;
        }

        if(ec) return error(ec, "read");

        // Send the response
        auto handle_request = [this]() -> http::message_generator {
          http::response<http::string_body> res{http::status::ok, req_.version()};
          res.set(http::field::server, "Boost.Beast");
          res.body() = "Hello ACCU 2023 from the Asynchronous Server!";
          res.prepare_payload();
          res.keep_alive(req_.keep_alive());
          return res;
        };

        beast::async_write(
            stream_,
            handle_request(),
            beast::bind_front_handler(
                &session::on_write, shared_from_this()));
    }

    void
    on_write(
        beast::error_code ec,
        std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if(ec) return error(ec, "write");

        stream_.socket().shutdown(tcp::socket::shutdown_send, ec);
        // Read another request
        do_read();
    }
};

// Accepts incoming connections and launches the sessions
class listener : public std::enable_shared_from_this<listener>
{
    asio::io_context& ioc_;
    tcp::acceptor acceptor_;

public:
    listener(
        asio::io_context& ioc,
        tcp::endpoint endpoint)
        : ioc_(ioc)
        , acceptor_(asio::make_strand(ioc))
    {
        beast::error_code ec;

        // Open the acceptor
        acceptor_.open(endpoint.protocol(), ec);
        if(ec) error(ec, "open");

        // Allow address reuse
        acceptor_.set_option(asio::socket_base::reuse_address(true), ec);
        if(ec) error(ec, "set_option");

        // Bind to the server address
        acceptor_.bind(endpoint, ec);
        if(ec) error(ec, "bind");

        // Start listening for connections
        acceptor_.listen(
            asio::socket_base::max_listen_connections, ec);
        if(ec) error(ec, "listen");
    }

    // Start accepting incoming connections
    void run()
    {
        do_accept();
    }

private:
    void
    do_accept()
    {
        // The new connection gets its own strand
        acceptor_.async_accept(
            asio::make_strand(ioc_),
            beast::bind_front_handler(
                &listener::on_accept,
                shared_from_this()));
    }

    void
    on_accept(beast::error_code ec, tcp::socket socket)
    {
        if(ec) error(ec, "accept");

        // Create the session and run it
        std::make_shared<session>(
            std::move(socket))->run();

        // Accept another connection
        do_accept();
    };
};

int main(int argc, char *argv[])
{
  if (argc != 4) {
    std::cerr << std::format(
      "Usage: {} <ip-address> <port> <threads>\n"
      "E.g.: {} 0.0.0.0 8080 2\n",
      argv[0], argv[0]
    );
    return EXIT_FAILURE;
  }

  auto const address = asio::ip::make_address(argv[1]);
  auto const port = static_cast<unsigned short>(std::atoi(argv[2]));
  auto const num_threads = std::max<int>(1, std::atoi(argv[3]));

  asio::io_context ioc{num_threads};

  // Create and launch a listening port
  std::make_shared<listener>(ioc, tcp::endpoint{address, port})->run();

  // Run the IO service with the requested number of threads
  std::vector<std::thread> v(num_threads-1);
  for (auto i = num_threads - 1; i; --i)
    v.emplace_back([&ioc]{ ioc.run(); });

  // Use the main thread as well
  ioc.run();

  return EXIT_SUCCESS;
}
