/**
 * bwt_mcm_action_timer.hpp
 *
 * The Timer action allows the user to wait for a given duration. Lua simply
 * polls the object to discover whether the timer has fired. The action allows
 * for non-blocking and blocking waits.
 *
 * Copyright © Blu Wireless. All Rights Reserved.
 * Licensed under the MIT license. See LICENSE file in the project.
 * Feedback: james.pascoe@bluwireless.com
 */
#pragma once

#include "asio/asio.hpp"

#include <memory>
#include <mutex>
#include <thread>

class Timer {
public:
  enum class WaitType {
    NOBLOCK,
    BLOCK,
  };

  // Constructs a timer
  Timer();

  // Cleans up after a timer
  ~Timer();

  // Do not allow copying or moving a Timer object
  Timer(Timer const&) = delete;
  Timer& operator=(Timer const&) = delete;
  Timer(Timer&&) = delete;
  Timer& operator=(Timer&&) = delete;

  // Invoke the timer using for the given duration
  void operator()(WaitType const waitType,
                  int const duration,
                  std::string const& timeUnit,
                  int notifyId);

  // Cancel a running timer
  unsigned int Cancel();

  // Check if the timer is waiting
  bool IsWaiting() const { return m_waiting; }

  // Check if the timer has expired
  bool HasExpired() const { return m_expired; }

private:
  using work = asio::executor_work_guard<asio::io_context::executor_type>;

  asio::io_context m_ctx;
  asio::steady_timer m_timer;

  // This prevents the context's run() call from returning when it has no
  // handlers to execute and grants us control over when that happens via the
  // unique_ptr.
  std::unique_ptr<work> m_work;

  // The thread to execute the timer handlers
  std::thread m_workerThread;

  // Mutex to make sure all timer state transitions are atomic
  std::mutex m_timerMutex;

  // Flag indicating whether the timer is waiting
  bool m_waiting = false;

  // Flag indicating whether the timer has fired
  bool m_expired = false;

  // Notification ID to return to the caller via the callback
  int m_notifyId = 0;

  // The function the thread will run
  void threadRoutine() { m_ctx.run(); }

  // Function to be executed when the timer expires or is cancelled
  void timerHandler(asio::error_code const& error);

  // Sets state variables to indicate that the timer is no longer running
  void expire();

  // Sets state variables to indicate that the timer has been cancelled
  void cancel();
};
