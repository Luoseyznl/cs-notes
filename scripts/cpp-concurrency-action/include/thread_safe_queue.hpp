#ifndef THREAD_SAFE_QUEUE_HPP
#define THREAD_SAFE_QUEUE_HPP

#include <condition_variable>
#include <memory>
#include <mutex>
#include <utility>

namespace concurrency {

template <typename T>
class ThreadSafeQueue {
 private:
  struct Node {
    std::shared_ptr<T> data;     // 持有智能指针，避免在锁内分配内存
    std::unique_ptr<Node> next;  // 链表下游节点所有权
  };

  std::mutex head_mutex_;       // 保护队头弹出操作
  std::unique_ptr<Node> head_;  // 指向当前可弹出的头节点（或哑节点）
  std::mutex tail_mutex_;       // 保护队尾推入操作
  Node* tail_;                  // 指向链表末尾的哑节点（裸指针）

  std::condition_variable data_cond_;

  // ==========================================
  // 辅助函数：底层物理指针流转与原子操作
  // ==========================================

  // 安全获取当前尾指针
  Node* GetTail() {
    std::lock_guard<std::mutex> tail_lock(tail_mutex_);
    return tail_;
  }

  // 阻塞等待队列不为空（锁住头锁，并在条件变量中通过 GetTail() 跨锁查看）
  std::unique_lock<std::mutex> WaitForData() {
    std::unique_lock<std::mutex> head_lock(head_mutex_);
    data_cond_.wait(head_lock, [&] { return head_.get() != GetTail(); });
    return head_lock;  // 将持锁的外部实例通过右值传出
  }

  // 弹出当前的头节点，并将 head_ 移向下一个节点
  std::unique_ptr<Node> PopHead() {
    std::unique_ptr<Node> old_head = std::move(head_);
    head_ = std::move(old_head->next);
    return old_head;
  }

  // 阻塞弹出辅助函数
  std::unique_ptr<Node> WaitPopHead() {
    std::unique_lock<std::mutex> head_lock(WaitForData());
    return PopHead();
  }

  // 阻塞弹出辅助函数（传出引用）
  std::unique_ptr<Node> WaitPopHead(T& value) {
    std::unique_lock<std::mutex> head_lock(WaitForData());
    // 🌟 核心异常安全：在修改 head_ 指针前，先将数据移动构造给用户
    // 如果这里移动抛出异常，整个队列的物理结构没有任何改变，实现绝对安全
    value = std::move(*head_->data);
    return PopHead();
  }

  // 非阻塞弹出辅助函数
  std::unique_ptr<Node> TryPopHead() {
    std::lock_guard<std::mutex> head_lock(head_mutex_);
    if (head_.get() == GetTail()) {
      return nullptr;  // 队列为空
    }
    return PopHead();
  }

  // 非阻塞弹出辅助函数（传出引用）
  std::unique_ptr<Node> TryPopHead(T& value) {
    std::lock_guard<std::mutex> head_lock(head_mutex_);
    if (head_.get() == GetTail()) {
      return nullptr;
    }
    value = std::move(*head_->data);
    return PopHead();
  }

 public:
  ThreadSafeQueue() : head_(new Node), tail_(head_.get()) {}

  // 禁用拷贝与赋值操作，确保多线程环境下的语义唯一性
  ThreadSafeQueue(const ThreadSafeQueue&) = delete;
  ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;

  // ==========================================
  // 公共接口：生产者入队与消费者出队
  // ==========================================

  void Push(T new_value) {
    // 🌟 优化：在锁的范围之外，提前在堆上把用户数据和新节点分配好
    // 内存分配是高能耗操作，移出锁外能极大地缩短 tail_mutex_ 的持有时间！
    std::shared_ptr<T> new_data = std::make_shared<T>(std::move(new_value));
    std::unique_ptr<Node> p(new Node);
    Node* const new_tail = p.get();

    {
      std::lock_guard<std::mutex> tail_lock(tail_mutex_);
      // 1. 把真正的数据挂在当前的哑节点上（当前哑节点正式“转正”为数据节点）
      tail_->data = new_data;
      // 2. 把新分配的空节点挂在尾部，让它成为全新的哑节点
      tail_->next = std::move(p);
      // 3. 移动尾指针到新的哑节点上
      tail_ = new_tail;
    }
    // 💡 最佳实践：在释放锁之后再通知条件变量，防止消费者刚醒来就被锁死
    data_cond_.notify_one();
  }

  // 阻塞式弹出：返回指向数据的智能指针
  std::shared_ptr<T> WaitAndPop() {
    std::unique_ptr<Node> const old_head = WaitPopHead();
    return old_head->data;
  }

  // 阻塞式弹出：通过引用将数据传出
  void WaitAndPop(T& value) {
    std::unique_ptr<Node> const old_head = WaitPopHead(value);
  }

  // 非阻塞式弹出：如果成功获取则返回数据指针，否则返回空智能指针
  std::shared_ptr<T> TryPop() {
    std::unique_ptr<Node> old_head = TryPopHead();
    return old_head ? old_head->data : std::shared_ptr<T>();
  }

  // 非阻塞式弹出：通过引用传出，返回 bool 表示是否成功弹出
  bool TryPop(T& value) {
    std::unique_ptr<Node> const old_head = TryPopHead(value);
    return old_head != nullptr;
  }

  // 检查队列是否为空
  bool Empty() {
    std::lock_guard<std::mutex> head_lock(head_mutex_);
    return (head_.get() == GetTail());
  }
};

}  // namespace concurrency

#endif  // THREAD_SAFE_QUEUE_HPP