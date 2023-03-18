/**
 * lua_chat_action_talk.hpp
 *
 * This action allows Lua behaviours to send messages to other Lua behaviours.
 * The primary use-case for this action is for Lua behaviours to implement
 * algorithms that require distributed co-ordination e.g. 'best 2of4'.
 *
 * Copyright © Blu Wireless. All Rights Reserved.
 * Licensed under the MIT license. See LICENSE file in the project.
 * Feedback: james.pascoe@bluwireless.com
 */
#pragma once

#include "asio/asio.hpp"

class Talk {
public:
  enum class ErrorType { SUCCESS, RESOLVE_FAILED, CONNECT_FAILED };

  inline static int const default_port = 7777;

  Talk(unsigned short port = default_port);

  ~Talk();

  // Do not allow instances to be copied or moved
  Talk(Talk const& rhs) = delete;
  Talk(Talk&& rhs) = delete;
  Talk& operator=(Talk const& rhs) = delete;
  Talk& operator=(Talk&& rhs) = delete;

  // Send a message to a remote behaviour
  ErrorType Send(std::string const& hostname_or_ip,
                 std::string const& port,
                 std::string const& message);

  // Returns whether a message is available to be read
  bool IsMessageAvailable(void) { return !m_messages.empty(); }

  // Returns the most recent message (or an empty string if none are available)
  std::string GetNextMessage(void);

private:
  using tcp = asio::ip::tcp;

  // Max number of messages to retain
  inline static const int max_messages = 32;

  // Class to encapsulate a TCP connection and the data sent over it. Note
  // that this is based on the ASIO Asychronous TCP server tutorial.
  class tcp_connection {
  public:
    using pointer = std::shared_ptr<tcp_connection>;

    static pointer create(asio::io_context& io_context) {
      return pointer(new tcp_connection(io_context));
    }

    tcp::socket& socket() { return m_socket; }

    std::string& data() { return m_data; }

  private:
    tcp_connection(asio::io_context& io_context) : m_socket(io_context) {}

    tcp::socket m_socket;
    std::string m_data;
  };

  // Asynchronous handlers for reading and accepting connections
  void handle_read(asio::error_code const& error,
                   std::size_t bytes_transferred,
                   tcp_connection::pointer connection);

  void handle_write(asio::error_code const& error,
                    std::size_t bytes_transferred,
                    tcp_connection::pointer connection);

  void handle_accept(tcp_connection::pointer new_connection,
                     asio::error_code const& error);

  void start_accept();

  // Member variables
  asio::io_context m_io_context;
  asio::ip::tcp::acceptor m_acceptor;

  std::thread m_thread;

  // FIFO vector to store received messages
  std::vector<std::string> m_messages;
};
