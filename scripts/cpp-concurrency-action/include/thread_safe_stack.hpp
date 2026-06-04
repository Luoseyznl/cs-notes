#ifndef THREAD_SAFE_STACK_HPP
#define THREAD_SAFE_STACK_HPP

#include <exception>
#include <memory>
#include <mutex>
#include <stack>

namespace concurrency {

// 异常类也在同一命名空间内，允许跨文件引用时捕获异常
struct EmptyStackException : public std::exception {
  const char* what() const noexcept override { return "Empty Stack"; }
};

template <typename T>
class ThreadSafeStack {
 private:
  std::stack<T> data_;
  mutable std::mutex mtx_;

 public:
  ThreadSafeStack() = default;

  // 拷贝构造函数，只需要锁住 other 即可，因为此时自身对象才刚构造
  ThreadSafeStack(const ThreadSafeStack& other) {
    std::lock_guard<std::mutex> lock(other.mtx_);
    data_ = other.data_;
  }

  // 禁用赋值操作符，不然容易逻辑混乱（很容易“狸猫换太子”）
  ThreadSafeStack& operator=(const ThreadSafeStack&) = delete;

  void push(T new_value) {
    std::lock_guard<std::mutex> lock(mtx_);
    data_.push(std::move(new_value));
  }

  // 出栈方式 1：通过引用传出参数（先移动构造，再弹出无效的对象）
  void pop(T& value) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (data_.empty()) {
      throw EmptyStackException();
    }
    value = std::move(data_.top());
    data_.pop();
  }

  // 出栈方式 2：返回 shared_ptr 保证异常安全
  std::shared_ptr<T> pop() {
    std::lock_guard<std::mutex> lock(mtx_);
    if (data_.empty()) {
      throw EmptyStackException();
    }
    // 即使 make_shared 抛出 std::bad_alloc，栈内的数据依然完好无损
    std::shared_ptr<T> const res = std::make_shared<T>(std::move(data_.top()));
    data_.pop();
    return res;
  }

  bool empty() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return data_.empty();
  }
};

}  // namespace concurrency

#endif  // THREAD_SAFE_STACK_HPP