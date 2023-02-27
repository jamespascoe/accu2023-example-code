

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/config.hpp>

#include <iostream>
#include <format>




int main(int argc, char *argv[])
{
  if (argc != 3) {
    std::cerr << std::format(
      "Usage: {} <ip-address> <port>\nE.g.: {} 0.0.0.0 8080\n",
      argv[0], argv[0]
    );
    return EXIT_FAILURE;
  }


}
