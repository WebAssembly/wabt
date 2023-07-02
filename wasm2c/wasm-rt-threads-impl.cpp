/*
 * Copyright 2018 WebAssembly Community Group participants
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "wasm-rt-threads.h"

#include <assert.h>
#include <atomic>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <thread>

#define WAITER_ATOMIC_STATE_NOTIFIED 0
#define WAITER_ATOMIC_STATE_WAITING 1
#define WAITER_ATOMIC_STATE_TIMEDOUT 2

struct waiter_info_t {
  std::atomic<uint8_t> state;
};

struct waiters_info_t {
  std::list<std::shared_ptr<waiter_info_t>> waiter_list;
};

struct wait_notify_info_t {
  // Mutex to guard all accesses to waiters_info_map
  std::mutex map_guard;
  std::map<uintptr_t, waiters_info_t> waiters_info_map;
};

wait_notify_info_t wait_notify_info;

/**
 * A timer thread --- Internal function that spwans to set an atomic variable
 * (waiter_info_weak->state) after a given timeout. This thread wakes up every
 * 100ms to see if the timer is still required. If the timer is no longer
 * required, it exits the thread.
 */
static void create_timeout(int64_t timeout,
                           std::weak_ptr<waiter_info_t> waiter_info_weak) {
  if (timeout < 0) {
    return;
  }

  // Spawn a timer thread
  std::thread([timeout, waiter_info_weak]() {
    const int64_t max_timeout_chunk = 100000000;  // 100 ms in nanos
    int64_t remaining = timeout;

    while (remaining > 0) {
      const int64_t chosen_time = std::min(remaining, max_timeout_chunk);

      std::this_thread::sleep_for(std::chrono::nanoseconds(chosen_time));
      remaining -= chosen_time;

      std::shared_ptr<waiter_info_t> waiter_info = waiter_info_weak.lock();

      // object was notified and the wait state no longer exists. Exit the timer
      // thread.
      if (!waiter_info) {
        return;
      }
    }

    // Timeout complete. Set the state to timed out if the object still exists
    // in the waiting state.
    std::shared_ptr<waiter_info_t> waiter_info = waiter_info_weak.lock();
    if (waiter_info) {
      uint8_t expected = WAITER_ATOMIC_STATE_WAITING;
      if (waiter_info->state.compare_exchange_strong(
              expected, WAITER_ATOMIC_STATE_TIMEDOUT)) {
        waiter_info->state.notify_one();
      }
    }
  }).detach();
}

/**
 * Wait on the given address for a notify or timeout.
 */
uint32_t wasm_rt_wait_on_address(uintptr_t address, int64_t timeout) {
  auto waiter = std::make_shared<waiter_info_t>();
  waiter->state = WAITER_ATOMIC_STATE_WAITING;

  {
    std::lock_guard<std::mutex> lock(wait_notify_info.map_guard);
    wait_notify_info.waiters_info_map[address].waiter_list.push_back(waiter);
  }

  std::weak_ptr<waiter_info_t> waiter_weak = waiter;
  create_timeout(timeout, waiter_weak);

  waiter->state.wait(WAITER_ATOMIC_STATE_WAITING);

  {
    std::lock_guard<std::mutex> lock(wait_notify_info.map_guard);
    wait_notify_info.waiters_info_map[address].waiter_list.remove(waiter);
  }

  return waiter->state;
}

/**
 * Notify (count) waiters on the given address.
 */
uint32_t wasm_rt_notify_at_address(uintptr_t address, uint32_t count) {
  uint32_t curr_notified = 0;

  std::lock_guard<std::mutex> lock(wait_notify_info.map_guard);
  auto p_waiter_list =
      &(wait_notify_info.waiters_info_map[address].waiter_list);

  for (auto p_waiter : *p_waiter_list) {
    if (curr_notified == count) {
      break;
    }

    uint8_t expected = WAITER_ATOMIC_STATE_WAITING;
    if (p_waiter->state.compare_exchange_strong(expected,
                                                WAITER_ATOMIC_STATE_NOTIFIED)) {
      p_waiter->state.notify_one();
      curr_notified++;
    }
  }

  return curr_notified;
}
