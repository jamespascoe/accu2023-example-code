/**
 * lua_mesh_action_connector.cpp
 *
 * This action allows Lua behaviours to 'connect' to other nodes that
 * are connected to the Mesh. In addittion, this allows other nodes to
 * connect to this node.
 *
 * Copyright © Blu Wireless. All Rights Reserved.
 * Licensed under the MIT license. See LICENSE file in the project.
 * Feedback: james.pascoe@bluwireless.com
 */

#include "lua_mesh_action_connector.hpp"

#include "lua_mesh_log_manager.hpp"

Connector::Connector(unsigned short port)
    : m_acceptor(m_io_context, tcp::endpoint(tcp::v4(), port)) {
  start_accept();

  m_thread = std::thread([this]() { m_io_context.run(); });

  log_trace("Connector action started");
}

Connector::~Connector() {
  log_trace("Cleaning up in connect action");

  m_io_context.stop();
  m_thread.join();

  log_trace("Connector action exiting");
}

Connector::ErrorType Connector::OpenConnection() {
  return ErrorType::SUCCESS;
}

// Asynchronous handlers
void Connector::handle_read(asio::error_code const& error,
                            std::size_t bytes_transferred,
                            tcp_connection::pointer connection) {
  if (error == asio::error::eof) {
    // Peer has disconnected
    m_peer_connected = false;
    return;
  }

  if (!error) {
    log_info("Received message ({} bytes): {}",
             bytes_transferred,
             connection->data());

    /* TODO: Setup for next read */
  } else
    log_error("Connector read failed: returned error: {}", error.message());
}

void Connector::handle_accept(tcp_connection::pointer connection,
                              asio::error_code const& error) {
  if (!error) {
    log_debug("Accepted connection");

    m_peer_connected = true;

    // Schedule an async_receive. If the recieve fails, then m_peer_connected = false;
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
