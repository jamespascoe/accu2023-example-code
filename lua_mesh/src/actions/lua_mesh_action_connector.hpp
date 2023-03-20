/**
 * lua_chat_action_connector.hpp
 *
 * This action allows Lua behaviours to 'connect' to other nodes that
 * are connected to the Mesh. In addittion, this allows other nodes to
 * connect to this node.
 *
 * Copyright © Blu Wireless. All Rights Reserved.
 * Licensed under the MIT license. See LICENSE file in the project.
 * Feedback: james.pascoe@bluwireless.com
 */
#pragma once

#include <string>
#include <vector>

#include "asio/asio.hpp"

class Connector {
public:
  enum class ErrorType { SUCCESS, RESOLVE_FAILED, CONNECT_FAILED };

  inline static int const default_listen_port = 7777;

  Connector(unsigned short port = default_listen_port);

  ~Connector();

  // Do not allow instances to be copied or moved
  Connector(Connector const &rhs) = delete;
  Connector(Connector &&rhs) = delete;
  Connector &operator=(Connector const &rhs) = delete;
  Connector &operator=(Connector &&rhs) = delete;

  // Methods relating to outgoing peer connections
  ErrorType Connect(std::string const &hostname_or_ip, std::string const &port);

  void Disconnect() {
    if (m_connection)
      m_connection->socket().close();
  }

  ErrorType Send(std::string const &message);

  // Methods relating to incoming peer connections
  bool IsPeerConnected() const { return m_peer_connected; }

  bool IsMessageAvailable() const { return !m_messages.empty(); }
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

    static pointer create(asio::io_context &io_context) {
      return pointer(new tcp_connection(io_context));
    }

    tcp::socket &socket() { return m_socket; }

    std::string &data() { return m_data; }

  private:
    tcp_connection(asio::io_context &io_context) : m_socket(io_context) {}

    tcp::socket m_socket;
    std::string m_data;
  };

  // Methods relating to accepting connections from Mesh nodes.
  void start_accept();
  void handle_accept(tcp_connection::pointer connection,
                     asio::error_code const &error);

  void handle_read(asio::error_code const &error, std::size_t bytes_transferred,
                   tcp_connection::pointer connection);

  void handle_write(asio::error_code const &error,
                    std::size_t bytes_transferred,
                    tcp_connection::pointer connection);

  // Member variables
  asio::io_context m_io_context;
  asio::ip::tcp::acceptor m_acceptor;

  std::thread m_thread;

  tcp_connection::pointer m_connection = nullptr;

  bool m_peer_connected = false;
  std::vector<std::string> m_messages;
};
