/**
 * lua_fiber_action_timer.cpp
 *
 * The Timer action allows the user to wait for a given duration. Lua simply
 * polls the object to discover whether the timer has fired. The action allows
 * for non-blocking and blocking waits.
 *
 * Copyright © Blu Wireless. All Rights Reserved.
 * Licensed under the MIT license. See LICENSE file in the project.
 * Feedback: james.pascoe@bluwireless.com
 */
#include "lua_fiber_action_timer.hpp"

#include "lua_fiber_log_manager.hpp"

#include <functional>

Timer::Timer()
    : m_ctx(),
      m_timer(m_ctx),
      m_work(std::make_unique<work>(asio::make_work_guard(m_ctx))),
      m_workerThread(std::bind(&Timer::threadRoutine, this)) {
  log_trace("New timer initialised.");
}

Timer::~Timer() {
  log_trace("Destroying timer");

  if (m_waiting)
    Cancel();

  // Force the work_guard out of scope to allow the io_context to stop
  m_work.reset();

  // We can now join the worker thread
  m_workerThread.join();
}

void Timer::operator()(WaitType const waitType,
                       int const duration,
                       std::string const& timeUnit,
                       int notifyId) {
  if (timeUnit != "s" && timeUnit != "ms" && timeUnit != "us") {
    log_error("Incorrect time unit supplied: {}. Supported are: s, ms, us",
              timeUnit);
    return;
  }

  auto castDuration = (timeUnit == "s")
                          ? std::chrono::seconds(duration)
                          : (timeUnit == "ms")
                                ? std::chrono::milliseconds(duration)
                                : std::chrono::microseconds(duration);

  {
    std::lock_guard lock(m_timerMutex);

    if (m_waiting) {
      log_debug("Timer already running (ID {})", m_notifyId);
      return;
    }

    m_waiting = true;
    m_expired = false;

    m_notifyId = notifyId;
  }

  try {
    m_timer.expires_after(castDuration);
  } catch (asio::system_error const& e) {
    log_error("Unable to set timer expiry. Aborting wait. ({})", e.what());

    cancel();
    return;
  }

  switch (waitType) {
    case WaitType::BLOCK: {
      log_debug("Timer {} blocking for {} {}", m_notifyId, duration, timeUnit);

      asio::error_code err;
      m_timer.wait(err);

      if (err) {
        // This shouldn't happen unless something's gone very wrong
        log_error("Unable to initiate blocking wait: {}", err.message());
      }

      expire();

      break;
    }
    case WaitType::NOBLOCK: {
      log_debug("Timer {} waiting in background for {} {}",
                m_notifyId,
                duration,
                timeUnit);

      m_timer.async_wait(
          std::bind(&Timer::timerHandler, this, std::placeholders::_1));

      break;
    }
    default: {
      log_error("Invalid wait type given");
      break;
    }
  }
}

unsigned int Timer::Cancel() {
  asio::error_code error;
  auto timersCancelled = m_timer.cancel(error);

  // If the timer has expired, then ASIO will not call the timer handler (as
  // there is no work to do). As a consequence, we need to reset the internal
  // state by calling 'cancel' directly.
  if (timersCancelled == 0)
    cancel();

  if (error)
    log_error("Unable to cancel timer {}: {}", m_notifyId, error.message());
  else
    log_debug("Cancelled {} timers", timersCancelled);

  return timersCancelled;
}

void Timer::timerHandler(asio::error_code const& error) {
  // Process the timer event
  if (!error) {
    expire();
  } else {
    // The timer has either been aborted i.e. cancelled explicitly via a call
    // to 'Cancel' or has encountered an error.
    if (error != asio::error::operation_aborted)
      log_error("Timer {} returned error: {}", m_notifyId, error.message());

    cancel();
  }
}

void Timer::expire() {
  log_debug("Timer {} expired", m_notifyId);

  std::lock_guard lock(m_timerMutex);

  m_notifyId = 0;

  m_expired = true;
  m_waiting = false;
}

void Timer::cancel() {
  log_debug("Timer {} cancelled", m_notifyId);

  std::lock_guard lock(m_timerMutex);

  m_notifyId = 0;

  m_expired = false;
  m_waiting = false;
}
