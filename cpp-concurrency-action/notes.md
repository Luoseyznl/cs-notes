# C++ 并发编程工具

> The Free Lunch Is Over.  -- Herb Sutter

在 C++ 语境下，并发 (Concurrency) 与并行 (Parallelism) 两个词的侧重点不同：

- **并发**：侧重于任务**分离**，关注如何将一个程序拆分为多个独立的执行单元，每个单元可独立调度、独立推进。
- **并行**：侧重于数据**吞吐**，关注如何同时执行多个任务来缩短总运行时间，从而在更短的时间内完成计算任务。

并发的实现方式有多进程并发 (Multi-process)和多线程并发 (Multi-thread)两种，C++ 采用的是**多线程并发**：

- 多进程并发 (Multi-process)：将应用程序分为多个独立的进程同时运行，通过 IPC 进行通信，启动慢、开销大、依赖操作系统 API。
- 多线程并发 (Multi-thread)：进程中的线程相互独立，在不同的指令序列中运行，共享地址空间，启动快、开销小、C++11 原生支持。

C++ 并发编程历史：

1. 黑暗时代 C++98：标准不承认线程存在。全靠 `#ifdef _WIN32` 调用 Windows API 或 `#include <pthread.h>` 调用 POSIX。
2. 破晓时代 C++11：引入了内存模型，以及 `std::thread`, `std::mutex`, `std::atomic` 等工具（来源于 Boost 线程库）。
3. 成熟时代 C++14/17：引入了 `std::shared_mutex`, `std::scoped_lock` 以及并行标准库算法。

C++ 的终极自信：
C++ 标准委员会在设计标准库时，确保使用高级 API 与使用底层 API 的性能收益相当（抽象代价 Abstraction Penalty 很低）

## 1. 线程管理 (Thread Management)

`std::thread` 对象的**本质**是底层系统线程在用户态的句柄（Handle）。它本身不是执行流，而是执行流的管理者。

`std::thread` 通过可调用对象构造（采用**值拷贝**的方式传参），线程对象创建时即启动，在程序结束前需要显式调用 `join()` 或 `detach()`，否则程序将直接调用 `std::terminate()` 崩溃。

> :warning: 注意避免**语法解析 Most Vexing Parse**：传递临时对象给构造函数时会被误认为函数声明

```cpp
// ❌ 错误：这被解析为声明了一个名为 my_thread 的函数
std::thread my_thread(background_task());

// ✅ 正确：使用多组括号、统一初始化（大括号）或 Lambda
std::thread t1((background_task())); 
std::thread t2{background_task()}; 
std::thread t3([]{ background_task()(); });
```
`std::thread` 对象的生命周期管理 (join vs detach)：

- `t.join()` 阻塞：主线程等待子线程执行完毕，清理资源。
- `t.detach()` 分离：子线程变成守护线程 (Daemon Thread) 在后台独立运行。由 C++ 运行库负责清理资源。

> :warning: 注意避免 `detach` 的线程访问主线程的局部变量的引用或指针，即**悬空引用 (Dangling Reference)**。

线程管理中的异常安全：C++11 采用通过 `try-catch` 确保正确地 `join()` 或封装一个 RAII 的线程类。
C++ 20 直接提供了 `std::jthread`，会在析构时会自动安全地阻塞（join）并支持协作式中断。

向可调用对象或函数传递参数直接将参数作为 `std::thread` 构造函数的附加参数即可，与 `std::bind` 类似（都是将可调用对象与参数一起暂存）。
但 :warning: 注意，有三个易踩坑的细节：

1. 传递字符数组时，隐式转换的时间不确定，可能引发悬空引用：
    ```cpp
    void oops(int some_param) {
        char buffer[1024];
        sprintf(buffer, "%i", some_param);
        // ❌ 危险：传递的是 buffer 的指针，转换 std::string 可能发生在 oops 退出之后！
        std::thread t(f, 3, buffer); 
        // ✅ 正确：显式构造，在主线程完成转换
        std::thread t(f, 3, std::string(buffer)); 
    }
    ```

