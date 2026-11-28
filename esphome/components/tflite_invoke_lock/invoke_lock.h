#pragma once

#include "esphome/core/helpers.h"

namespace esphome::tflite_invoke_lock {

namespace internal {
/// The one lock of the firmware. A function static, so it exists before its first use.
inline Mutex &invoke_mutex() {
  static Mutex mutex;
  return mutex;
}
/// Set on the task that holds invoke_mutex().
inline bool &holds_invoke_mutex() {
  static thread_local bool holds = false;
  return holds;
}
}  // namespace internal

/// @brief Hold this around every tflite::MicroInterpreter::Invoke().
///
/// The optimized ESP-NN kernels keep their scratch buffers in globals, so only one interpreter may invoke at a
/// time. The constructor waits while another task is invoking. It does not wait when the calling task already
/// invokes, for example from a log emitted by a kernel: acquired() is then false and the model must not run.
class InvokeLock {
 public:
  InvokeLock() : acquired_(!held_by_current_task()) {
    if (!this->acquired_)
      return;
    internal::invoke_mutex().lock();
    internal::holds_invoke_mutex() = true;
  }
  ~InvokeLock() {
    if (!this->acquired_)
      return;
    internal::holds_invoke_mutex() = false;
    internal::invoke_mutex().unlock();
  }
  InvokeLock(const InvokeLock &) = delete;
  InvokeLock &operator=(const InvokeLock &) = delete;

  bool acquired() const { return this->acquired_; }

  /// True while the calling task is invoking a model. Such a task must not block on another model's lock, whose
  /// owner may be waiting for this one.
  static bool held_by_current_task() { return internal::holds_invoke_mutex(); }

 private:
  const bool acquired_;
};

}  // namespace esphome::tflite_invoke_lock
