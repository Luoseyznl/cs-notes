#include <gtest/gtest.h>

#include <atomic>
#include <thread>
#include <vector>

#include "thread_safe_stack.hpp"

using namespace concurrency;

// --- 测试 1：单线程下的基础逻辑与异常安全 ---
TEST(ThreadSafeStackTest, SingleThreadBasic) {
  ThreadSafeStack<int> st;

  EXPECT_TRUE(st.empty());  // 初始状态应该为空

  EXPECT_THROW(st.pop(), EmptyStackException);  // 栈空时调用 pop 必须抛出异常

  st.push(42);
  EXPECT_FALSE(st.empty());

  // 测试引用出栈
  int val = 0;
  st.pop(val);
  EXPECT_EQ(val, 42);
  EXPECT_TRUE(st.empty());

  // 测试指针出栈
  st.push(99);
  auto ptr = st.pop();
  ASSERT_NE(ptr, nullptr);  // 确保指针非空
  EXPECT_EQ(*ptr, 99);
  EXPECT_TRUE(st.empty());
}

// --- 测试 2：多线程并发测试 ---
TEST(ThreadSafeStackTest, ConcurrentPushPop) {
  ThreadSafeStack<int> st;
  std::atomic<int> pop_count{0};  // 记录成功弹出的元素总数

  const int num_threads = 10;
  const int items_per_thread = 1000;

  // 生产者 Lambda
  auto producer = [&]() {
    for (int i = 0; i < items_per_thread; ++i) {
      st.push(i);
    }
  };

  // 消费者 Lambda
  auto consumer = [&]() {
    for (int i = 0; i < items_per_thread; ++i) {
      try {
        auto ptr = st.pop();
        if (ptr) {
          pop_count++;
        }
      } catch (const EmptyStackException&) {
        // 如果遭遇栈空异常，说明消费者跑得比生产者快，主动退让并重试
        std::this_thread::yield();
        --i;
      }
    }
  };

  std::vector<std::thread> threads;
  for (int i = 0; i < num_threads; ++i) {
    threads.emplace_back(producer);
    threads.emplace_back(consumer);
  }

  for (auto& t : threads) {
    t.join();
  }

  EXPECT_EQ(pop_count.load(), num_threads * items_per_thread);
  EXPECT_TRUE(st.empty());
}