2. 线程对象的构造函数内部会将值拷贝以右值形式传递（为了支持移动构造），如果形参是左值引用的话，会引发类型不匹配错误。
    ```cpp
    void oops() {
        widget_data data;
        // ✅ 正确：显式使用 std::ref 告诉 std::thread 按引用传递，而非按值盲目拷贝右值
        std::thread t(update_data_for_widget, std::ref(data));
        t.join();
    }
    ```

3. 传递 Move-only 类型时要用 `std::move` 将对象的所有权强行转移到线程上下文中。（注意避免覆盖正在运行的线程）
    ```cpp
    std::unique_ptr<big_object> p(new big_object);
    std::thread t(process, std::move(p)); // p 变为空
    ```

`std::thread` 对象的特性是：**可移动，不可复制 (Move-only)**。它独占底层的操作系统线程句柄。

实践中一些常用的方法：

```cpp
std::thread::hardware_concurrency();  // 探测 CPU 的硬件并发线程数，如果系统信息无法获取，可能会返回 0
std::this_thread::get_id();  // 获取线程自身的 ID
t.get_id();  // 获取某个线程对象的 ID
```

## 2. 共享数据 (Shared Data)

首先**不变量 (Invariant)**是指在程序执行的某个阶段，无论经历多少次操作，都始终保持为真的性质或条件。
共享数据的问题在于：在多线程并发修改共享数据时，往往会破坏数据结构的不变量。主要原因有条件竞争 (Race Condition) 和数据竞争 (Data Race)：

- 条件竞争：程序的执行结果取决于多个线程交错执行的顺序。大多数情况下是良性的。
- 数据竞争：两个线程并发修改同一个独立对象，且未加同步措施。在 C++ 中属于未定义行为 (UB)。

防御策略三大流派：

- 悲观锁定：使用互斥量 `std::mutex`，确保修改期间其他线程被阻塞。
- 乐观无锁：通过 CAS (Compare-And-Swap) 原子操作实现无锁修改数据结构及其不变量（极难）。
- 软件事务内存 (STM)：将更新视为数据库事务，发生冲突则回滚（C++ 目前无原生支持）。

### 2.1 基础工具：互斥量与 RAII 锁

不要自己手动调用 `mtx.lock()` 和 `mtx.ublock()`，推荐使用 RAII 锁自动化管理。

| RAII 锁 | 特点 |
|:---:|:---:|
| `std::lock_guard` (C++11) | 构造时加锁，析构时解锁，零额外开销 |
| `std::unique_lock` (C++11) | 支持延迟加锁、提前解锁、转移所有权，较为灵活 |
| `std::scoped_lock` (C++17) | 利用模板参数推导，可同时锁定多个互斥量 |

> :warning: 注意，不要将受保护数据的指针或引用通过函数返回值传出，更不要将其作为参数传递给外部提供的可调用对象。

即使类内的每个方法都加了锁保护，接口间也会存在条件竞争（这是接口固有的问题，与实现方式无关）：

```cpp
// ❌ 经典接口竞争：empty 和 top 之间，或者 top 和 pop 之间，
if (!s.empty()) {
    int value = s.top();
    s.pop();
    do_something(value);
}
```

线程安全栈的 2 种方案：

- 通过引用获取结果：`void pop(T& value)`（缺点：类型必须支持默认构造和赋值）。
- 返回智能指针 `std::shared_ptr<T> pop()`（推荐：既避免了拷贝异常丢失数据，又解决了内存管理开销）。

当要同时锁定多个互斥量时，需要考虑死锁问题，有以下实践经验：

1. 使用 C++17 的 `std::scoped_lock guard(m1, m2);` 一次性、原子化地获取所有需要的锁。
2. 避免嵌套锁：如果已经持有一个锁，尽量不要再去获取第二个锁。
3. 避免在持有锁时调用外部代码.
4. 按固定顺序加锁（例如：手递手锁定 hand-over-hand locking）。
5. 层次锁 (Hierarchical Mutex)：给每个互斥量分配层级编号，规定线程只能按照层级递减的顺序加锁（运行时强制检查抛异常）。

### 2.2 特定场景下的专用工具

在某些特定场景下，用 `std::mutex` 过于沉重，C++ 提供了专用工具：

1. 保护初始化过程（如单例模式）

