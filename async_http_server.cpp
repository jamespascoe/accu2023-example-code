

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/config.hpp>

#include <iostream>
#include <format>

namespace asio = boost::asio;
using tcp = boost::asio::ip::tcp;

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
  auto const threads = std::max<int>(1, std::atoi(argv[3]));

  asio::io_context ioc{threads};


}
