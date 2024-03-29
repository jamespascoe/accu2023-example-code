/**
 * lua_fiber_action_connector.cpp
 *
 * This action allows Lua behaviours to send messages to other Lua behaviours.
 * The primary use-case for this action is for Lua behaviours to implement
 * algorithms that require distributed co-ordination.
 *
 * Copyright © Blu Wireless. All Rights Reserved.
 * Licensed under the MIT license. See LICENSE file in the project.
 * Feedback: james.pascoe@bluwireless.com
 */

#include "lua_fiber_action_connector.hpp"

#include "lua_fiber_log_manager.hpp"

Connector::Connector(unsigned short port)
    : m_acceptor(m_io_context, tcp::endpoint(tcp::v4(), port)) {
  start_accept();

  m_thread = std::thread([this]() { m_io_context.run(); });

  log_trace("Connector action starting");
}

Connector::~Connector() {
  log_trace("Cleaning up in Connector action");

  m_io_context.stop();
  m_thread.join();

  log_trace("Connector action exiting");
}

// Send a message to a remote behaviour
Connector::ErrorType Connector::Send(std::string const& hostname_or_ip,
                           std::string const& port,
                           std::string const& message) {
  // Resolve the destination endpoint
  tcp::resolver resolver(m_io_context);
  tcp::resolver::results_type endpoints;

  try {
    endpoints = resolver.resolve(hostname_or_ip, port);
  } catch (asio::system_error& e) {
    log_error(
        "Connector send failed: unable to resolve {}:{}", hostname_or_ip, port);

    return ErrorType::RESOLVE_FAILED;
  }

  // Open a connection
  tcp_connection::pointer connection = tcp_connection::create(m_io_context);

  try {
    asio::connect(connection->socket(), endpoints);
  } catch (asio::system_error& e) {
    log_error("Connector send failed: could not connect to {}:{}",
              hostname_or_ip,
              port);

    return ErrorType::CONNECT_FAILED;
  }

  asio::async_write(connection->socket(),
                    asio::buffer(message),
                    [this, connection](const asio::error_code& error,
                                       std::size_t bytes_transferred) {
                      handle_write(error, bytes_transferred, connection);
                    });

  return ErrorType::SUCCESS;
}

// Returns the most recent message (or an empty string if none are available)
std::string Connector::GetNextMessage(void) {
  if (!IsMessageAvailable())
    return "";

  std::string ret = m_messages.front();

  m_messages.erase(m_messages.begin());

  return ret;
}

// Asynchronous handlers for reading and accepting connections
void Connector::handle_read(asio::error_code const& error,
                          std::size_t bytes_transferred,
                          tcp_connection::pointer connection) {
  if (!error || error == asio::error::eof) {
    // Limit the message array to prevent it from growing uncontrollably
    if (m_messages.size() > max_messages)
      m_messages.erase(m_messages.begin());

    m_messages.emplace_back(connection->data());

    log_info("Received message ({} bytes): {}",
             bytes_transferred,
             connection->data());
  } else
    log_error("Connector read failed: returned error: {}", error.message());
}

// Note the 'maybe_unused' attribute for the TCP connection. This ensures that
// the underlying TCP socket is not closed until the write handler has exited.
void Connector::handle_write(
    asio::error_code const& error,
    std::size_t bytes_transferred,
    [[maybe_unused]] tcp_connection::pointer connection) {
  if (!error)
    log_info("Sent message ({} bytes)", bytes_transferred);
  else
    log_error("Connector send failed: returned error: {}", error.message());
}

void Connector::handle_accept(tcp_connection::pointer connection,
                         asio::error_code const& error) {
  if (!error) {
    log_debug("Accepted message connection");

    asio::async_read(connection->socket(),
                     asio::dynamic_buffer(connection->data()),
                     [this, connection](const asio::error_code& error,
                                        std::size_t bytes_transferred) {
                       handle_read(error, bytes_transferred, connection);
                     });
  } else
    log_error("Connector accept failed: returned error {}", error.message());

  start_accept();
}

void Connector::start_accept() {
  tcp_connection::pointer connection =
      tcp_connection::create(m_acceptor.get_executor().context());

  m_acceptor.async_accept(connection->socket(),
                          [this, connection](const asio::error_code& error) {
                            handle_accept(connection, error);
                          });
}