❌ 双重检查锁定模式 (Double-Checked Locking, DCLP) 由于内存重排，会导致数据竞争（读到未完全初始化对象的指针）。
```cpp
std::shared_ptr<some_resource> resource_ptr;
std::mutex resource_mutex;

void undefined_behaviour_with_double_checked_locking() {
    if (!resource_ptr) { // 1. 第一次检查（无锁）
        std::lock_guard<std::mutex> lk(resource_mutex);
        if (!resource_ptr) { // 2. 第二次检查（有锁）
            // 3. 致命缺陷：new 操作包含了三步：分配内存、调用构造函数、将地址赋给指针。
            // 编译器或 CPU 可能会发生“指令重排”，先将未初始化的内存地址赋给 resource_ptr。
            resource_ptr.reset(new some_resource); 
        }
    }
    resource_ptr->do_something(); 
}
```

✅ 现代 C++ 标准方案 1：`std::once_flag` 配合 `std::call_once`
```cpp
std::shared_ptr<some_resource> resource_ptr;
std::once_flag resource_flag; // 必须将其与需要初始化的数据定义在同一作用域

void init_resource() {
    resource_ptr.reset(new some_resource);
}

void foo() {
    // std::call_once 保证了 init_resource 也绝对只会被执行一次，且其他线程会阻塞等待初始化彻底完成。
    std::call_once(resource_flag, init_resource);
    resource_ptr->do_something();
}

// 💡 提示：std::call_once 也常用于类内部，结合 Lambda 或成员函数来延迟初始化成员变量。
```

✅ 现代 C++ 标准方案 2：C++11 规定局部静态变量的初始化是线程安全的
```cpp
my_class& get_instance() {
    static my_class instance; // C++11 原生保证线程安全且只初始化一次
    return instance;
}
```

## 3. 同步操作 (Synchronizing Concurrent Operations)

这一章涵盖了条件变量、Future 机制、时间 API 以及 C++20 引入的阶段性同步原语（锁存器与栅栏）。

### 3.1 条件变量 (Condition Variable)

C++ 标准库对条件变量有两套实现: `std::condition_variable` 和 `std::condition_variable_any`，都包含在 `<condition_variable>` 中：

- `std::condition_variable`：只能与 `std::unique_lock<std::mutex>` 配合使用（性能极高，首选）。
- `std::condition_variable_any`：可以与任何 BasicLockable 的锁配合（更灵活，但有额外性能开销）。

> :warning: 注意**虚假唤醒**：操作系统可能会在没有接收到 `notify` 时意外唤醒等待的线程，因此需要在 `wait()` 中传入 Lambda 表达式作为谓词检查。

```cpp
std::mutex mut;
std::queue<data_chunk> data_queue;
std::condition_variable data_cond;

// 消费者线程
void data_processing_thread() {
    std::unique_lock<std::mutex> lk(mut); // 必须用 unique_lock，因为 wait 内部需要解锁和重锁
    // ✅ 严谨写法：传入 Lambda。只有当队列不为空时，才会真正停止等待，否则继续休眠。
    data_cond.wait(lk, []{ return !data_queue.empty(); }); 
    // 处理数据...
}

// 生产者线程
void data_preparation_thread() {
    {
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(data);
    } // 💡 最佳实践：先解锁，再通知。避免消费者醒来后立刻因为抢不到锁又被迫休眠。
    data_cond.notify_one(); 
}
```

### 3.2 异步编程工具

Future 是一次性事件的接收端，代表一个“未来必将达到的结果”。C++ 提供了三种获取 Future 的途径:

1. `std::async` 异步任务，返回 `std::future`。
2. `std::packaged_task` 任务打包（放入任务队列 / 线程池），像是一个带有 Future 通道的 `std::function`。
3. `std::promise` 提供 `set_value()` 和 `set_exception()` 手动将值或异常塞入通道。

`std::async` 的启动策略 (Launch Policy)：

- `std::launch::async`：强制开启一个新线程去执行。
- `std::launch::deferred`：延迟执行（惰性求值）。直到在主线程调用 `future.get()` 或 `future.wait()` 时才会在主线程中**串行执行**。

`std::future` vs `std::shared_future`：

