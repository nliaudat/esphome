#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <thread>

#include "esphome/components/tflite_invoke_lock/invoke_lock.h"

namespace esphome::tflite_invoke_lock::testing {

TEST(InvokeLockTest, ANestedLockOnTheSameTaskIsRefused) {
  EXPECT_FALSE(InvokeLock::held_by_current_task());
  {
    InvokeLock outer;
    EXPECT_TRUE(outer.acquired());
    {
      InvokeLock nested;
      EXPECT_FALSE(nested.acquired());
    }
    // Leaving the refused lock does not release the outer one.
    EXPECT_TRUE(InvokeLock::held_by_current_task());
  }
  EXPECT_FALSE(InvokeLock::held_by_current_task());
  InvokeLock again;
  EXPECT_TRUE(again.acquired());
}

TEST(InvokeLockTest, AnotherTaskWaitsUntilTheLockIsReleased) {
  std::atomic<bool> entered{false};
  std::thread other;
  {
    InvokeLock lock;
    other = std::thread([&entered]() {
      EXPECT_FALSE(InvokeLock::held_by_current_task());
      InvokeLock waiting;
      EXPECT_TRUE(waiting.acquired());
      entered = true;
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_FALSE(entered);
  }
  other.join();
  EXPECT_TRUE(entered);
}

}  // namespace esphome::tflite_invoke_lock::testing