1. `std::future` 是 Move-only 的，它的 `.get()` 只能调用一次，调用后内部状态被转移。
2. 如果多个线程需要等待同一个异步结果（比如广播启动信号），必须使用支持 Copy 的 `std::shared_future`。

C++ 提供了两种超时维度：

- 时间段 (`_for`)：如 `wait_for(100ms)`。
- 时间点 (`_until`)：如 `wait_until(timeout_time)`。

> :warning: 注意，时钟 `std::chrono` 有两种：`std::chrono::system_clock`（系统/墙上时钟）和 `std::chrono::steady_clock`，
> 系统时钟可以被修改，`wait_until` 可能会陷入无限等待，因此在超时等待中优先使用单调递增的 `std::chrono::steady_clock`。

### 3.3 异步编程架构

1. 函数式编程 (FP) 并发：向函数传入参数，通过 `std::async` 返回结果，函数内部绝不修改共享数据。
2. CSP / Actor 模型：线程是完全独立的实体（没有共享的全局变量），而是通过“消息队列”相互发送指令进行状态机流转。

> Communicating Sequential Processes (CSP，通信顺序进程) 由图灵奖得主 C.A.R. Hoare 提出。

### 3.4 C++20 阶段同步原语

当涉及一组线程的协同工作时：

- Latch (锁存器)：计数器递减。减到 0 时，所有阻塞在 `wait()` 的线程被放行。❌ 不可复用。
- Barrier (栅栏)：所有线程到达后 (`arrive_and_wait()`) 才可进入下一阶段。✅ 可重置复用。每轮可执行一个收尾回调函数。

---

# 内存模型和原子操作

C++是系统级别的编程语言，标准委员会的目标是不需要比C++还要底层的高级语言。
多线程（感知）内存模型定义了基本部件应该如何工作。原子类型和原子操作就可以“接触硬件”，并提供底层同步操作。

## 1. 内存模型

C++ 中“一切皆对象”（无论是 `int` 及其衍生类型、函数、自定义类对象）。对象仅仅是对 C++ 数据块的声明，存储在一个或多个**内存位置**上。
两个线程访问不同的内存位置，或访问同一个内存位置但都不修改数据（只读）是安全的。但若多个线程访问同一个内存位置，且至少有一个在写，若不加同步（如锁或原子操作），将触发**未定义行为 (UB)**。

- 内存位置 (Memory Location)：标量类型对象或是相邻位域 (Bit-field) 的最大连续序列

> CPU 读写内存的最小物理单位是“字节 (Byte)”（甚至是更大的“字 Word”）。相邻的位域（如 `struct { int a:1; int b:1; }`）在底层硬件上**共享同一个内存位置（被塞在同一个字节/字中）**。

- 修改顺序 (Modification Order)：对于某一个特定的原子变量，编译器负责同步所有线程对他的操作顺序（除非是非限制序（Relaxed））。

## 2. 原子类型

- 绝对无锁：`std::atomic_flag`，必须用 `ATOMIC_FLAG_INIT` 初始化，提供“读-改-写” `test_and_set()` 和“清除” `clear()` 操作。
- 通用布尔类型：`std::atomic<bool>`，支持赋值、`load()`, `store()`, `exchange()`（类似 `test_and_set()`）。此外还有两种 CAS 操作：
  - `compare_exchange_weak`：可能因为 CPU 架构原因（如线程切换）导致“伪失败 (Spurious failure)”，必须配合 while 循环使用。
  - `compare_exchange_strong`：保证只在值不相等时才失败。在 x86 等强缓存连贯性架构上可以直接使用；但在底层使用 LL/SC 指令的架构（如 ARM）上性能较弱。
- 指针与整型特化 `std::atomic<T*>` / `std::atomic<int>`：支持原子加减：`fetch_add()`, `fetch_sub()`, `++`, `--`，返回旧值。
- 自定义类型原子化 `std::atomic<UDT>`：必须满足是**可平凡拷贝的 (Trivially Copyable)**，**没有虚函数**，**不能有用户自定义的拷贝赋值操作符**。

> `test_and_set()` 经历了 Read-Modify-Write 的过程，也就是返回旧值-写入寄存器-写回内存，但只能将值改为 `true`。`exchange()`也是一样，返回旧值，设置新值。

> CAS 更为谨慎，只有当旧值与期望值匹配时才设置新值，返回是否成功修改。

## 3. 内存序 (Memory Order)

C++ 允许通过指定内存序来让编译器和 CPU 放弃部分指令重排，从而强制建立同步关系（Synchronizes-with）和先行关系（Happens-before）。

- **同步关系**：使用正确的内存序（Acquire-Release 语义）在不同线程间操作同一个原子变量时，就会建立同步关系。
- **先行关系**：操作 A 先行于 B，意思是操作 A 所产生的内存修改，操作 B 全都可以看见。

| 内存序 `std::memory_order_` | 核心语义 |
|:---:|:---:|
| `relaxed` 自由序 | 仅保证操作的原子性，不提供线程同步或指令重排限制 |
| `consume` 数据依赖序 | 仅同步与当前读取的值有直接“数据依赖”的变量 |
| `acquire` 获取序 | 用于“读”操作。保证当前线程之后的读写操作不被重排到前面 |
| `release` 释放序 | 用于“写”操作。保证当前线程之前的读写操作不被重排到后面 |
| `acq_rel` 获取-释放序 | 用于“读-改-写 (RMW)”操作。兼具 acquire 和 release 的双重屏障效果 |
| `seq_cst` 顺序一致序 | 最强（默认）。保证所有线程先后顺序一致 |

## 4. 内存屏障

`std::atomic_thread_fence(memory_order)` 是一种独立于特定变量的全局同步机制，限制 CPU 的指令重排。

> - **释放屏障 (`memory_order_release`)**：放在 `relaxed` 写操作**之前**。保证屏障前的读写不会到屏障后。
> - **获取屏障 (`memory_order_acquire`)**：放在 `relaxed` 读操作**之后**。保证屏障后的读写不会到屏障前。

---

# 基于锁的并发数据结构

设计并发数据结构的核心在于**缩小保护区域，减少序列化操作**。串行化的代码虽然线程安全，但并发度为 0。

## 1. 线程安全栈 (Stack) —— 粗粒度锁

直接用 `std::mutex` + `std::lock_guard` 保护整个对象。缺点是**完全串行化**，同一时刻只能有一个线程操作。

[ThreadSafeStack](../scripts/cpp-concurrency-action/include/thread_safe_stack.hpp)

## 2. 线程安全队列 (Queue) —— 细粒度锁与哑节点

为了实现真正的并发，必须让 push 和 pop 能够同时执行。这就需要把头指针（head）和尾指针（tail）分开保护。

分配一个无数据的哑节点，这样 head 和 tail 永远不会指向同一个物理节点（空队列时 `head->next == tail`）。

[ThreadSafeQueue](../scripts/cpp-concurrency-action/include/thread_safe_queue.hpp)

## 3. 线程安全查询表 (Hash Map) —— 分段锁与读写锁

分段锁 + 读写锁分离：建立固定数量的“桶 (Bucket)”，每个桶分配一把锁。查询表通常是“读多写少”，使用 `std::shared_mutex`。

## 4. 线程安全链表 (Linked List) —— 步进式加锁 (Hand-over-hand Locking)

步进式加锁模型：每个节点自带 `std::mutex`，遍历时先锁住当前节点，再锁住下一个节点，然后释放当前节点的锁。

死锁防御：所有线程必须严格从 head 开始，按照单向的顺序进行遍历和加锁，绝不允许反向遍历。

---

# 无锁的并发数据结构

使用互斥量、条件变量，以及 `future` 可以用来同步算法和数据结构。调用库函数将会挂起执行线程，操作系统会完全挂起一个阻塞线程（并将其时间片交给其他线程）直到解阻塞。

“解阻塞”的方式很多，比如互斥锁解锁、通知条件变量达成，或让“future状态”就绪。不使用阻塞库的数据结构和算法称为**无阻塞结构**。不过无阻塞的数据结构并非都是无锁的。

无阻塞结构有三层境界：

1. 无阻碍 (Obstruction-Free)：其他线程都暂停了才能运行完。
2. 无锁 (Lock-Free)：保证有一个线程能推进进度（可能存在个别线程“饥饿”）。
3. 无等待 (Wait-Free)：保证所有线程都能在有限步数内完成操作（没有无限重试的 while 循环）。

无锁不代表绝对的高性能。在低冲突下无锁极快；但在超高冲突下，大量 CAS 失败重试会引发缓存一致性风暴（Cache Ping-Pong），反而不如细粒度互斥锁。

## 1. 无锁栈 (Lock-Free Stack)

通过硬件 CPU 提供的原子的 CAS 操作，每次原子地替换**一个内存地址**（如指针）。单链表的压栈和弹栈，本质上只需要修改唯一的全局指针 `head`。

## 2. 无锁队列 (Lock-Free Queue)与线程互助 (Thread Helping)

无锁 `push` 时，需要做两件事：① 将旧尾节点的 `next` 指向新节点；② 将全局 `tail` 指针指向新节点。**这两步不能通过一次 CAS 原子地完成**。

破局思想就是**线程互助**：在 `push` 时发现尾节点的 `next` 不是 null（刚插入了新节点但还没来得及更新 tail），就顺手用 CAS 把全局 tail 指针挪过去。

## 3. 无锁设计的避坑指南

1. ABA 问题：当一个线程读取了一个值 A，准备进行 CAS 操作时，另一个线程将 A 修改为 B，又修改回 A。第一个线程的 CAS 成功了，但实际上数据已经被修改过了。

解决办法就是在指针中加入版本号（如 `std::atomic<std::pair<Node*, int>>`），每次修改指针时版本号加 1，这样即使值回到 A，版本号也不一样了。

2. 内存模型降级：只有在两步操作之间存在“先行关系”传递链时，才能使用更弱的内存序（如 `memory_order_relaxed`）来提升性能。否则就使用 `memory_order_seq_cst` 来保证全局一致。

3. 活锁 (Livelock)：虽然无锁结构不会死锁，但如果多个线程并发极高，一直互相破坏对方 CAS 的前提条件，导致所有线程都在疯狂执行 `while` 循环（忙等待），这就是**活锁**。

解决方法就是引入退让机制（如 `std::this_thread::yield()`），或者使用上述的“线程互助”策略，化竞争为协作。

4. 内存碎片与延迟回收爆炸：手写延迟回收列表时，如果回收阈值设置不当，或者高压下回收线程抢不到时间片，会导致旧节点堆积如山，最终 Out Of Memory (OOM) 而亡。

---

# 并发设计

## 1. Dividing Work Between Threads

1. 数据划分 (Data Partitioning)

切数据。适用于数据量固定，且每个数据的处理耗时差不多。

2. 递归划分 (Recursive Partitioning)

分治法。不断把大问题劈成两半，每劈一次就交给一个新线程（或扔进线程池）去做。比如快速排序、归并排序。

3. 按任务类型划分 / 流水线 (Pipeline)

流水线。线程 A 负责下载，线程 B 负责解压，线程 C 负责解析。

## 2. 性能的硬件级“内鬼”

1. 乒乓缓存 (Cache Ping-Pong)

现代 CPU 都有自己的 L1/L2 缓存。如果多核频繁修改同一个变量，这个变量所在的缓存行（Cache Line）必须在各个 CPU 核心之间来回传递（刷新和失效），这就叫“乒乓缓存”。

2. 伪共享 (False Sharing)

CPU 读写内存是按“缓存行（Cache Line，通常是 64 字节）”打包读取。

比如 `array[0]` 和 `array[1]` 靠得太近，被打包进了同一个缓存行。thread1 修改 `array[0]` 时，会导致整个缓存行失效，连累 thread2 被迫重新从主存读取 `array[1]`。

3. 超额申请 (Oversubscription)

申请过多的纯计算线程时，操作系统要在线程之间疯狂进行上下文切换（Context Switch），每次切换都要保存寄存器、刷新页表、清空缓存。

## 3. 阿姆达尔定律 (Amdahl's Law)

$$
    P = \frac{1}{(1-f)+\frac{f}{N}}
$$

程序中有 10% 的代码（比如启动初始化、最后的合并汇总）必须串行执行。那么无论买多么牛逼的服务器、加多少个 CPU 核心，程序最多也只能比原来快 10 倍。